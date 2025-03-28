#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <ctype.h>
#include "kbhit.h"
#include "pcb.h"
#include "queue.h"

/*------------------------------FUNCIONES DE INICIALIZACIÓN------------------------------*/
// Inicializa el buffer y el índice
void initialize_buffer(char *buffer, int *index)
{
    // Se limpia el buffer
    for (int i = 0; i < SIZE_BUFFER; i++)
    {
        buffer[i] = 0;
    }
    // Se reinicia el índice
    *index = 0;
}

/*------------------------------FUNCIONES AUXILIARES------------------------------*/
// Convierte a mayúsculas una cadena
void str_upper(char *s)
{
    // Recorre la cadena
    for (int i = 0; s[i] != '\0'; i++)
    {
        // Si el carácter es una letra minúscula
        if (s[i] >= 'a' && s[i] <= 'z')
        {
            s[i] = s[i] - 32; // Convierte a mayúscula
        }
    }
}

// Verifica si una cadena es numérica
int is_numeric(char *str)
{
    // Verificar si la cadena es nula
    if (str == NULL)
        return FALSE;

    // Manejar el caso de un número negativo
    if (*str == '-')
        str++; // avanza al siguiente caracter

    // Si el siguiente carácter no es número, regresa 0
    if (*str == '\0')
        return FALSE;

    // Verificar cada carácter de la cadena
    while (*str)
    {
        if (!isdigit(*str))
            return FALSE; // Si se encuentra un carácter no numérico, regresar 0
        str++;
    }
    return TRUE; // Si todos los caracteres son dígitos, regresar 1
}

// Limpia el prompt de la línea de comandos en la ventana
void clear_prompt(int row)
{
    mvprintw(row, 2, "Artemisos>");
    // Se limpia la línea de comandos de la ventana de 
    for (int i = 0; i < 128; i++)
    {
        mvaddch(row, 12 + i, ' ');
    }
}

// Limpia el área de mensajes
void clear_messages(void)
{
    mvprintw(14, 2, "-                                                                               -");
    mvprintw(15, 2, "-                                                                               -");
    mvprintw(16, 2, "-                                                                               -");
    mvprintw(17, 2, "-                                                                               -");
    mvprintw(18, 2, "-                                                                               -");
    // Se actualiza la pantalla
    refresh();
}

// Imprime las colas de procesos
void queues_area(Queue execution, Queue ready, Queue finished)
{
    int row = 6;
    int col = 86;
    int i = 0;

    // Se imprime la plantilla de las colas
    mvprintw(row, col, "------------------------------EJECUCION----------------------------------          ");
    for (row = 7, i = 0; execution.head != NULL; i++, row++)
    {
        mvprintw(row, col, "PID:[%u] FILE:[%s] AX:[%ld] BX:[%ld] CX:[%ld] DX:[%ld] PC:[%u] IR:[%s]             ",
                 execution.head->pid, execution.head->file_name, execution.head->AX, execution.head->BX,
                 execution.head->CX, execution.head->DX, execution.head->PC, execution.head->IR);
        execution.head = execution.head->next;
    }
    mvprintw(row, col, "--------------------------------LISTOS-----------------------------------          ");
    for (i = 0, row = row + 1; ready.head != NULL; i++, row++)
    {
        mvprintw(row, col, "PID:[%u] FILE:[%s] AX:[%ld] BX:[%ld] CX:[%ld] DX:[%ld] PC:[%u] IR:[%s]             ",
                 ready.head->pid, ready.head->file_name, ready.head->AX, ready.head->BX,
                 ready.head->CX, ready.head->DX, ready.head->PC, ready.head->IR);
        ready.head = ready.head->next;
    }
    mvprintw(row, col, "-----------------------------TERMINADOS----------------------------------          ");
    for (i = 0, row = row + 1; finished.head != NULL; i++, row++)
    {
        mvprintw(row, col, "PID:[%u] FILE:[%s] AX:[%ld] BX:[%ld] CX:[%ld] DX:[%ld] PC:[%u] IR:[%s]             ",
                 finished.head->pid, finished.head->file_name, finished.head->AX, finished.head->BX,
                 finished.head->CX, finished.head->DX, finished.head->PC, finished.head->IR);
        finished.head = finished.head->next;
    }
    // Se actualiza la pantalla
    refresh();
}

// Busca un registro en la cadena
int search_register(char *p)
{
    // Se compara la cadena con los registros
    if (!strcmp(p, "AX"))
        return 'A';
    else if (!strcmp(p, "BX"))
        return 'B';
    else if (!strcmp(p, "CX"))
        return 'C';
    else if (!strcmp(p, "DX"))
        return 'D';
    else
        return -1; // Registro no indentificado
}

// Obtiene el valor de un registro
int value_register(PCB *pcb, char r)
{
    if (r == 'A')
        return pcb->AX;
    else if (r == 'B')
        return pcb->BX;
    else if (r == 'C')
        return pcb->CX;
    else if (r == 'D')
        return pcb->DX;
    return -1;
}

/*------------------------------FUNCIONES PRINCIPALES------------------------------*/
// Manejo de comandos ingresados por el usuario en la línea de comandos
int command_handling(char buffers[NUMBER_BUFFERS][SIZE_BUFFER],
                     int *c, int *index, int *index_history,
                     Queue *execution, Queue *ready, Queue *finished,
                     unsigned *timer, unsigned *init_timer)
{
    // Si se presionó una tecla en la terminal (kbhit)
    if (kbhit()) 
    {
        *c = getch(); // Se obtiene la tecla presionada
        if (*c == ENTER) // Si se presionó ENTER
        {
            // Indicador de que si salió del programa
            int exited = FALSE;
            // Se le coloca carácter nulo para finalizar la cadena prompt
            buffers[0][*index] = '\0';
            // Se evalua el comando en buffer
            exited = evaluate_command(buffers[0], execution, ready, finished);
            // Se crea historial
            strcpy(buffers[5], buffers[4]);
            strcpy(buffers[4], buffers[3]);
            strcpy(buffers[3], buffers[2]);
            strcpy(buffers[2], buffers[1]);
            strcpy(buffers[1], buffers[0]);
            // Se Reinicia índice de historial
            *index_history = 0;
            // Se limpia el buffer y la prompt
            initialize_buffer(buffers[0], index);
            clear_prompt(0);
            // Se imprimen los buffers de historial
            print_history(buffers);
            move(0, 12); // Se coloca el cursor en su lugar
            if (exited)  // Significa que escribió el comando EXIT
            {
                return 1; // Indica que salió del programa
            }
        }
        else if (*c == KEY_BACKSPACE) // Retrocede un carácter
        {
            // Se elimina el último carácter del buffer
            if (*index > 0)
            {
                (*index)--;                                 // Se decrementa el índice
                move(getcury(stdscr), getcurx(stdscr) - 1); // Mueve el cursor hacia atrás
                delch();                                    // Elimina el carácter en la posición actual
            }
        }
        else if (*c == KEY_UP) // Avanza a buffers superiores
        {
            /*  Se comprueba que no se salga del arreglo de buffers y que el
                el comando superior no sea nulo */
            if (*index_history > 0 && buffers[*index_history - 1][0] != '\0')
            {
                // Se compia el comando superior
                strcpy(buffers[0], buffers[*index_history - 1]);
                (*index_history)--;
                // Se limpia la prompt y se imprime el buffer prompt con el comando superior
                clear_prompt(0);
                mvprintw(0, 12, "%s", buffers[0]);
                // Se mueve el índice del buffer prompt para seguir escribiendo
                *index = strlen(buffers[0]);
                move(0, 12 + *index);
            }
        }
        else if (*c == KEY_DOWN) // Comando anterior (avanza a buffers inferiores)
        {
            /*  Se comprueba que no se salga del arreglo de buffers y que el
                el comando anterior no sea nulo */
            if (*index_history < NUMBER_BUFFERS - 1 && buffers[(*index_history) + 1][0] != '\0')
            {
                // Se compia el comando anterior
                strcpy(buffers[0], buffers[*index_history + 1]);
                (*index_history)++;
                // Se limpia la prompt y se imprime el buffer prompt con el comando anterior
                clear_prompt(0);
                mvprintw(0, 12, "%s", buffers[0]);
                // Se mueve el índice del buffer prompt para seguir escribiendo
                *index = strlen(buffers[0]);
                move(0, 12 + *index);
            }
        }
        else if (*c == KEY_RIGHT) // Aumenta la velocidad de escritura
        {
            if (*init_timer < MAX_TIME) // Que no sobrepase el límite del temporizador el inicializador
            {
                (*init_timer) += STEP_TIME; // Se incrementa el temporizador
            }
            // Se imprime el nivel de velocidad
            mvprintw(25, 2, "Nivel velocidad: %d  ", *init_timer / STEP_TIME);
        }
        else if (*c == KEY_LEFT) // Disminuye la velocidad de escritura
        {
            if ((*init_timer) > 0) // Que sea mayor que cero el inicializador de temporizador
            {
                (*init_timer) -= STEP_TIME; // Se decrementa el temporizador
            }
            // Se imprime el nivel de velocidad
            mvprintw(25, 2, "Nivel velocidad: %d  ", *init_timer / STEP_TIME);
        }
        else if (*c == ESC) // Acceso rápido para salir del programa con ESC
        {
            clear_messages(); // Se limpia el área de mensajes
            // Se imprime mensaje de confirmación
            mvprintw(14, 4, "¿Seguro que quieres salir? [S/N]: ");
            // Se obtiene la confirmación
            char confirmation = toupper(getch());
            addch(confirmation);
            if (confirmation == 'S') // Si la confirmación es S
            {
                // Se eliminan todas las colas
                kill_queue(execution);
                kill_queue(ready);
                /* No se llama kill_queue(finished) porque
                   la función cierra los archivos, y en la cola Terminados
                   se supone que los nodos los archivos ya están cerrados */
                while (finished->head)
                {
                    PCB *temp = finished->head;
                    finished->head = finished->head->next; // La cabecera almacena el nodo adelante
                    free(temp);
                }
                return 1; // Indica que salió del programa
            }
            else // Si la confirmación es N
            {
                clear_messages(); // Se limpia el área de mensajes
                move(0, 12);      // Se coloca el cursor en su lugar
            }
        }
        else // Si se presionó cualquier otra tecla
        {
            // Concatena la tecla presionada al buffer para su posterior impresión
            buffers[0][(*index)++] = *c;
            mvaddch(0, 11 + (*index), *c);
        }
    }
    (*timer)++; // Se incrementa el temporizador
    return 0; // Indica que no salió del programa
}

// Evalúa los comandos ingresados por el usuario
int evaluate_command(char *buffer, Queue *execution, Queue *ready, Queue *finished)
{
    /* TOKENS */
    char command[256] = {0};
    char parameter1[256] = {0};
    // char parameter2[128] = {0};
    int pid_to_search = 0; // Variable para almacenar el pid del pcb a matar
    FILE *file = NULL;

    // Se convierte a mayúsculas el buffer de prompt
    str_upper(buffer);

    // Se separa en tokens el comando leído de prompt
    sscanf(buffer, "%s %s", command, parameter1);

    clear_messages(); // Se limpia el área de mensajes

    // Se verifica si el comando es EXIT y no tiene parámetros
    if (!(strcmp(command, "EXIT")) && !parameter1[0])
    {
        // Se imprime mensaje de confirmación
        mvprintw(14, 4, "¿Seguro que quieres salir? [S/N]: ");
        // Se obtiene la confirmación
        char confirmation = toupper(getch());
        addch(confirmation);
        if (confirmation == 'S') // Si la confirmación es S
        {
            // Se eliminan todas las colas
            kill_queue(execution);
            kill_queue(ready);
            /* No se llama kill_queue(finished) porque
               la función cierra los archivos, y en la cola Terminados
               se supone que los nodos los archivos ya están cerrados */
            while (finished->head)
            {
                PCB *temp = finished->head;
                finished->head = finished->head->next; // La cabecera almacena el nodo adelante
                free(temp);
            }
            return 1; // Indica que salió del programa
        }
        else // Si la confirmación es N
        {
            clear_messages(); // Se limpia el área de mensajes
            move(0, 12);      // Se coloca el cursor en su lugar
        }
    }
    else if (!strcmp(command, "LOAD")) // Si el comando es LOAD
    {
        if (parameter1[0]) // Se indicó un archivo a cargar
        {
            // Se abre el archivo en modo lectura
            file = fopen(parameter1, "r");
            if (file)
            {
                // Inserta el nodo en la cola Listos
                enqueue(create_pcb(&ready->pid, parameter1, &file), ready);
                (ready->pid)++;
            }
            else // Si el archivo no existe
            {
                mvprintw(14, 4, "Error: archivo %s no existe.", parameter1);
            }
        }
        else // Si no se especificó un archivo
        {
            mvprintw(14, 4, "Error: faltan parámetros.");
            mvprintw(15, 4, "Sintaxis: LOAD «archivo»");
        }
    }
    else if (!strcmp(command, "KILL")) // Si el comando es KILL
    {
        if (parameter1[0]) // Se verifica si se especificó un pid
        {
            if (is_numeric(parameter1)) // Se verifica si el parámetro de KILL es numérico
            {
                // Se convierte el parámetro a valor numérico
                pid_to_search = atoi(parameter1);
                // Se busca y extrae el pcb solicitado en Ejecución
                PCB *pcb_extracted = search_pcb(pid_to_search, execution);
                if (pcb_extracted) // El pcb se encontró en la cola de Ejecución
                {
                    // Es necesario cerrar el archivo antes de pasar a la cola de Ejecución
                    fclose(pcb_extracted->program);
                    pcb_extracted->program = NULL; // Evita puntero colgante
                    // Se encola el pcb en Terminados
                    enqueue(pcb_extracted, finished);
                    // Se actualiza la impresión de las colas
                    queues_area(*execution, *ready, *finished);
                    // Se limpia el área procesador y se imprime mensaje de terminación
                    processor_template();
                    mvprintw(14, 4, "El pcb con pid [%d] ha sido terminado", pid_to_search);
                }
                // El pcb se encontró en la cola de Listos
                else if ((pcb_extracted = search_pcb(pid_to_search, ready))) 
                {
                    // Es necesario cerrar el archivo antes de pasar a la cola de Ejecución
                    fclose(pcb_extracted->program);
                    pcb_extracted->program = NULL; // Evita puntero colgante
                    // Se encola el pcb en Terminados
                    enqueue(pcb_extracted, finished);
                    // Se actualiza la impresión de las colas
                    queues_area(*execution, *ready, *finished);
                    // Se imprime mensaje de terminación
                    mvprintw(14, 4, "El pcb con pid [%d] ha sido terminado", pid_to_search);
                }
                else // El pcb no se encontró en ninguna cola
                {
                    mvprintw(14, 4, "Error: no se pudo encontrar el pcb con pid [%d].", pid_to_search); // No se encontro el índice
                }
            }
            else // Si el parámetro no es numérico
            {
                mvprintw(14, 4, "Error: parámetro incorrecto \"%s\".", parameter1); // El parámetro no es un número
            }
        }
        else // Si no se especificó un pid
        {
            mvprintw(14, 4, "Error: faltan parámetros.");
            mvprintw(15, 4, "Sintaxis: KILL «índice»");
        }
    }
    else // Si el comando no es EXIT
    {
        mvprintw(14, 4, "Error: comando \"%s\" no encontrado.", command); // Comando no encontrado
    }
    return 0; // Indica que no salió del programa
}

// Lee una línea del archivo y la almacena en un buffer de línea
int read_line(FILE **f, char *line)
{
    // Se lee una línea del archivo
    fgets(line, SIZE_LINE, *f);
    if (!feof(*f)) // Si no se ha llegado al final del archivo
    {
        return 1; // Hay todavía líneas por leer
    }
    else // Si se llegó al final del archivo
    {
        return 0; // No hay líneas por leer
    }
}

// Interpreta y ejecuta la instrucción leída de una línea del archivo de programa de un pcb
int interpret_instruction(char *line, PCB *pcb)
{
    /* TOKENS */
    char instruction[32] = {0}; // Instrucción leída de línea
    char p1[32] = {0};          // Primer parámetro (registro)
    char p2[32] = {0};          // Segundo parámetro (registro o número)
    int number_p2 = 0;          // Valor numérico si p2 es un número

    int is_num_p2 = FALSE; // Bandera que indica si p2 es numérico

    // Se pasa a mayúsculas la instrucción leída
    str_upper(line);

    // Se almacena la instrucción en el registro IR del pcb actual
    strcpy(pcb->IR, line);

    // Se separa en tokens la línea leída y se calcula cuántos hay
    int items = sscanf(line, "%s %s %s", instruction, p1, p2);

    // No se leyó nada
    if (items == -1) 
        return 1; // No se encontró instrucción

    //  Se comprueba si la instrucción es END
    if (!strcmp(instruction, "END"))
    {
        clear_messages(); // Se limpia el área de mensajes
        return 0; // La instrucción leída es END
    }

    // Se comprueba si el segundo parámetro existe y es un número
    is_num_p2 = is_numeric(p2);
    if (is_num_p2)
        number_p2 = atoi(p2); // Se extrae número del segundo parámetro

    // Se comprueba que el primer parámetro sea un registro válido
    int reg1 = search_register(p1);
    if (reg1 == -1)
    {
        clear_messages(); // Se limpia el área de mensajes
        mvprintw(14, 4, "Error: registro inválido \"%s\"", p1);
        return -1; // El primer parámetro es un registro inválido
    }

    // El segundo parámetro es un registro
    if (!is_num_p2 && p2[0] != '\0')
    {
        /*  Se comprueba a qué registro corresponde p2.
            Si sí lo es, se obtiene valor del registro correspondiente. */
        switch (search_register(p2))
        {
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
                mvprintw(14, 4, "Error: registro inválido \"%s\"", p2);
                return -1; // El segundo parámetro es un registro inválido
            break;
        }
    }

    if (items == 3) // Instrucciones de 3 paŕametros (MOV, ADD, SUB, MUL, DIV)
    {
        if (!strcmp(instruction, "MOV"))
        {
            if (reg1 == 'A')
                pcb->AX = number_p2;
            else if (reg1 == 'B')
                pcb->BX = number_p2;
            else if (reg1 == 'C')
                pcb->CX = number_p2;
            else
                pcb->DX = number_p2;
        }
        else if (!strcmp(instruction, "ADD"))
        {
            if (reg1 == 'A')
                pcb->AX += number_p2;
            else if (reg1 == 'B')
                pcb->BX += number_p2;
            else if (reg1 == 'C')
                pcb->CX += number_p2;
            else
                pcb->DX += number_p2;
        }
        else if (!strcmp(instruction, "SUB"))
        {
            if (reg1 == 'A')
                pcb->AX -= number_p2;
            else if (reg1 == 'B')
                pcb->BX -= number_p2;
            else if (reg1 == 'C')
                pcb->CX -= number_p2;
            else
                pcb->DX -= number_p2;
        }
        else if (!strcmp(instruction, "MUL"))
        {
            if (reg1 == 'A')
                pcb->AX *= number_p2;
            else if (reg1 == 'B')
                pcb->BX *= number_p2;
            else if (reg1 == 'C')
                pcb->CX *= number_p2;
            else
                pcb->DX *= number_p2;
        }
        else if (!strcmp(instruction, "DIV"))
        {
            if (number_p2 != 0)
            {
                if (reg1 == 'A')
                    pcb->AX /= number_p2;
                else if (reg1 == 'B')
                    pcb->BX /= number_p2;
                else if (reg1 == 'C')
                    pcb->CX /= number_p2;
                else
                    pcb->DX /= number_p2;
            }
            else
            {
                clear_messages(); // Se limpia el área de mensajes
                mvprintw(14, 4, "Error: División por 0 inválida");
                return -1; // Instrucción no encontrada
            }
        }
        else // Si la instrucción no es MOV, ADD, SUB, MUL, DIV
        {
            clear_messages(); // Se limpia el área de mensajes
            // Si la instrucción es INC o DEC, se indica que hay exceso de parámetros
            if (!strcmp(instruction, "INC") || !strcmp(instruction, "DEC")) 
                mvprintw(14, 4, "Error: instrucción \"%s\" execeso de parámetros", instruction);
            else // Si no, se indica que la instrucción no fue encontrada
                mvprintw(14, 4, "Error: instrucción inválida \"%s\"", instruction);
            return -1; // Sintaxis de instrucción incorrecta
        }
    }
    else if (items == 2) // Instrucciones de 2 paŕametros (INC, DEC)
    {
        if (!strcmp(instruction, "INC"))
        {
            if (reg1 == 'A')
                pcb->AX += 1;
            else if (reg1 == 'B')
                pcb->BX += 1;
            else if (reg1 == 'C')
                pcb->CX += 1;
            else
                pcb->DX += 1;
        }
        else if (!strcmp(instruction, "DEC"))
        {
            if (reg1 == 'A')
                pcb->AX -= 1;
            else if (reg1 == 'B')
                pcb->BX -= 1;
            else if (reg1 == 'C')
                pcb->CX -= 1;
            else
                pcb->DX -= 1;
        }
        else // Si la instrucción no es INC o DEC
        {
            clear_messages(); // Se limpia el área de mensajes
            // Si la instrucción es MOV, ADD, SUB, MUL, DIV, se indica que faltan parámetros
            if (!strcmp(instruction, "MOV") || 
                !strcmp(instruction, "ADD") || 
                !strcmp(instruction, "SUB") ||
                !strcmp(instruction, "MUL") || 
                !strcmp(instruction, "DIV")) {
                mvprintw(14, 4, "Error: instrucción \"%s\" faltan parámetros", instruction);
            }
            else // Si no, se indica que la instrucción no fue encontrada
                mvprintw(14, 4, "Error: instrucción inválida \"%s\"", instruction);
            return -1; // Sintaxis de instrucción incorrecta
        }
    }
    return 1; // Instrucción ejecutada correctamente
}

// Plantilla de la ventana de procesador
void processor_template(void)
{
    mvprintw(6, 2, "----------------------------------PROCESADOR-------------------------------------");
    mvprintw(7, 2, "- AX:                                PC:                                        -");
    mvprintw(8, 2, "- BX:                                IR:                                        -");
    mvprintw(9, 2, "- CX:                                PID:                                       -");
    mvprintw(10, 2, "- DX:                                NAME:                                      -");
    mvprintw(11, 2, "-                                    SIG:                                       -");
    mvprintw(12, 2, "---------------------------------------------------------------------------------");
    refresh();
}

// Plantilla de la ventana de mensajes
void messages_template(void)
{
    mvprintw(13, 2, "-----------------------------------MENSAJES--------------------------------------");
    mvprintw(14, 2, "-                                                                               -");
    mvprintw(15, 2, "-                                                                               -");
    mvprintw(16, 2, "-                                                                               -");
    mvprintw(17, 2, "-                                                                               -");
    mvprintw(18, 2, "-                                                                               -");
    mvprintw(19, 2, "---------------------------------------------------------------------------------");
    refresh();
}

// Imprime el historial de comandos en la ventana
void print_history(char buffers[NUMBER_BUFFERS][SIZE_BUFFER])
{
    // Se imprimen los buffers de historial
    for (int i = 1; i < NUMBER_BUFFERS; i++)
    {
        if (buffers[i][0] != 0) // Si el buffer no está vacío
        {
            clear_prompt(i);
            mvprintw(i, 2, "Artemisos>%s", buffers[i]);
        }
    }
}

// Imprime los registros del pcb en ejecución en la ventana de procesador
void print_registers(PCB pcb)
{
    mvprintw(7, 7, "[%ld]      ", pcb.AX);
    mvprintw(7, 42, "[%u]      ", pcb.PC);
    mvprintw(8, 7, "[%ld]       ", pcb.BX);
    mvprintw(8, 42, "[%s]            ", pcb.IR);
    mvprintw(9, 7, "[%ld]      ", pcb.CX);
    mvprintw(9, 43, "[%u]      ", pcb.pid);
    mvprintw(10, 7, "[%ld]      ", pcb.DX);
    mvprintw(10, 44, "[%s]      ", pcb.file_name);
    mvprintw(11, 43, "[%p]      ", pcb.next);
}
