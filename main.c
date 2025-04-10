#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>
#include <string.h>
#include "pcb.h"
#include "queue.h"

int main(void)
{
  int exited = FALSE; // Indica si salió (ESC ó EXIT)
  char buffers[NUMBER_BUFFERS][SIZE_BUFFER] = {0}; // Buffers de historial
  int c = 0; // Carácter introducido por usuario
  int index = 0; // índice de buffer de prompt
  int result_interpretation = 0; // Retorno de interpretar la instrucción leída
  int index_history = 0; // Índice de buffer de historial seleccionado (↑ o ↓)
  char line[SIZE_LINE] = {0}; // Línea leída de archivo
  int quantum = 0; // Contador para el número de instrucciones antes de alcanzar el MAXQUANTUM
  int speed_level = 1; // Nivel de velocidad de lectura de instrucciones
  unsigned timer = 0; // Temporizador para determinar la velocidad de escritura
  unsigned init_timer = 0; // Inicializador para timer, reduce la brecha entre MAX_TIME
  //int file_counter = 0; // Contador de los programas diferentes cargados (Ejecución o Listos)
  int minor_priority = 0; // Variable para saber que nodo extraer de ready para mandarlo a ejecución
  int uid_execution = 0;  // Almacena el uid del proceso actualmente en ejecución
  // Se instancían las colas
  Queue execution;
  Queue ready;
  Queue finished;

  // Inicializar las colas
  initialize_queue(&execution);
  initialize_queue(&ready);
  initialize_queue(&finished);

  initscr(); // Inicia la ventana
  noecho(); // Evita la impresión automática de caracteres
  keypad(stdscr, TRUE); // Habilita las teclas especiales
  set_escdelay(10); // Proporciona el tiempo de expiración del ESC en milisegundos
  processor_template(); // Se imprime plantilla de la ventana del área del procesador
  messages_template(); // Se imprime plantilla de la ventana del área de mensajes
  print_queues(execution, ready, finished); // Se imprimen las colas en su área
  users_area(NumUs); // Se imprime la cantidad de programas cargados
  mvprintw(0, 2, "Artemisos>"); // Prompt fija donde se escriben los comandos
  do
  {
    if (!execution.head) // Si la cola Ejecución está vacía
    {
      // Gestor de comandos de terminal
      exited = command_handling(buffers, &c, &index, &index_history,
                                &execution, &ready, &finished,
                                &timer, &init_timer, &speed_level);
      if (ready.head) // Verifica si hay nodos en la cola Listos
      {
        //enqueue(dequeue(&ready), &execution); // Se extrae el primer nodo y se pasa a Ejecución
        // Ahora ya no se extrae el primer nodo de listos, se busca el de menor prioridad
        minor_priority = get_minor_priority(ready);
        enqueue(get_priority_pcb(minor_priority, &ready),&execution);
        quantum = 0; // Se establece en 0 para el proceso que acaba de entrar
      }
    }
    else // Hay un nodo en Ejecución
    {
      if (timer <= MAX_TIME) // Permite escribir en la prompt antes de que se alcance el MAX_TIME
      {
        // Gestor de comandos de terminal
        exited = command_handling(buffers, &c, &index, &index_history,
                                  &execution, &ready, &finished,
                                  &timer, &init_timer, &speed_level);
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
          // Se limpia el área de mensaje
          clear_messages();
          // Se verifica que no hay otro proceso del mismo usuario en Listos
          if(!search_uid(execution.head->UID, ready)) {
            NumUs--;
            // Se decrementa el peso por usuarios en sesion, ya se fue el bro
            W--;    
          }
          // Se extrae proceso de Ejecución y se encola a Terminados
          enqueue(dequeue(&execution), &finished);
          mvprintw(15, 4, "Terminación anormal del programa..");   
          // Se limpia el área de procesador
          processor_template();
        }
        else // Si hay más líneas por leer
        {
          // Línea sin salto de línea o retorno de carro
          line[strcspn(line, "\r\n")] = '\0';
          // Interpreta y ejecuta la instrucción
          result_interpretation = interpret_instruction(line, execution.head);
          if (result_interpretation != 0 && result_interpretation != -1) // Instrucción ejecutada correctamente
          {
            // Si se leyó salto de línea, solo se incremente quantum
            if (result_interpretation != 2)
            {
              (execution.head->PC)++; // Se incrementa PC
            }
            // Se incrementa el número de instrucciones ejecutadas (quantum)
            quantum++;
            // Se actualizan la impresión de los registros del pcb en ejecución
            print_registers(*execution.head);
            
            // HAY UN PROCESO EN EJECUCIÓN Y AÚN NO TERMINA SU QUANTUM
            // Se actualizan los valores de uso de CPU para el proceso en ejecución (KCPU)
            execution.head-> KCPU += IncCPU;  // Va dando saltos de 15 en 15
            // Se obtiene el uid del proceso actualmente en ejecución
            uid_execution = execution.head -> UID;
            // Se actualizan los contadores de uso de cpu (KCPUxU) para TODOS los procesos 
            // no terminados (ready y execution) del mismo usuario. KCPUxU también dará saltos de 15 en 15
            // Hacer función de reccorrer la cola listos y en todos los procesos con uid parar a atualizar
          }
          else // Se encontró la instrucción END o un error, en cuarquier caso terminar el pcb
          {
            // Cierra el archivo
            fclose(execution.head->program);
            // Evita puntero colgante
            execution.head->program = NULL;
            if (result_interpretation == -1) // Hubo un error
            {
              mvprintw(15, 4, "Terminación anormal del programa.");
            }
            else // Hubo un END
            {
              mvprintw(15, 4, "Programa finalizado correctamente.");
            }
            // Se verifica que no hay otro proceso del mismo usuario en Listos
            if(!search_uid(execution.head->UID, ready)) {
              NumUs--;
              // Se decrementa el peso por usuarios en sesion, ya se fue el bro
              W--;    
            }
            // Se extrae nodo en ejecución y encola en Terminados
            enqueue(dequeue(&execution), &finished);
            // Se limpia el área de procesador
            processor_template();
          }
          if (quantum == MAXQUANTUM) // quantum llegó a MAXQUANTUM (4) y aún no se llega al fin del archivo
          {
            if (ready.head) // Se verifica si hay proceso en Listos
            {
              // Sacar el único nodo en Ejecución, e insertarlo al final de Listos
             enqueue((dequeue(&execution)), &ready);

              // Sacar un nodo de Listos y pasarlo a Ejecución
              //enqueue((dequeue(&ready)), &execution);
              // Ya no se saca el primer nodo de listos
              // Ahora ya no se extrae el primer nodo de listos, se busca el de menor prioridad
              minor_priority = get_minor_priority(ready);
              mvprintw(40,3,"                                                                            ");
              mvprintw(40,3,"Nodo de menor prioridad que entró a ejecución %d",minor_priority);
              enqueue(get_priority_pcb(minor_priority, &ready),&execution);
            }
            /* Se reinicia para comenzar a ejecutar
            el siguiente proceso o el mismo si no hay más */
            quantum = 0;
          }
        }
        timer = init_timer; // Se reinicia temporizador para volver a escribir en línea de comandos
        users_area(NumUs); // Se imprime la cantidad de programas cargados
        print_queues(execution, ready, finished); // Se imprimen las colas en su área
      }
    }
    move(0, 12 + (index)); // Se coloca el cursor en su lugar
    refresh(); // Refresca la ventana
  } while (!exited);
  endwin();	// Cierra la ventana
  printf("\n"); // Salto de línea en terminal
  return 0;
}
