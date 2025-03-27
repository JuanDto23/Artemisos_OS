#include <stdio.h>                                  
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include "kbhit.h"

// Valores numéricos obtenidos de kbwhat.c
#define ENTER 10
#define BACKSPACE 127

#define NUMBER_BUFFERS 3
#define SIZE_BUFFER 256
#define SIZE_LINE 16

/* Delay de N milisegundos que se detendrá
 * la ejecución del programa entre cada iteración */
#define N 300

// Valores booleanos
#define TRUE 1
#define FALSE 0

// Prototipos
int command_handling(char buffers[NUMBER_BUFFERS][SIZE_BUFFER], int *read_flag, char *c, int *index, FILE **f);
int evaluate_command(char *buff, int *index, int *read_flag, FILE **f);
int read_line(FILE **f, char *line);
void clear_prompt(int row);

void print_registers(void)
{
    mvprintw(4,2, "----------------------------------PROCESADOR-------------------------------------");
    mvprintw(5,2, "- AX:                                PC:                                        -"); 
    mvprintw(6,2, "- BX:                                                                           -");
    mvprintw(7,2, "- CX:                                IR:                                        -");
    mvprintw(8,2, "- DX:                                                                           -");
    mvprintw(9,2, "---------------------------------------------------------------------------------");
    refresh();
}

void print_messages(void)
{
    mvprintw(11,2, "-----------------------------------MENSAJES--------------------------------------");
    mvprintw(12,2, "-                                                                               -");
    mvprintw(13,2, "-                                                                               -");
    mvprintw(14,2, "-                                                                               -");
    mvprintw(15,2, "-                                                                               -");
    mvprintw(16,2, "-                                                                               -");
    mvprintw(17,2, "---------------------------------------------------------------------------------");
    refresh();
}

void print_history(char buffers[NUMBER_BUFFERS][SIZE_BUFFER])
{
    for (int i = 1; i < NUMBER_BUFFERS; i++) {
        if (buffers[i][0] != 0) {
            clear_prompt(i);
            mvprintw(i,2,"Artemisos>%s",buffers[i]);
        }
    }
}

void initialize_buffer(char *buffer, int *index)
{
    for (int i = 0; i < SIZE_BUFFER; i++) {
        buffer[i] = 0;
    }
    *index = 0;
}

void to_uppercase(char *s) 
{
    for (int i = 0; s[i] != '\0'; i++) {
        if (s[i] >= 'a' && s[i] <= 'z') {
            s[i] = s[i] - 32;
        }
    }
}

void clear_prompt(int row) 
{
    mvprintw(row,2,"Artemisos>");
    for (int i = 0; i < 128; i++) {
        mvaddch(row,12+i,' ');
    }
}

/* ------------------------------------ FUNCIÓN MAIN ------------------------------------*/
int main(void)
{
    char buffers[NUMBER_BUFFERS][SIZE_BUFFER] = {0};
    int read_flag = FALSE;
    char c = 0;
    int index = 0;
    char line[SIZE_LINE] = {0};
    FILE *f;

    initscr();
    noecho();
    print_registers();
    print_messages();
    mvprintw(0,2,"Artemisos>");
    while (TRUE) {
        if (!read_flag) {
            command_handling(buffers, &read_flag, &c, &index, &f);
        } else {
            if (!read_line(&f, line)) {
                fclose(f);
                read_flag = FALSE;    
                mvprintw(12,2, "-                                                                               -");
                mvprintw(13,2, "-                                                                               -");
                move(0,12); // Se coloca el cursor en su lugar
            } else {
                mvprintw(7,43,"%s",line);
                mvprintw(7,55,"                           -");
                move(0,12); // Se coloca el cursor en su lugar
                sleep(2);
            }
        }
        refresh();
    }
    return 0;
}

int command_handling(char buffers[NUMBER_BUFFERS][SIZE_BUFFER], int *read_flag, char *c, int *index, FILE **f)
{
    if (kbhit()) {
            *c = getch();
            if (*c == ENTER) {                                                                
                buffers[0][*index] = '\0';
                evaluate_command(buffers[0], index, read_flag, f); // Se evalua el comando en buffer
                // Se crea historial
                strcpy(buffers[2],buffers[1]);
                strcpy(buffers[1],buffers[0]);
                // Se limpia el buffer y la prompt
                initialize_buffer(buffers[0], index);
                clear_prompt(0);
                // Se imprimen los buffers de historial
                print_history(buffers);
                move(0,12); // Se coloca el cursor en su lugar
            } else if (*c == BACKSPACE) {
                // Se elimina el último carácter del buffer
                if (*index > 0) {
                    (*index)--;
                    move(getcury(stdscr), getcurx(stdscr) - 1); // Mueve el cursor hacia atrás
                    delch();  // Elimina el carácter en la posición actual
                }
            } else {
                // Concatena la tecla presionada al buffer, para su posterior impresión
                buffers[0][(*index)++] = *c;
                mvaddch(0,11+(*index),*c);
            }
    }
    return 0;
}

int evaluate_command(char *buffer, int *index, int *read_flag, FILE **f)
{
    char command[32], parameter1[64], parameter2[64];

    // Parámetros vacíos
    parameter1[0] = 0;
    parameter2[0] = 0;

    // Se convierte a mayúsculas
    to_uppercase(buffer);

    sscanf(buffer, "%s %s", command, parameter1);

    if(!(strcmp(command, "EXIT")) && !parameter1[0]) {
        printf("\n");
        endwin(); // Cierra la ventana
        exit(0);
    } else {
        mvprintw(12,2, "-                                                                               -");
        if(!(strcmp(command, "LOAD"))) {
            if(parameter1[0]) {   // Se indicó un archivo a cargar
                *f = fopen(parameter1, "r");
                if(*f) {
                    mvprintw(12,4,"Comando: %s Nombre de archivo: %s", command, parameter1);
                    mvprintw(13,4,"Leyendo...");
                    *read_flag = TRUE; // Enciende la bandera de lectura
                } else {
                    mvprintw(12,4,"Error: archivo no existe.");
                }
            } else {
                mvprintw(12,4,"Error: faltan parámetros. Sintaxis: LOAD «archivo»");
            }
        } else {
            mvprintw(12,4,"Error: \"%s\" no es comando válido.", buffer);
        }
    }
    return 0;
}

int read_line(FILE **f, char *line)
{
    if(!feof(*f)) {
        line = fgets(line, SIZE_LINE, *f);
        return 1;   // Hay todavía líneas por leer
    } else {
        return 0; // No hay líneas por leer
    }
}

