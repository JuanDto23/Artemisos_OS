#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
// Bibliotecas propias
#include "pcb.h"
#include "queue.h"
#include "gui.h"
#include "memory.h"

int main(void)
{
  int exited = FALSE;                              // Indica si salió (ESC ó EXIT)
  char buffers[NUMBER_BUFFERS][BUFFER_SIZE] = {0}; // Buffers de historial
  int c = 0;                                       // Carácter introducido por usuario
  int index = 0;                                   // índice de buffer de prompt
  int result_interpretation = 0;                   // Retorno de interpretar la instrucción leída
  int index_history = 0;                           // Índice de buffer de historial seleccionado (↑ o ↓)
  char line[INSTRUCTION_SIZE] = {0};               // Línea leída de archivo
  int quantum = 0;                                 // Contador para el número de instrucciones antes de alcanzar el MAXQUANTUM
  int speed_level = 1;                             // Nivel de velocidad de lectura de instrucciones
  unsigned timer = 0;                              // Temporizador para determinar la velocidad de escritura
  unsigned init_timer = 0;                         // Inicializador para timer, reduce la brecha entre MAX_TIME
  int minor_priority = 0;                          // Variable para saber que nodo extraer de ready para mandarlo a ejecución
  FILE *swap = NULL;                               // Bloque contiguo de almacenamiento binario en disco duro
  TMS tms;                                         // Estructura con: Arreglo de enteros para marcar el estado de los marcos de la SWAP (disponible/ocupado) y número de marcos disponibles
  int swap_disp = 0;                               // Desplazamiento de SWAP
  int tms_disp = 0;                                // Desplazamiento de TMS
  int tmp_disp = 0;                                // Desplazamiento de TMP
  int lists_disp = 0;                              // Desplazamiento de Listas

  // Se crean instancias de las colas
  Queue execution;
  Queue ready;
  Queue finished;
  Queue new;

  // Inicializar las colas
  initialize_queue(&execution);
  initialize_queue(&ready);
  initialize_queue(&finished);
  initialize_queue(&new);

  // Se crea instancia de la GUI
  GUI gui;

  initscr();                                                       // Inicia la ventana
  start_color();                                                   // Permite el uso de colores
  use_default_colors();                                            // Permite usar -1 como fondo transparente
  noecho();                                                        // Evita la impresión automática de caracteres
  set_escdelay(10);                                                // Proporciona el tiempo de expiración del ESC en milisegundos
  initialize_gui(&gui);                                            // Inicializar GUI
  keypad(gui.inner_prompt, TRUE);                                  // Habilita las teclas especiales después de inicializar la GUI
  empty_processor(gui.inner_cpu);                                  // Se imprime el contenido del CPU inicialmente vacío
  print_queues(gui.inner_queues, execution, ready, finished, new); // Se imprimen las colas en su ventana
  print_prompt(gui.inner_prompt, 0);                               // Se imprime prompt en fila 0
  swap = create_swap();                                            // Crear un archivo lleno de ceros al inicio del programa (SWAP)
  print_swap(gui.inner_swap, swap, swap_disp);
  initialize_tms(&tms);                                            // Inicializar tabla TMS (arreglo en ceros y páginas máximas)
  do
  {
    if (!execution.head) // Si la cola Ejecución está vacía
    {
      // Gestor de comandos de terminal
      exited = command_handling(&gui, buffers, &c, &index, &index_history,
                                &execution, &ready, &finished, &new,
                                &timer, &init_timer, &speed_level, &tms, &swap, &swap_disp);
      if (ready.head) // Verifica si hay nodos en la cola Listos
      {
        // Ahora ya no se extrae el primer nodo de listos, se busca el de menor prioridad
        minor_priority = get_minor_priority(ready);
        enqueue(extract_by_priority(minor_priority, &ready), &execution);
        quantum = 0; // Se establece en 0 para el proceso que acaba de entrar
      }
    }
    else // Hay un nodo en Ejecución
    {
      if (timer <= MAX_TIME) // Permite escribir en la prompt antes de que se alcance el MAX_TIME
      {
        // Gestor de comandos de terminal
        exited = command_handling(&gui, buffers, &c, &index, &index_history,
                                  &execution, &ready, &finished, &new,
                                  &timer, &init_timer, &speed_level, &tms, &swap, &swap_disp);
      }
      else // Si se alcanzó el MAX_TIME se ejecuta la instrucción
      {
        /* Si no hay más líneas del archivo por leer, esto es,
        si no se encuentra END, lo que representa un error*/
        if (!read_line(&execution.head->program, line))
        {
          // Cierra el archivo
          fclose(execution.head->program);
          // Evita puntero colgante
          execution.head->program = NULL;
          // Se verifica que no hay otro proceso del mismo usuario en Listos
          if (!search_uid(execution.head->UID, ready))
          {
            NumUs--;
          }
          // Se extrae proceso de Ejecución y se encola a Terminados
          enqueue(dequeue(&execution), &finished);
          // Se actualiza el valor de W
          if (NumUs)
          {
            W = 1.0 / NumUs;
          }
          else
          {
            W = 0.0; // No hay ningún usuario
          }
          /* Se limpia área de mensajes con wclear para que redibuje todo
             y no queden residuos de carácteres */
          wclear(gui.inner_msg);
          // Se imprime mensaje
          mvwprintw(gui.inner_msg, 0, 0, "Terminación anormal del programa.");
          // Se imprime procesador vacío
          empty_processor(gui.inner_cpu);
          // Se refresca la subventana de mensajes
          wrefresh(gui.inner_msg);
        }
        else // Si hay más líneas por leer
        {
          // Línea sin salto de línea o retorno de carro
          line[strcspn(line, "\r\n")] = '\0';
          // Interpreta y ejecuta la instrucción
          result_interpretation = interpret_instruction(&gui, line, execution.head);
          if (result_interpretation != 0 && result_interpretation != -1) // Instrucción ejecutada correctamente
          {
            // Si se lee algo, se incrementa PC y quantum, de otro modo, solo quantum
            if (result_interpretation != 2)
            {
              (execution.head->PC)++; // Se incrementa PC
            }
            // Se incrementa el número de instrucciones ejecutadas (quantum)
            quantum++;
            // Se actualizan la impresión de los registros del pcb en ejecución
            print_processor(gui.inner_cpu, *execution.head);
            // Se actualizan los valores de uso de CPU para el proceso en ejecución (KCPU)
            execution.head->KCPU += IncCPU; // Saltos de 15
            // También incrementa el uso de procesador por usuario al proceso en ejecución, para darle paso a otro que no se haya ejecutado todavia
            // Se quiere que todos los procesos del mismo usuario se vayan intercalando, no que se ejecuten secuencialemente (de a ratitos cada uno)
            execution.head->KCPUxU += IncCPU;
            /* Actualiza los contadores de uso del CPU para todos los procesos (no Terminados)
              del usuario dueño del proceso en ejecución */
            update_KCPUxU_per_process(execution.head->UID, &ready);
          }
          else // Se encontró la instrucción END o un error, en cuarquier caso terminar el pcb
          {
            // Cierra el archivo
            fclose(execution.head->program);
            // Evita puntero colgante
            execution.head->program = NULL;
            if (result_interpretation == -1) // Hubo un error
            {
              mvwprintw(gui.inner_msg, 1, 0, "Terminación anormal del programa.");
            }
            else // Hubo un END
            {
              mvwprintw(gui.inner_msg, 0, 0, "Programa finalizado correctamente.");
            }
            // Se verifica que no hay otro proceso del mismo usuario en Listos
            if (!search_uid(execution.head->UID, ready))
            {
              NumUs--;
            }
            // Se actualiza el valor de W
            if (NumUs)
            {
              W = 1.0 / NumUs;
            }
            else
            {
              W = 0.0; // No hay ningún usuario
            }
            // Se extrae nodo en ejecución y encola en Terminados
            enqueue(dequeue(&execution), &finished);
            // Se imprime procesador vacío
            empty_processor(gui.inner_cpu);
            // Se refresca la subventana de mensajes
            wrefresh(gui.inner_msg);
          }
          if (quantum == MAXQUANTUM) // quantum llegó a MAXQUANTUM (4) y aún no se llega al fin del archivo
          {
            if (ready.head) // Se verifica si hay proceso en Listos para hacer el SWAP
            {
              // Sacar el único nodo en Ejecución, e insertarlo al final de Listos
              enqueue((dequeue(&execution)), &ready);
              // Actualiza los parámetros de planificación, para todos los nodos de la cola Listos
              update_parameters(&ready);
              // Mostrar el proceso extraído de Ejecución por 3 segundos
              print_queues(gui.inner_queues, execution, ready, finished, new);
              sleep(3);
              // Ahora ya no se extrae el primer nodo de listos, se busca el de menor prioridad y se extrae
              minor_priority = get_minor_priority(ready);
              enqueue(extract_by_priority(minor_priority, &ready), &execution);
            }
            /* Se reinicia para comenzar a ejecutar
            el siguiente proceso o el mismo si no hay más */
            quantum = 0;
          }
        }
        timer = init_timer;                                              // Se reinicia temporizador para volver a escribir en línea de comandos
        print_ginfo(gui.inner_ginfo, execution);                         // Se imprime información general
        print_queues(gui.inner_queues, execution, ready, finished, new); // Se imprimen las colas en su ventana
      }
    }
    wmove(gui.inner_prompt, 0, PROMPT_START + index); // Se coloca el cursor en su lugar
    wrefresh(gui.inner_prompt);                       // Refresca las ventana de inner_prompt
  } while (!exited);
  fclose(swap); // Se cierra swap hasta el final de la ejecución del simulador
  endwin();     // Cierra la ventana
  printf("\n"); // Salto de línea en terminal
  return 0;
}
