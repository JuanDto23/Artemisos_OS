#include <stdio.h>                                  
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <ctype.h>

#include "kbhit.h"
#include "pcb.h"

void initialize_pcb(PCB *pcb)
{
    pcb -> AX = 0;
    pcb -> BX = 0;
    pcb -> CX = 0;
    pcb -> DX = 0;
    pcb -> PC = 0;
    pcb -> IR[0] = '\0';
}

void initialize_buffer(char *buffer, int *index)
{
    for (int i = 0; i < SIZE_BUFFER; i++) {
        buffer[i] = 0;
    }
    *index = 0;
}

void strUpper(char *s)
{
    for (int i = 0; s[i] != '\0'; i++) {
        if (s[i] >= 'a' && s[i] <= 'z') {
            s[i] = s[i] - 32;
        }
    }
}

int isNumeric(char *str) 
{
    // Verificar si la cadena es nula
    if (str == NULL) {
        return FALSE;
    }

    // Manejar el caso de un número negativo
    if (*str == '-') {
        str++;  //avanza al siguiente caracter
    }

    // Si el siguiente carácter no es número, regresa 0
    if (*str == '\0') {
        return FALSE;
    }

    // Verificar cada carácter de la cadena
    while (*str) {
        if (!isdigit(*str)) {
            return FALSE; // Si se encuentra un carácter no numérico, regresar 0
        }
        str++;
    }
    return TRUE; // Si todos los caracteres son dígitos, regresar 1
}

void clear_prompt(int row) 
{
    mvprintw(row,2,"Artemisos>");
    for (int i = 0; i < 128; i++) {
        mvaddch(row,12+i,' ');
    }
}

void clear_messages(void)
{
    mvprintw(12,2, "-                                                                               -");
    mvprintw(13,2, "-                                                                               -");
    mvprintw(14,2, "-                                                                               -");
    mvprintw(15,2, "-                                                                               -");
    mvprintw(16,2, "-                                                                               -");
}

int search_register(char * p)
{
    if (!strcmp(p, "AX")){
        return 'A';
    } else if (!strcmp(p, "BX")) {
        return 'B';
    } else if (!strcmp(p, "CX")) {
        return 'C';
    } else if (!strcmp(p, "DX")) {
        return 'D';
    } else {
        return -1; // Registro no indentificado
    }
}

int value_register(const PCB *pcb, char r)
{
	if (r == 'A') {
		return pcb->AX;
	} else if (r == 'B') {
		return pcb->BX;
	} else if (r == 'C') {
		return pcb->CX;	
	} else if (r == 'D') {
		return pcb->DX;
	}
	return -1;
}


int command_handling(char buffers[NUMBER_BUFFERS][SIZE_BUFFER], 
    int *read_flag, int *c, int *index, FILE **f, int *index_history)
{
    if (kbhit()) {
        *c = getch();
        if (*c == ENTER) {                                                                
            buffers[0][*index] = '\0';
            evaluate_command(buffers[0], index, read_flag, f); // Se evalua el comando en buffer
            // Se crea historial
            strcpy(buffers[3],buffers[2]);
            strcpy(buffers[2],buffers[1]);
            strcpy(buffers[1],buffers[0]);
            // Se Reinicia índice de historial
            *index_history = 0;
            // Se limpia el buffer y la prompt
            initialize_buffer(buffers[0], index);
            clear_prompt(0);
            // Se imprimen los buffers de historial
            print_history(buffers);
            move(0,12); // Se coloca el cursor en su lugar
        } else if (*c == KEY_BACKSPACE) {
            // Se elimina el último carácter del buffer
            if (*index > 0) {
                (*index)--;
                move(getcury(stdscr), getcurx(stdscr) - 1); // Mueve el cursor hacia atrás
                delch();  // Elimina el carácter en la posición actual
            }
        } else if (*c == KEY_UP) {  // Avanza a buffers superiores
            /*  Se comprueba que no se salga del arreglo de buffers y que el
                el comando superior no sea nulo */
            if (*index_history >  0 && buffers[*index_history - 1][0] != '\0') {
                // Se compia el comando superior
                strcpy(buffers[0], buffers[*index_history -1]);
                (*index_history)--;
                // Se limpia la prompt y se imprime el buffer prompt con el comando superior
                clear_prompt(0);
                mvprintw(0,12, "%s", buffers[0]);
                // Se mueve el índice del buffer prompt para seguir escribiendo
                *index = strlen(buffers[0]);
                move(0, 12 + *index);
            }
        } else if (*c == KEY_DOWN) { // Comando anterior (avanza a buffers inferiores)
            /*  Se comprueba que no se salga del arreglo de buffers y que el
                el comando anterior no sea nulo */
            if (*index_history <  NUMBER_BUFFERS - 1 && buffers[(*index_history) + 1][0] != '\0') {
                // Se compia el comando anterior
                strcpy(buffers[0], buffers[*index_history + 1]);
                (*index_history)++;
                // Se limpia la prompt y se imprime el buffer prompt con el comando anterior
                clear_prompt(0);
                mvprintw(0,12,"%s",buffers[0]);
                // Se mueve el índice del buffer prompt para seguir escribiendo
                *index = strlen(buffers[0]);
                move(0, 12 + *index);
            }
        } else if (*c == ESC) { // Acceso rápido para salir del programa con ESC
            clear_messages(); // Se limpia el área de mensajes
            mvprintw(12,4,"¿Seguro que quieres salir? [S/N]: ");
            char confirmation = toupper(getch());
            addch(confirmation);
            if (confirmation == 'S'){
                endwin();
                exit(0); 
            } else {
                clear_messages(); // Se limpia el área de mensajes
                move(0,12); // Se coloca el cursor en su lugar
            }
        } else {
            // Concatena la tecla presionada al buffer para su posterior impresión
            buffers[0][(*index)++] = *c;
            mvaddch(0,11+(*index),*c);
        }
    }
    return TRUE;
}

int evaluate_command(char *buffer, int *index, int *read_flag, FILE **f)
{
    /* TOKENS */
    char command[32] = {0}; 
    char parameter1[128] = {0};
    char parameter2[128] = {0};

    // Se convierte a mayúsculas
    strUpper(buffer);

    // Se separa en tokens el comando leído de prompt
    sscanf(buffer, "%s %s", command, parameter1);

    if (!(strcmp(command, "EXIT")) && !parameter1[0]) {
        printf("\n");
        endwin(); // Cierra la ventana
        exit(0);
    } else {
        clear_messages(); // Se limpia el área de mensajes
        if (!(strcmp(command, "LOAD"))) {
            if (parameter1[0]) {   // Se indicó un archivo a cargar
                *f = fopen(parameter1, "r");
                if(*f) {
                    mvprintw(12,4,"Nombre de archivo: %s", parameter1);
                    mvprintw(13,4,"Leyendo...");
                    *read_flag = TRUE; // Enciende la bandera de lectura
                } else {
                    mvprintw(12,4,"Error: archivo no existe.");
                }
            } else {
                mvprintw(12,4,"Error: faltan parámetros.");
                mvprintw(13,4,"Sintaxis: LOAD «archivo»");
            }
        } else {
            mvprintw(12,4,"Error: \"%s\" no es comando válido.", buffer);
        }
    }
    return TRUE;
}

int read_line(FILE **f, char *line)
{
    line = fgets(line, SIZE_LINE, *f);
    if(!feof(*f)) {
        strUpper(line); // Convierte la intrucción leída en mayúsculas
        return 1;   // Hay todavía líneas por leer
    } else {
        return 0; // No hay líneas por leer
    }
}

int interpret_instruction(char *line, PCB *pcb)
{   
    /* TOKENS */
    char instruction[32] = {0};  // Instrucción leída de línea
    char p1[32] = {0};           // Primer parámetro (registro)
    char p2[32] = {0};          // Segundo parámetro (registro o número)
    int number_p2 = 0;          // Valor numérico si p2 es un número

    int is_num_p2 = FALSE; // Bandera que indica si p2 es numérico

    // Se pasa a mayúsculas la instrucción leída
    strUpper(line);

    // Se separa en tokens la línea leída y se calcula cuántos hay
    int items = sscanf(line, "%s %s %s", instruction, p1, p2);
    
    // Se comprueba si la instrucción es END
    if (!strcmp(instruction, "END")) {
        clear_messages();
        return 0; // La instrucción leída es END
    }

    // Se comprueba si el segundo parámetro existe y es un número
    is_num_p2 = isNumeric(p2);
    if (is_num_p2) {
        number_p2 = atoi(p2); // Se extrae número del segundo parámetro
    }
    
    // Se comprueba que el primer parámetro sea un registro válido
    int reg1 = search_register(p1);
    if (reg1 != -1) {
        if (!is_num_p2 && p2[0] != '\0') { // El segundo parámetro es un registro
            /*  Se comprueba a qué registro corresponde p2.
                Si sí lo es, se obtiene valor del registro correspondiente. */
            switch (search_register(p2)) {
                case 'A':
                    number_p2 = value_register(pcb, 'A');
                    break;
                case 'B':
                    number_p2 = value_register(pcb, 'B');
                    break;
                case 'C':
                    number_p2 = value_register(pcb, 'C');
                    break;
                case 'D':                        
                    number_p2 = value_register(pcb, 'D');
                    break;                       
                default:
                    clear_messages(); // Se limpia el área de mensajes
                    mvprintw(12,4,"Error: registro inválido \"%s\"", p2);
                    return -1; // El segundo parámetro es un registro inválido
                    break;                        
            }
        }
        if (items == 3) { // Instrucciones de 3 paŕametros (MOV, ADD, SUB, MUL, DIV)
            if (!strcmp(instruction,"MOV")){
                if (reg1 == 'A') {          
                    pcb->AX = number_p2;
                } else if (reg1 == 'B') {      
                    pcb->BX = number_p2;
                } else if (reg1 == 'C') {      
                    pcb->CX = number_p2;
                } else {                           
                    pcb->DX = number_p2;    
                }
            } else if (!strcmp(instruction,"ADD")) {
                if (reg1 == 'A'){         
                    pcb->AX += number_p2;
                } else if (reg1 == 'B') {     
                    pcb->BX += number_p2;
                } else if (reg1 == 'C') {      
                    pcb->CX += number_p2;
                } else {
                    pcb->DX += number_p2;    
                }
            } else if (!strcmp(instruction,"SUB")) {
                if (reg1 == 'A') {          
                    pcb->AX -= number_p2;
                } else if (reg1 == 'B') {      
                    pcb->BX -= number_p2;
                } else if (reg1 == 'C') {      
                    pcb->CX -= number_p2;
                } else {                           
                    pcb->DX -= number_p2;    
                }
            } else if(!strcmp(instruction,"MUL")) {
                if (reg1 == 'A') {          
                    pcb->AX *= number_p2;
                } else if (reg1 == 'B') {      
                    pcb->BX *= number_p2;
                } else if (reg1 == 'C') {      
                    pcb->CX *= number_p2;
                } else {                           
                    pcb->DX *= number_p2;    
                }
            } else if(!strcmp(instruction,"DIV")) {
                if (reg1 == 'A') {          
                    pcb->AX /= number_p2;
                } else if (reg1 == 'B') {      
                    pcb->BX /= number_p2;
                } else if (reg1 == 'C') {      
                    pcb->CX /= number_p2;
                } else {                           
                    pcb->DX /= number_p2;    
                }
            } else {
                clear_messages(); // Se limpia el área de mensajes
                mvprintw(12,4,"Error: instrucción inválida \"%s\"", instruction);
                return -1; // Instrucción no encontrada
            }   
        } else if(items == 2) { // Instrucciones de 2 paŕametros (INC, DEC)
            if (!strcmp(instruction,"INC")) {
                if (reg1 == 'A') {          
                    pcb->AX += 1;
                } else if (reg1 == 'B') {      
                    pcb->BX += 1;
                } else if (reg1 == 'C') {      
                    pcb->CX += 1;
                } else{                           
                    pcb->DX += 1;    
                }
            } else if (!strcmp(instruction,"DEC")) {
                if (reg1 == 'A'){          
                    pcb->AX -= 1;
                } else if (reg1 == 'B') {      
                    pcb->BX -= 1;
                } else if (reg1 == 'C') {      
                    pcb->CX -= 1;
                } else{                           
                    pcb->DX -= 1;    
                }
            }
            else {
                clear_messages(); // Se limpia el área de mensajes
                mvprintw(12,4,"Error: instrucción inválida \"%s\"", instruction);
                return -1; // Instrucción no identificada
            }
        }     
        return 1;
    }
    clear_messages(); // Se limpia el área de mensajes
    mvprintw(12,4,"Error: registro inválido \"%s\"", p1);
    return -1; // El primer parámetro es un registro inválido
}

void print_processor(void)
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

void print_registers(const PCB * pcb)
{
    mvprintw(5,8,"%d      ", pcb->AX);
    mvprintw(6,8,"%d      ", pcb->BX);
    mvprintw(7,8,"%d      ", pcb->CX);
    mvprintw(8,8,"%d      ", pcb->DX);
    mvprintw(5,43,"%d      ", pcb->PC);
}
