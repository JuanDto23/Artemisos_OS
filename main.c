#include <stdio.h>                                  
#include <unistd.h>
#include <ncurses.h>
// Bibliotecas propias
#include "pcb.h"

int main(void)
{
    char buffers[NUMBER_BUFFERS][SIZE_BUFFER] = {0};    // Buffers de historial
    int read_flag = FALSE;                              // Bandera de lectura de archivo 
    int c = 0;                                          // Carácter introducido por usuario
    int index = 0;                                      // índice de buffer de prompt
    int result_interpretation = 0;                      // Retorno de interpretar la instrucción leída
    int index_history = 0;                              // Índice de buffer de historial seleccionado (↑ o ↓)
    char line[SIZE_LINE] = {0};                         // Línea leída de archivo
    FILE *f = NULL;                                     // Puntero a archivo abierto en modo lectura
    PCB pcb;                                            // Process Control Process
    int pid = 1;                                        // Contador que identifica cada pcb
    int processor_busy = FALSE;                         // Bandera que indica que el procesador está ocupado
    PCB *detached_pcb = NULL;                           // Nodo pcb desligado al hacer PULL y que se encuentra en el procesador

    // Inicializar los registros
    initialize_pcb(&pcb);

    initscr();
    noecho(); // Evita la impresión automática de caracteres       
    keypad(stdscr, TRUE); // Habilita las teclas especiales
    set_escdelay(10); // Proporciona el tiempo de expiración del ESC en milisegundos
    processor_template(); // Se imprime plantilla de la ventana del área del procesador
    messages_template(); // Se imprime plantilla de la ventana del área de mensajes
    queue_template(); // Se imprime plantilla de la ventana del área de cola
    mvprintw(0,2,"Artemisos>"); // Prompt fija donde se escriben los comandos
    while (TRUE) {
        if (!read_flag) {
            // Gestor de comando
            command_handling(buffers, &read_flag, &c, &index, &f, &index_history, 
                &pid, &processor_busy, &detached_pcb);
        } else {
            command_handling(buffers, &read_flag, &c, &index, &f, &index_history, 
                &pid, &processor_busy, &detached_pcb);
            if (!read_line(&f, line)) {
                fclose(f);
                read_flag = FALSE;
                clear_messages(); // Se limpia el área de mensajes
                pcb.PC = 0; // Se reinicia el PC
            } else {
                mvprintw(7,43,"%s",line); // Se imprime línea en IR
                mvprintw(7,55,"                           -");
                result_interpretation = interpret_instruction(line, &pcb);
                // Instrucción ejecutada correctamente
                if (result_interpretation != 0 && result_interpretation != -1) {
                    (pcb.PC)++; // Se incrementa PC
                    print_registers(&pcb);
                    sleep(1);
                } else { // Se encontró la instrucción END o un error
                    fclose(f);
                    read_flag = FALSE; 
                    if (result_interpretation == -1) { // Hubo un error
                        mvprintw(13,4,"Terminación anormal del programa.");  
                    } else { // Hubo un END
                        mvprintw(12,4,"Programa finalizado correctamente.");
                    }
                    pcb.PC = 0; // Se reinicia el PC
                }     
            }
            move(0,12+(index)); // Se coloca el cursor en su lugar
        }
        refresh();
    }
    return 0;
}