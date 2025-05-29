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
  create_swap(&swap);                                              // Crear un archivo lleno de ceros al inicio del programa (SWAP)
  print_swap(gui.inner_swap, swap, swap_disp);                     // Imprime el contenido de la SWAP con desplazamiento
  initialize_tms(&tms);                                            // Inicializar tabla TMS (arreglo en ceros y páginas máximas)
  print_tms(gui.inner_tms, tms, tms_disp);                         // Imprime el contenido de la TMS con desplazamiento
  print_tmp(gui.inner_tmp, execution.head, tmp_disp);              // Imprime el contenido de la TMP con desplazamiento
  do
  {
    if (!execution.head) // Si la cola Ejecución está vacía
    {
      // Gestor de comandos de terminal
      exited = command_handling(&gui, buffers, &c, &index, &index_history,
                                &execution, &ready, &finished, &new,
                                &timer, &init_timer, &speed_level, &tms,
                                &swap, &swap_disp, &tms_disp, &tmp_disp, execution.head);
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
                                  &timer, &init_timer, &speed_level, &tms,
                                  &swap, &swap_disp, &tms_disp, &tmp_disp, execution.head);
      }
      else // Si se alcanzó el MAX_TIME se ejecuta la instrucción
      {
        /* Si no hay más instrucciones del proceso en ejecución, esto es,
        si no se encuentra END, lo que representa un error*/
        read_line_from_swap(swap, line, execution.head);
        if (execution.head->PC == execution.head->lines && strcmp(line, "END"))
        {
          // Se hacen acciones para cuando se terminó de ejecutar el proceso
          // Calculo de parámetros de planificación
          // Se verifica que no hay otro proceso del mismo usuario en Listos
          if (!search_uid(execution.head->UID, ready))
          {
            NumUs--;
          }

          // El proceso termina y no queda ningún proceso hermano
          PCB *brother_pcb;
          if (!(brother_pcb = search_brother_process(execution.head->UID, execution.head->file_name, ready)))
          {
            // Establecer sus marcos de SWAP cómo libres en la TMS
            free_pages_from_tms(execution.head, &tms);
            // Liberar la memoria que ocupe la TMP asociada
            free(execution.head->TMP);
            execution.head->TMP = NULL;
            
            // Si hay procesos en la lista Nuevos
            if (new.head)
            {
              // Buscar por orden de llegada, algún proceso que sea de menor tamaño que la SWAP libre
              // Mientras se encuentren procesos que se quepan en swap
              PCB *new_pcb = search_process_smaller_swap(&new, tms.available_pages);

              if (new_pcb)
              {
                // Cargarlo a listos
                enqueue(new_pcb, &ready);
                // Se imprime mensaje de proceso creado
                mvwprintw(gui.inner_msg, 0, 0, "Proceso: %d [%s] UID: [%d]", new_pcb->pid, new_pcb->file_name, new_pcb->UID);
                mvwprintw(gui.inner_msg, 1, 0, "Proceso nuevo cargado a la SWAP.");

                // Se reserva espacio para la tmp del proceso
                new_pcb->TMP = (int *)malloc(sizeof(int) * new_pcb->TmpSize);

                // Cargar instrucciones en swap y registrar en TMS los marcos ocupados por el proceso
                load_to_swap(new_pcb, &tms, &swap, new_pcb->lines, &gui);

                // Se cierra el archivo una vez que se ha cargado en la swap
                fclose(new_pcb->program);
                // Se evita puntero colgante
                new_pcb->program = NULL;
              }
            }
          }
          else // El proceso termina pero aún hay procesos hermanos
          {
            // Actualiza el PID en TMS para el hermano encontrado que sigue vivo
            update_pages_from_tms(brother_pcb, &tms);
          }
          // Se muestran los cambios en la tms
          print_tms(gui.inner_tms, tms, tms_disp);

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
          // empty_processor(gui.inner_cpu);

          // Se refresca la subventana de mensajes
          wrefresh(gui.inner_msg);
        }
        else // Si hay más líneas por leer
        {
          // Interpreta y ejecuta la instrucción
          result_interpretation = interpret_instruction(&gui, line, execution.head);
          if (result_interpretation != 0 && result_interpretation != -1) // Instrucción ejecutada correctamente
          {
            // Se incrementa PC
            (execution.head->PC)++;
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
            if (result_interpretation == -1) // Hubo un error
            {
              mvwprintw(gui.inner_msg, 1, 0, "Terminación anormal del programa.");
            }
            else // Hubo un END
            {
              mvwprintw(gui.inner_msg, 0, 0, "Programa finalizado correctamente.");
            }
            // Se verifica que no hay otro proceso del mismo usuario en Listos (proceso hermano)
            if (!search_uid(execution.head->UID, ready))
            {
              NumUs--;
            }

            // El proceso termina y no queda ningún proceso hermano
            PCB *brother_pcb;
            if (!(brother_pcb = search_brother_process(execution.head->UID, execution.head->file_name, ready)))
            {
              // Establecer sus marcos de SWAP cómo libres en la TMS
              free_pages_from_tms(execution.head, &tms);
              // Liberar la memoria que ocupe la TMP asociada
              free(execution.head->TMP);
              execution.head->TMP = NULL;

              // Si hay procesos en la lista Nuevos
              if (new.head)
              {
                // Buscar por orden de llegada, algún proceso que sea de menor tamaño que la SWAP libre
                // Mientras se encuentren procesos que se quepan en swap
                PCB *new_pcb = search_process_smaller_swap(&new, tms.available_pages);

                if (new_pcb)
                {
                  // Cargarlo a listos
                  enqueue(new_pcb, &ready);
                  // Se imprime mensaje de proceso creado
                  mvwprintw(gui.inner_msg, 0, 0, "Proceso: %d [%s] UID: [%d]", new_pcb->pid, new_pcb->file_name, new_pcb->UID);
                  mvwprintw(gui.inner_msg, 1, 0, "Proceso nuevo cargado a la SWAP.");

                  // Se reserva espacio para la tmp del proceso
                  new_pcb->TMP = (int *)malloc(sizeof(int) * new_pcb->TmpSize);

                  // Cargar instrucciones en swap y registrar en TMS los marcos ocupados por el proceso
                  load_to_swap(new_pcb, &tms, &swap, new_pcb->lines, &gui);

                  // Se cierra el archivo una vez que se ha cargado en la swap
                  fclose(new_pcb->program);
                  // Se evita puntero colgante
                  new_pcb->program = NULL;
                }
              }
            }
            else // El proceso termina pero aún hay procesos hermanos
            {
              // Actualiza el PID en TMS para el hermano encontrado que sigue vivo
              update_pages_from_tms(brother_pcb, &tms);
            }
            // Se muestran los cambios en la tms
            print_tms(gui.inner_tms, tms, tms_disp);

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
          if (quantum == MAXQUANTUM) // quantum llegó a MAXQUANTUM y aún no se llega al fin del archivo
          {
            if (execution.head)
            {
              // PID del proceso antes de desencolar en Ejecución
              int pid_before = execution.head->pid;

              // Se limpia subventana de mensajes
              werase(gui.inner_msg);

              // Sacar el único nodo en Ejecución, e insertarlo al final de Listos
              enqueue((dequeue(&execution)), &ready);
              // Actualiza los parámetros de planificación, para todos los nodos de la cola Listos
              update_parameters(&ready);

              // Mostrar el proceso extraído de Ejecución por 3 segundos y recalcular prioridades
              print_queues(gui.inner_queues, execution, ready, finished, new);

              // Se imprime mensaje de recalcular
              mvwprintw(gui.inner_msg, 0, 0, "Recalculando prioridades del planificador...");
              minor_priority = get_minor_priority(ready);
              // Se imprime la menor prioridad encontrada
              mvwprintw(gui.inner_msg, 1, 0, "Menor prioridad encontrada: [%d]", minor_priority);
              // Se imprime el tiempo antes de continuar
              mvwprintw(gui.inner_msg, 3, 0, "Tres segundos antes de continuar...");
              // Se refresca la subventana de mensajes
              wrefresh(gui.inner_msg);
              // Se coloca el cursor en su lugar
              wmove(gui.inner_prompt, 0, PROMPT_START);
              // Refresca las ventana de inner_prompt
              wrefresh(gui.inner_prompt);
              sleep(3);

              enqueue(extract_by_priority(minor_priority, &ready), &execution);

              // Se resetea el número de desplazamientos de la tmp cuando hay más de un proceso
              if (pid_before != execution.head->pid)
              {
                tmp_disp = 0;
              }
            }
            /* Se reinicia para comenzar a ejecutar
            el siguiente proceso o el mismo si no hay más */
            quantum = 0;
            // Se reinicia
          }
        }
        timer = init_timer;                                              // Se reinicia temporizador para volver a escribir en línea de comandos
        print_ginfo(gui.inner_ginfo, execution);                         // Se imprime información general
        print_queues(gui.inner_queues, execution, ready, finished, new); // Se imprimen las colas en su ventana
        print_tmp(gui.inner_tmp, execution.head, tmp_disp);              // Imprime el contenido de la TMP del proceso en ejecución con desplazamiento
        print_processor(gui.inner_cpu, *execution.head);
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
