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
#include "prompt.h"

// Prototipos de funciones
void run_simulator(void);
void initialize_ncurses_gui(GUI *gui);

// === FUNCIÓN PRNCIPAL ===
int main(int argc, char *argv[])
{
  // Se ejecuta el simulador del sistema operativo
  run_simulator();
  return 0;
}

void run_simulator(void)
{
  // Variables de control del simulador
  int exited = false;                            // Indica si salió (ESC ó EXIT)
  char buffers[HISTORY_SIZE][PROMPT_SIZE] = {0}; // Buffers de historial
  int index = 0;                                 // índice de buffer de prompt
  int result_interpretation = 0;                 // Retorno de interpretar la instrucción leída
  int index_history = 0;                         // Índice de buffer de historial seleccionado (↑ o ↓)
  char instruction[INSTRUCTION_SIZE] = {0};      // Instrucción leída de archivo SWAP
  int quantum = 0;                               // Contador para el número de instrucciones antes de alcanzar el MAXQUANTUM

  // Variables de velocidad de lectura de instrucciones
  int speed_level = 1;         // Nivel de velocidad de lectura de instrucciones
  unsigned init_timer = 0;     // Inicializador para timer, reduce la brecha entre MAX_TIME
  unsigned timer = init_timer; // Temporizador para determinar la velocidad de escritura

  int minor_priority = 0; // Variable para saber que nodo extraer de ready para mandarlo a ejecución

  // Variables de manejo de memoria
  FILE *swap = NULL; // Bloque contiguo de almacenamiento binario en disco duro
  TMS tms;           // Estructura con tabla de marcos de SWAP y número de páginas disponibles
  TMM tmm;           // Estructura con tabla de marcos de memoria y número de páginas disponibles

  // Variables para la navegacción de la SWAP, TMS, TMP y Listas
  int swap_disp = 0;  // Desplazamiento de SWAP
  int tms_disp = 0;   // Desplazamiento de TMS
  int tmp_disp = 0;   // Desplazamiento de TMP
  int lists_disp = 0; // Desplazamiento de Listas
  int ram_disp = 0;   // Desplazamiento de RAM

  // Se crean instancias de las colas y se inicializan
  Queue execution;
  initialize_queue(&execution);
  Queue ready;
  initialize_queue(&ready);
  Queue finished;
  initialize_queue(&finished);
  Queue new;
  initialize_queue(&new);

  // Se crea instancia de la GUI
  GUI gui;
  // Inicializar parámetros de ncurses y GUI
  initialize_ncurses_gui(&gui);

  // Impresión de plantillas de cada ventana
  empty_processor(gui.inner_cpu);                                              // Se imprime el contenido del CPU inicialmente vacío
  print_queues(gui.inner_queues, execution, ready, new, finished, lists_disp); // Se imprimen las colas en su ventana
  print_prompt(gui.inner_prompt, 0);                                           // Se imprime prompt en fila 0
  create_swap(&swap);                                                          // Crear un archivo lleno de ceros al inicio del programa (SWAP)
  print_swap(gui.inner_swap, swap, swap_disp);                                 // Imprime el contenido de la SWAP con desplazamiento
  initialize_tms(&tms);                                                        // Inicializar tabla TMS (arreglo en ceros y páginas máximas)
  print_tms(gui.inner_tms, tms, tms_disp);                                     // Imprime el contenido de la TMS con desplazamiento
  print_tmp(gui.inner_tmp, execution.head, tmp_disp);                          // Imprime el contenido de la TMP con desplazamiento
  initialize_tmm(&tmm);                                                        // Inicializar tabla de marcos de memoria (TMM)
  print_tmm(gui.inner_tmm, tmm);                                               // Imprime el contenido de la TMS con desplazamiento
  print_ram(gui.inner_ram, ram_disp);  

  // Ciclo principal del simulador
  do
  {
    // Si la cola Ejecución está vacía
    if (!execution.head)
    {
      // Gestor de comandos de terminal
      command_handling(&gui, &exited, buffers, &index, &index_history,
                       &execution, &ready, &finished, &new,
                       &timer, &init_timer, &speed_level, &tms,
                       &swap, &swap_disp, &tms_disp, &tmp_disp, &lists_disp, &ram_disp, execution.head);
      // Verifica si hay nodos en la cola Listos
      if (ready.head)
      {
        // Ahora ya no se extrae el primer nodo de listos, se busca el de menor prioridad y extrae
        recalculate_priorities(&gui, ready, &minor_priority);
        enqueue(extract_by_priority(minor_priority, &ready), &execution);
        // Se establece en 0 para el proceso que acaba de entrar
        quantum = 0;
        // Se imprimen colas, la TMP y la traducción dado que se acaba de encolar un proceso a Ejecución
        print_queues(gui.inner_queues, execution, ready, new, finished, lists_disp);
        print_tmp(gui.inner_tmp, execution.head, tmp_disp);
        print_traduction(gui.inner_msg, execution.head);
      }
    }
    // Hay un nodo en Ejecución
    else if (timer <= MAX_TIME) // Permite escribir en la prompt antes de que se alcance el MAX_TIME
    {
      // Gestor de comandos de terminal
      command_handling(&gui, &exited, buffers, &index, &index_history,
                       &execution, &ready, &finished, &new,
                       &timer, &init_timer, &speed_level, &tms,
                       &swap, &swap_disp, &tms_disp, &tmp_disp, &lists_disp, &ram_disp, execution.head);
    }
    // Si se alcanzó el MAX_TIME se ejecuta la instrucción
    else
    {
      // Se lee una instrucción del proceso en ejecución desde la RAM
      read_inst_from_ram(instruction, execution.head,&tmm, swap);
      //read_inst_from_swap(swap, instruction, execution.head);

      /* Si no hay más instrucciones del proceso en ejecución, esto es,
      si no se encuentra END, lo que representa un error*/
      if (execution.head->PC == execution.head->lines && strcmp(instruction, "END"))
      {
        // Se actualiza el número de usuarios (NumUs) y el peso W
        update_users(execution.head->UID, ready);
        // Se extrae nodo en ejecución
        PCB *process_extracted = dequeue(&execution);
        // El proceso termina y se realiza la gestión necesaria, una vez se saca de ejecución
        handle_process_termination(&gui, process_extracted, &execution, &ready, &new, &tms, tms_disp, &swap);
        // Se encola a Terminados
        enqueue(process_extracted, &finished);
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
        // Interpreta y ejecuta la instrucción
        result_interpretation = interpret_instruction(&gui, instruction, execution.head);

        if (result_interpretation != 0 && result_interpretation != -1) // Instrucción ejecutada correctamente
        {
          // Se incrementa PC
          (execution.head->PC)++;
          // Se incrementa el número de instrucciones ejecutadas (quantum)
          quantum++;
          // Se actualizan la impresión de los registros del proceso en ejecución
          print_processor(gui.inner_cpu, execution.head);
          // Se actualizan los valores de uso de CPU para el proceso en ejecución (KCPU)
          execution.head->KCPU += IncCPU; // Saltos de 15
          // También incrementa el uso de procesador por usuario al proceso en ejecución, para darle paso a otro que no se haya ejecutado todavia
          // Se quiere que todos los procesos del mismo usuario se vayan intercalando, no que se ejecuten secuencialemente (de a ratitos cada uno)
          execution.head->KCPUxU += IncCPU;
          /* Actualiza los contadores de uso del CPU para todos los procesos (no Terminados)
            del usuario dueño del proceso en ejecución */
          update_KCPUxU_per_process(execution.head->UID, &ready);
        }
        else // Se encontró la instrucción END o un error, en cuarquier caso terminar el proceso
        {
          if (result_interpretation == -1) // Hubo un error
            mvwprintw(gui.inner_msg, 1, 0, "Terminación anormal del programa.");
          else // Hubo un END
            mvwprintw(gui.inner_msg, 0, 0, "Programa finalizado correctamente.");

          // Se actualiza el número de usuarios (NumUs) y el peso W
          update_users(execution.head->UID, ready);
          // Se extrae nodo en ejecución
          PCB *process_extracted = dequeue(&execution);
          // El proceso termina y se realiza la gestión necesaria, una vez se saca de ejecución
          handle_process_termination(&gui, process_extracted, &execution, &ready, &new, &tms, tms_disp, &swap);
          // Se encola a Terminados
          enqueue(process_extracted, &finished);
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
            int previous_pid = execution.head->pid;
            // Se limpia subventana de mensajes
            werase(gui.inner_msg);
            // Sacar el único nodo en Ejecución, e insertarlo al final de Listos
            enqueue((dequeue(&execution)), &ready);
            // Actualiza los parámetros de planificación, para todos los nodos de la cola Listos
            update_parameters(&ready);
            // Mostrar el proceso extraído de Ejecución por 3 segundos y recalcular prioridades
            print_queues(gui.inner_queues, execution, ready, new, finished, lists_disp);
            // Recalcular prioridades de la cola de Listos
            recalculate_priorities(&gui, ready, &minor_priority);
            // Encolar el proceso de menor prioridad de Listos a Ejecución
            enqueue(extract_by_priority(minor_priority, &ready), &execution);
            // Se resetea el número de desplazamientos de la tmp cuando hay más de un proceso
            if (previous_pid != execution.head->pid)
              tmp_disp = 0;
          }
          /* Se reinicia para comenzar a ejecutar
          el siguiente proceso o el mismo si no hay más */
          quantum = 0;
        }
      }
      timer = init_timer;                                                          // Se reinicializa temporizador para volver a escribir en línea de comandos
      print_ginfo(gui.inner_ginfo, execution);                                     // Se imprime información general
      print_queues(gui.inner_queues, execution, ready, new, finished, lists_disp); // Se imprimen las colas en su ventana
      print_tmp(gui.inner_tmp, execution.head, tmp_disp);                          // Imprime el contenido de la TMP del proceso en ejecución con desplazamiento
      print_processor(gui.inner_cpu, execution.head);                              // Se imprime el contenido del CPU con el proceso en ejecución
      print_traduction(gui.inner_msg, execution.head);                             // Se imprime la traducción de la dirección virtual (PC), por la dirección real en SWAP
    }
    wmove(gui.inner_prompt, 0, PROMPT_START + index); // Se coloca el cursor en su lugar
    wrefresh(gui.inner_prompt);                       // Refresca las ventana de inner_prompt
  } while (!exited);

  fclose(swap); // Se cierra swap hasta el final de la ejecución del simulador
  endwin();     // Cierra la ventana
  printf("\n"); // Salto de línea en terminal
}

void initialize_ncurses_gui(GUI *gui)
{
  // Inicialización de parámetros de ncurses y GUI
  initscr();                       // Inicia la ventana
  start_color();                   // Permite el uso de colores
  use_default_colors();            // Permite usar -1 como fondo transparente
  noecho();                        // Evita la impresión automática de caracteres
  set_escdelay(10);                // Proporciona el tiempo de expiración del ESC en milisegundos
  initialize_gui(gui);             // Inicializar GUI
  keypad(gui->inner_prompt, true); // Habilita las teclas especiales después de inicializar la GUI
}