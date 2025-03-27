#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>
#include <string.h>
#include "pcb.h"
#include "queue.h"

int main(void)
{
    int exited = FALSE;                              // Indica si salió (ESC ó EXIT)
    char buffers[NUMBER_BUFFERS][SIZE_BUFFER] = {0}; // Buffers de historial
    int c = 0;                                       // Carácter introducido por usuario
    int index = 0;                                   // índice de buffer de prompt
    int result_interpretation = 0;                   // Retorno de interpretar la instrucción leída
    int index_history = 0;                           // Índice de buffer de historial seleccionado (↑ o ↓)
    char line[SIZE_LINE] = {0};                      // Línea leída de archivo
    int quantum = 0;                                 // Contador para el número de instrucciones antes de alcanzar el MAXQUANTUM
    unsigned timer = 0;                              // Temporizador para determinar la velocidad de escritura
    unsigned init_timer = 0;                         // Valor inicial para temporizador, ajusta velocidad

    // Se instancían las colas
    Queue execution;
    Queue ready;
    Queue finished;

    // Inicializar las colas
    initialize_queue(&execution);
    initialize_queue(&ready);
    initialize_queue(&finished);

    initscr();
    noecho();                                // Evita la impresión automática de caracteres
    keypad(stdscr, TRUE);                    // Habilita las teclas especiales
    set_escdelay(10);                        // Proporciona el tiempo de expiración del ESC en milisegundos
    processor_template();                    // Se imprime plantilla de la ventana del área del procesador
    messages_template();                     // Se imprime plantilla de la ventana del área de mensajes
    queues_area(execution, ready, finished); // Se imprime plantilla de la ventana del área de cola
    mvprintw(0, 2, "Artemisos>");            // Prompt fija donde se escriben los comandos
    do
    {
        if (!execution.head) // Si la cola Ejecución está vacía
        {
            // Gestor de comandos de terminal
            exited = command_handling(buffers, &c, &index, &index_history,
                                        &execution,&ready, &finished, 
                                        &timer, &init_timer);
            if (ready.head) // Verifica si hay nodos en la cola Listos
            {
                enqueue(dequeue(&ready), &execution); // Se extrae el primer nodo y se pasa a Ejecución
                quantum = 0;                          // Se establece en 0 para el proceso que acaba de entrar
            }
        }
        else
        {
            if (timer <= MAX_TIME) // Permite escribir en la prompt antes de que se alcance el MAX_TIME
            {
                exited = command_handling(buffers, &c, &index, &index_history,
                                             &execution,&ready, &finished,
                                             &timer, &init_timer);
            }
            else // Si se alcanzó el MAX_TIME se ejecuta la instrucción
            {
                /* Si no hay más líneas del archivo por leer, esto es,
                   si NO se encuentra END y no hay ningún error */
                if (!read_line(&execution.head->program, line))
                {
                    fclose(execution.head->program); // Cierra el archivo
                    execution.head->program = NULL;  // Evita puntero colgante
                    // Se limpia el área de mensaje
                    clear_messages();
                    // Se extrae proceso de Ejecución y se encola a Terminados
                    enqueue(dequeue(&execution), &finished);
                    mvprintw(15, 4, "Terminación anormal del programa..");
                    // Se limpia el área de procesador
                    processor_template();
                }
                else { // Si hay más líneas por leer
                    // Línea sin salto de línea o retorno de carro
                    line[strcspn(line, "\r\n")] = '\0'; 
                    // Interpreta y ejecuta la instrucción
                    result_interpretation = interpret_instruction(line, execution.head);
                    if (result_interpretation != 0 && result_interpretation != -1 ) // Instrucción ejecutada correctamente
                    {
                        (execution.head->PC)++; // Se incrementa PC
                        // Se incrementa el número de instrucciones ejecutadas (quantum)
                        quantum++;
                        // Se actualizan la impresión de los registros del pcb en ejecución
                        print_registers(*execution.head);
                    }
                    else // Se encontró la instrucción END o un error
                    {
                        fclose(execution.head->program); // Cierra el archivo
                        execution.head->program = NULL;  // Evita puntero colgante
                        if (result_interpretation == -1) // Hubo un error
                        {
                            mvprintw(15, 4, "Terminación anormal del programa.");
                        }
                        else // Hubo un END
                        {
                            mvprintw(15, 4, "Programa finalizado correctamente.");
                        }
                        // Se extrae nodo en ejecución y encola en Terminados
                        enqueue(dequeue(&execution), &finished);
                        processor_template();
                    } 
                    if (quantum == MAXQUANTUM) // quantum llegó a MAXQUANTUM (4) y aún no se llega al fin del archivo
                    {
                        if (ready.head) // Se verifica si hay proceso en Listos
                        {
                            // Sacar el único nodo en Ejecución, e insertarlo al final de Listos
                            enqueue((dequeue(&execution)), &ready);
                            // Sacar un nodo de Listos y pasarlo a Ejecución
                            enqueue((dequeue(&ready)), &execution);
                        }
                        /* Se reinicia para comenzar a ejecutar
                        el siguiente proceso o el mismo si no hay más */
                        quantum = 0;
                    }
                    
                }
                timer = init_timer;                      // Se reinicia temporizador para volver a escribir en línea de comandos
                queues_area(execution, ready, finished); // Se imprimen las colas en su área
            }
        }
        move(0, 12 + (index)); // Se coloca el cursor en su lugar
        refresh();
    } while (!exited);
    endwin(); // Cierra la ventana
    printf("\n"); // Salto de línea en terminal
    return 0;
}
