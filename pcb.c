#include <stdio.h>                                  
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <ctype.h>
// Bibliotecas propias
#include "kbhit.h"
#include "pcb.h"
#include "queue.h"

/*------------------------------FUNCIONES DE INICIALIZACIÓN------------------------------*/
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

/*------------------------------FUNCIONES AUXILIARES------------------------------*/
void str_upper(char *s)
{
    for (int i = 0; s[i] != '\0'; i++) {
        if (s[i] >= 'a' && s[i] <= 'z') {
            s[i] = s[i] - 32;
        }
    }
}

int is_numeric(char *str) 
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
    mvprintw(14,2, "-                                                                               -");
    mvprintw(15,2, "-                                                                               -");
    mvprintw(16,2, "-                                                                               -");
    mvprintw(17,2, "-                                                                               -");
    mvprintw(18,2, "-                                                                               -");
}

void clear_queue(void)
{ 
    extern int pcb_count;
    for(int i = 21; i < 22 + pcb_count; i++){
        mvprintw(i,0,"                                                                                  ");
    }
    refresh();
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

int value_register(PCB *pcb, char r)
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

/*------------------------------FUNCIONES PRINCIPALES------------------------------*/
int command_handling(char buffers[NUMBER_BUFFERS][SIZE_BUFFER], 
    int *read_flag, int *c, int *index, FILE **f, int *index_history, 
    int *pid, int *processor_busy, PCB **detached_pcb)
{
    if (kbhit()) {
        *c = getch();
        if (*c == ENTER) {                                                                
            buffers[0][*index] = '\0';
            // Se evalua el comando en buffer
            evaluate_command(buffers[0], index, read_flag, f, pid, processor_busy, detached_pcb);
            // Se crea historial
            strcpy(buffers[5],buffers[4]);
            strcpy(buffers[4],buffers[3]);
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
        } 
        else if (*c == KEY_BACKSPACE) {
            // Se elimina el último carácter del buffer
            if (*index > 0) {
                (*index)--;
                move(getcury(stdscr), getcurx(stdscr) - 1); // Mueve el cursor hacia atrás
                delch();  // Elimina el carácter en la posición actual
            }
        } 
        else if (*c == KEY_UP) {  // Avanza a buffers superiores
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
        } 
        else if (*c == KEY_DOWN) { // Comando anterior (avanza a buffers inferiores)
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
        } 
        else if (*c == ESC) { // Acceso rápido para salir del programa con ESC
            clear_messages(); // Se limpia el área de mensajes
            mvprintw(14,4,"¿Seguro que quieres salir? [S/N]: ");
            char confirmation = toupper(getch());
            addch(confirmation);
            if (confirmation == 'S'){
                kill_queue();    //Se vacia la cola.
                if(*detached_pcb){ // Se elimina el pcb si hay
                    remove_pcb(detached_pcb); // Si hay pcb en procesador, se elimina
                }
                endwin();
                printf("\n");
                exit(0); 
            } else {
                clear_messages(); // Se limpia el área de mensajes
                move(0,12); // Se coloca el cursor en su lugar
            }
        } 
        else {
            // Concatena la tecla presionada al buffer para su posterior impresión
            buffers[0][(*index)++] = *c;
            mvaddch(0,11+(*index),*c);
        }
    } 
    return TRUE;
}

int evaluate_command(char *buffer, int *index, int *read_flag, 
    FILE **f, int *pid, int *processor_busy, PCB **detached_pcb)
{
    /* TOKENS */
    char command[256] = {0}; 
    char parameter1[256] = {0};
    //char parameter2[128] = {0};
    int pid_to_search = 0;      // Variable para almacenar el pid del pcb a matar
    extern PCB * queue;         // Se consulta la cola para verificar las extracción 
    
    // Se convierte a mayúsculas
    str_upper(buffer);

    // Se separa en tokens el comando leído de prompt
    sscanf(buffer, "%s %s", command, parameter1);

    if (!(strcmp(command, "EXIT")) && !parameter1[0]) {
        kill_queue();   //Se vacía la cola
        if(*detached_pcb){ // Si hay pcb en procesador, se elimina 
            remove_pcb(detached_pcb);
        }
        endwin(); // Cierra la ventana
        printf("\n");
        exit(0); 
    }
    else {
        clear_messages(); // Se limpia el área de mensajes
        /*if (!strcmp(command, "LOAD")) {
            if (parameter1[0]) {   // Se indicó un archivo a cargar
                *f = fopen(parameter1, "r");
                if(*f) {
                    mvprintw(14,4,"Nombre de archivo: %s", parameter1);
                    mvprintw(15,4,"Leyendo...");
                    *read_flag = TRUE; // Enciende la bandera de lectura
                } else {
                    mvprintw(14,4,"Error: archivo no existe.");
                }
            } else {
                mvprintw(14,4,"Error: faltan parámetros.");
                mvprintw(15,4,"Sintaxis: LOAD «archivo»");
            }
        }*/
        if(!strcmp(command, "INSERT")) {
            if (parameter1[0]) { // Se especificó un archivo 
                *f = fopen(parameter1, "r");
                if (*f) { // El archivo existe
                    enqueue(create_pcb(pid, parameter1, f));
                    print_queue();
                    (*pid)++;
                } 
                else {
                    mvprintw(14,4,"Error: archivo no existe.");
                }
            }
            else {
                mvprintw(14,4,"Error: faltan parámetros.");
                mvprintw(15,4,"Sintaxis: INSERT «archivo»");
            } 
        }

       
        else if (!strcmp(command, "PULL")) {
            if (!(*processor_busy)) {   // Se comprueba que el procesador no esté ocupado
                if (queue) { // Extrae de la cola si la cola no está vacía
                    *detached_pcb = dequeue(); // Nodo desligado para colocarlo en el procesador
                    clear_queue();
                    print_queue();
                    print_registers(*detached_pcb); // Se muestran datos del nodo pcb en el área del procesador
                    *processor_busy = TRUE;
                    mvprintw(14,4,"El pcb con pid [%d] ha sido puesto en procesador.", (*detached_pcb)->pid);
                }
                else {
                    mvprintw(14,4,"Error: la cola esta vacia.");
                }
            }
            else {
                mvprintw(14,4,"Error: procesador ocupado.");
            }
        }
        else if (!strcmp(command, "PUSH")) {
            if (*processor_busy) { // Verifica que el procesador está ocupado para encolar
                enqueue(*detached_pcb); // Se encola el nodo que estaba en el procesador
                *processor_busy = FALSE; // El procesador vuelve a estar disponible
                clear_queue();
                print_queue();
                processor_template(); // Se limpia todo el área del procesador
                mvprintw(14,4,"El pcb con pid [%d] ha sido puesto en cola.", (*detached_pcb)->pid);
                *detached_pcb = NULL;   // Se libera el puntero auxiliar
            }
            else {
                mvprintw(14,4,"Error: no hay ningún proceso activo.");
            }
        }
        else if (!strcmp(command, "KILL")) {
            if (parameter1[0]) { // Se verifica si se especificó un pid
                if (is_numeric(parameter1)) { // Se verifica si el parámetro de KILL es numérico
                    // Se convierte el parámetro a valor numérico
                    pid_to_search = atoi(parameter1);
                    // Se busca y extrae el pcb solicitado
                    PCB *pcb_extracted = search_pcb(pid_to_search); 
                    if (pcb_extracted) { // Se encontró en la cola
                        remove_pcb(&pcb_extracted);
                        clear_queue();
                        print_queue();
                        mvprintw(14,4,"El pcb con pid [%d] ha sido eliminado", pid_to_search);
                    }
                    else if (*detached_pcb && (*detached_pcb) -> pid == pid_to_search) { // No estaba en cola, se busca en procesador
                        remove_pcb(detached_pcb);
                        processor_template();
                        *processor_busy = FALSE;
                        mvprintw(14,4,"El pcb con pid [%d] ha sido eliminado", pid_to_search);
                    }
                    else {
                        mvprintw(14,4,"Error: no se pudo encontrar el pcb con pid [%d].", pid_to_search); // No se encontro el índice
                    }
                } 
                else {
                    mvprintw(14,4,"Error: parámetro incorrecto \"%s\".", parameter1);  //El parámetro no es un número
                }
            }
            else {
                mvprintw(14,4,"Error: faltan parámetros.");
                mvprintw(15,4,"Sintaxis: KILL «índice»");
            } 
        }
        else {
            mvprintw(14,4,"Error: \"%s\" no es comando válido.", buffer);
        }
    }
    return TRUE;
}

int read_line(FILE **f, char *line)
{
    line = fgets(line, SIZE_LINE, *f);
    if(!feof(*f)) {
        str_upper(line); // Convierte la intrucción leída en mayúsculas
        return 1;   // Hay todavía líneas por leer
    } else {
        return 0; // No hay líneas por leer
    }
}

int interpret_instruction(char *line, PCB *pcb)
{   
    /* TOKENS */
    char instruction[4] = {0};  // Instrucción leída de línea
    char p1[4] = {0};           // Primer parámetro (registro)
    char p2[32] = {0};          // Segundo parámetro (registro o número)
    int number_p2 = 0;          // Valor numérico si p2 es un número

    int is_num_p2 = FALSE; // Bandera que indica si p2 es numérico

    // Se pasa a mayúsculas la instrucción leída
    str_upper(line);

    // Se separa en tokens la línea leída y se calcula cuántos hay
    int items = sscanf(line, "%s %s %s", instruction, p1, p2);
    
    // Se comprueba si la instrucción es END
    if (!strcmp(instruction, "END")) {
        clear_messages();
        return 0; // La instrucción leída es END
    }

    // Se comprueba si el segundo parámetro existe y es un número
    is_num_p2 = is_numeric(p2);
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
                    mvprintw(14,4,"Error: registro inválido \"%s\"", p2);
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
                if(number_p2 != 0){   
                    if (reg1 == 'A') {          
                        pcb->AX /= number_p2;
                    } else if (reg1 == 'B') {      
                        pcb->BX /= number_p2;
                    } else if (reg1 == 'C') {      
                        pcb->CX /= number_p2;
                    } else {                           
                        pcb->DX /= number_p2;    
                    }
                }
                else
                {
                    clear_messages(); // Se limpia el área de mensajes
                    mvprintw(14,4,"Error: División por 0 inválida");
                    return -1; // Instrucción no encontrada
                }
            } else {
                clear_messages(); // Se limpia el área de mensajes
                mvprintw(14,4,"Error: instrucción inválida \"%s\"", instruction);
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
                mvprintw(14,4,"Error: instrucción inválida \"%s\"", instruction);
                return -1; // Instrucción no identificada
            }
        }     
        return 1;
    }
    clear_messages(); // Se limpia el área de mensajes
    mvprintw(14,4,"Error: registro inválido \"%s\"", p1);
    return -1; // El primer parámetro es un registro inválido
}

void processor_template(void)
{
    mvprintw(6,2, "----------------------------------PROCESADOR-------------------------------------");
    mvprintw(7,2, "- AX:                                PC:                                        -"); 
    mvprintw(8,2, "- BX:                                IR:                                        -");
    mvprintw(9,2, "- CX:                                PID:                                       -");
    mvprintw(10,2,"- DX:                                NAME:                                      -");
    mvprintw(11,2,"-                                    SIG:                                       -");
    mvprintw(12,2, "---------------------------------------------------------------------------------");
    refresh();
}

void messages_template(void)
{
    mvprintw(13,2, "-----------------------------------MENSAJES--------------------------------------");
    mvprintw(14,2, "-                                                                               -");
    mvprintw(15,2, "-                                                                               -");
    mvprintw(16,2, "-                                                                               -");
    mvprintw(17,2, "-                                                                               -");
    mvprintw(18,2, "-                                                                               -");
    mvprintw(19,2, "---------------------------------------------------------------------------------");
    refresh();
}

void queue_template(void)
{
    mvprintw(20,2, "-------------------------------------COLA----------------------------------------");
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

void print_registers(PCB * pcb)
{
    mvprintw(7,7,"[%d]      ", pcb->AX);
    mvprintw(7,42,"[%d]      ", pcb->PC);
    mvprintw(8,7,"[%d]      ", pcb->BX);
    mvprintw(8,42,"[%s]      ", pcb->IR);
    mvprintw(9,7,"[%d]      ", pcb->CX);
    mvprintw(9,43,"[%d]      ", pcb->pid);
    mvprintw(10,7,"[%d]      ", pcb->DX);
    mvprintw(10,44,"[%s]      ", pcb->file_name);
    mvprintw(11,43,"[%p]      ", pcb->next);
}