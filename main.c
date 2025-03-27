#include <stdio.h>                                  
#include <unistd.h>
#include <ncurses.h>
#include "pcb.h"

/*
    PROTPTIPO: int main(void);
    
    PROPOSITO: Establece el flujo de control de todo el programa como tal,
    y no es una función que invoque.
    
    ENTRADAS: «void» asegura que la función no reciba ningún parámetro de entrada.
    
    SALIDAS: «0» que significa que todo salió bien. 

    DESCRIPCIÓN: En esta función se declaran las variables de control
    (buffers, pcb, read flag, etc.). También es la encargada de comprobar 
    el estado de la bandera de lectura de archivo. Si la bandera de lectura no 
    está activa, esta función permite la captura de la línea de comandos 
    (command_handling) para su posterior interpretación. Además, cuando la bandera 
    de lectura se activa, llama a la función read_line para procesar el archivo 
    línea por línea.
    
*/

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

    // Inicializar los registros
    initialize_pcb(&pcb);

    initscr();
    noecho(); // Evita la impresión automática de caracteres       
    keypad(stdscr, TRUE); // Habilita las teclas especiales
    set_escdelay(10); // Proporciona el tiempo de expiración del ESC en milisegundos
    print_processor();
    print_messages();
    mvprintw(0,2,"Artemisos>");
    while (TRUE) {
        if (!read_flag) {
            // Gestor de comando
            command_handling(buffers, &read_flag, &c, &index, &f, &index_history);
        } else {
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
            move(0,12); // Se coloca el cursor en su lugar
        }
        refresh();
    }
    return 0;
}
