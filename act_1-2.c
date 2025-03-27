#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "kbhit.h"

// Valores numéricos obtenidos de kbwhat.c
#define ENTER 10
#define BACKSPACE 127

#define SIZE_BUFFER 256
#define SIZE_LINE 128

/* Delay de N milisegundos que se detendrá
 * la ejecución del programa entre cada iteración */
#define N 300

// Valores booleanos
#define TRUE 1
#define FALSE 0

// Prototipos
void print_buffer(int *index, char *buff);
void initialize_buffer(char *buff, int *index);
int command_handling(char *buff, int *read_flag, char *c, int *index, FILE **f);
int evaluate_command(char *buff, int *index, int *read_flag, FILE **f);
int read_line(FILE **f, char *line);

int main(void)
{
    char buff[SIZE_BUFFER];      // Buffer que almacena teclas precionadas
    char prev_buff[SIZE_BUFFER]; // Buffer auxiliar previo al buffer actual 
    int read_flag = FALSE;       // Bandera de lectura de archivo
    char c = 0;                  // Carácter ingresado por usuario
    int index = 0;               // índice del buffer
    char line[256];              // Línea leída desde el archivo

    // Inicialización de buffers
    initialize_buffer(buff, &index);    
    prev_buff[0] = -1;

    FILE *f = NULL;
   
    while(TRUE) {
        if (read_flag == TRUE) {
            // ¿Se encontró el fin de archivo? Cambia la bandera a disponible
            if(!read_line(&f, line)) {
                fclose(f); 
                read_flag = FALSE;   
            } else {
                printf("Artemisos>\n%s\n", line);
            }
        } else {
            /* - Si buff y prev_buff son iguales, significa que no
             *   se ha ingresado un carácter.
             *
             * - Si buff y prev_buff son distintos, significa que
             *   se ha ingresado un carácter. */
            if(strcmp(buff, prev_buff)) { // Si son distintos
                print_buffer(&index, buff);
                strcpy(prev_buff, buff);
            }
            command_handling(buff, &read_flag, &c, &index, &f);
        }
        //usleep(N);
    }
    return 0;
}

void print_buffer(int *index, char *buff)
{
    printf("\nArtemisos>");
    for (int i = 0; i < *index; i++) {
        printf("%c", buff[i]);
    }
}

void initialize_buffer(char *buff, int *index)
{
    for(int i = 0; i < SIZE_BUFFER; i++) {
        buff[i] = 0;
    }
    *index = 0;
}

int command_handling(char *buff, int *read_flag, char *c, int *index, FILE **f)
{
    if (kbhit()) {
            *c = getchar();
            if (*c == ENTER) {
                buff[*index] = '\0';
                evaluate_command(buff, index, read_flag, f); // Se evalua el comando en buffer   
            } else if(*c == BACKSPACE) {
                // Se elimina el último carácter del buffer
                if (*index > 0) {
                    (*index)--;
                    buff[*index] = '\0';
                    
                } else {
                    buff[*index] = '\0';
                }
                
            } else {
                // Concatena la tecla presionada al buffer, para su posterior impresión
                buff[*index] = *c;
                (*index)++;    
            }
            
    }
    return 0;
}

int evaluate_command(char *buff, int *index, int *read_flag, FILE **f)
{
    char command[30], parameter1[30], parameter2[30];
    
    // Parámetros vacíos
    parameter1[0] = 0;
    parameter2[0] = 0;

    sscanf(buff, "%s %s", command, parameter1);

    if(!(strcmp(command, "EXIT")) && !parameter1[0]) {
        printf("\n");
        exit(0);
    } else {
        if(!(strcmp(command, "LOAD"))) {
            if(parameter1[0]) {   // Se indicó un archivo a cargar  
                *f = fopen(parameter1, "r");
                if(*f) {
                    printf("\nComando: %s Nombre de archivo: %s\n", command, parameter1);
                    *read_flag = TRUE; // Enciende la bandera de lectura
                } else {
                    printf("\nError: archivo no existe.\n");
                } 
            } else {
                printf("\nError: faltan parámetros."
                        "\nSintaxis: LOAD «archivo»");
            }   
        } else {
            printf("\nError: comando no encontrado.\n");
        }  
    }
    initialize_buffer(buff, index); // Limpia el buffer 

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
