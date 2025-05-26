#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <ctype.h>
#include <math.h>
// Bibliotecas propias
#include "kbhit.h"
#include "pcb.h"
#include "queue.h"
#include "gui.h"

// Variables globales (menos pbase que se ocupa al crear el pcb)
// Se ocupan para calcular la nueva prioridad y el uso del CPU del proceso y usuario
int IncCPU = 60 / MAXQUANTUM;
int NumUs = 0;
double W = 0.0; // Después el peso es 1/NumUs
const int PBase = 60;

/*------------------------------FUNCIONES DE INICIALIZACIÓN------------------------------*/
// Inicializa el buffer y el índice
void initialize_buffer(char *buffer, int *index)
{
  // Se limpia el buffer
  for (int i = 0; i < BUFFER_SIZE; i++)
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

// Pregunta si quiere salir del simulador
int end_simulation(WINDOW *inner_msg)
{
  // Se imprime mensaje de confirmación
  mvwprintw(inner_msg, 0, 0, "¿Seguro que quieres salir? [S/N]:");
  // Se obtiene la confirmación
  char confirmation = toupper(wgetch(inner_msg));
  if (confirmation == 'S')
  {
    return TRUE;
  }
  return FALSE;
}

// Imprime el historial de comandos en la ventana
void print_history(char buffers[NUMBER_BUFFERS][BUFFER_SIZE], WINDOW *inner_prompt)
{
  // Se imprimen los buffers de historial
  for (int i = 1; i < NUMBER_BUFFERS; i++)
  {
    if (buffers[i][0] != 0) // Si el buffer no está vacío
    {
      clear_prompt(inner_prompt, i);
      print_prompt(inner_prompt, i);
      mvwprintw(inner_prompt, i, PROMPT_START, "%s", buffers[i]);
    }
  }
}

// Cuenta el número de lineas igonarando líneas vacías
int count_lines(FILE *file)
{
  int lines = 0;
  int c, last = '\n';
  while ((c = fgetc(file)) != EOF)
  {
    // Dos saltos de líneas consecutivos indican una línea vacía
    if (c == '\n' && last != '\n')      
      lines++;
    last = c;
  }
  rewind(file); // Regresa al inicio del archivo
  return lines;
}

/*------------------------------FUNCIONES PRINCIPALES------------------------------*/
// Manejo de comandos ingresados por el usuario en la línea de comandos
int command_handling(GUI *gui, char buffers[NUMBER_BUFFERS][BUFFER_SIZE],
                     int *c, int *index, int *index_history,
                     Queue *execution, Queue *ready, Queue *finished, Queue *new,
                     unsigned *timer, unsigned *init_timer, int *speed_level, TMS * tms, FILE **swap,
                     int *swap_disp, int *tms_disp, int * tmp_disp)
{
  // Si se presionó una tecla en la terminal (kbhit)
  if (kbhit())
  {
    // Se obtiene la tecla presionada en la subventana de prompt
    *c = wgetch(gui->inner_prompt);
    if (*c == ENTER) // Si se presionó ENTER
    {
      // Indicador de que si salió del programa
      int exited = FALSE;
      // Se le coloca carácter nulo para finalizar la cadena prompt
      buffers[0][*index] = '\0';
      // Se evalua el comando en buffer
      exited = evaluate_command(gui, buffers[0], execution, ready, finished, new, tms, swap, swap_disp, tmp_disp);
      // Se crea historial
      for (int i = NUMBER_BUFFERS - 1; i >= 0; i--)
      {
        strcpy(buffers[i], buffers[i - 1]);
      }
      // Se Reinicia índice de historial
      *index_history = 0;
      // Se limpia el buffer y la prompt
      initialize_buffer(buffers[0], index);
      clear_prompt(gui->inner_prompt, 0);
      // Se imprimen los buffers de historial
      print_history(buffers, gui->inner_prompt);
      wmove(gui->inner_prompt, 0, PROMPT_START); // Se coloca el cursor en su lugar
      if (exited)                                // Significa que escribió el comando EXIT
      {
        return 1; // Indica que salió del programa
      }
    }
    else if (*c == KEY_BACKSPACE) // Retrocede un carácter
    {
      // Se elimina el último carácter del buffer
      if (*index > 0)
      {
        // Se decrementa el índice
        (*index)--;
        // Se sustituye el cáracter a eliminar por espacio en blanco
        mvwaddch(gui->inner_prompt, getcury(gui->inner_prompt), getcurx(gui->inner_prompt) - 1, ' ');
        // Mueve el cursor hacia atrás
        wmove(gui->inner_prompt, getcury(gui->inner_prompt), getcurx(gui->inner_prompt) - 1);
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
        clear_prompt(gui->inner_prompt, 0);
        mvwprintw(gui->inner_prompt, 0, 12, "%s", buffers[0]);
        // Se mueve el índice del buffer prompt para seguir escribiendo
        *index = strlen(buffers[0]);
        wmove(gui->inner_prompt, 0, PROMPT_START + *index);
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
        clear_prompt(gui->inner_prompt, 0);
        mvwprintw(gui->inner_prompt, 0, 12, "%s", buffers[0]);
        // Se mueve el índice del buffer prompt para seguir escribiendo
        *index = strlen(buffers[0]);
        wmove(gui->inner_prompt, 0, PROMPT_START + *index);
      }
    }
    else if (*c == KEY_LEFT) // Disminuye la velocidad de escritura
    {
      if (*speed_level > 1) // Que sea mayor que el nivel 1 de velocidad
      {
        (*init_timer) -= MAX_TIME / ((int)pow(2, *speed_level - 1)); // Se decrementa el temporizador
        (*speed_level)--;                                            // Se decrementa el nivel de velocidad
      }
    }
    else if (*c == KEY_RIGHT) // Aumenta la velocidad de escritura
    {
      if (*speed_level < MAX_LEVEL) // Que no sobrepase el límite máximo de niveles
      {
        (*init_timer) += MAX_TIME / ((int)pow(2, *speed_level)); // Se incrementa el temporizador
        (*speed_level)++;                                        // Se incrementa el nivel de velocidad
      }
    }
    else if (*c == KEY_F(5)) // Retrocede TMS (PgUp)
    {
      if((*tms_disp) > 0)
      {
        (*tms_disp)--;
      }
      print_tms(gui -> inner_tms, *tms, *tms_disp);
    }
    else if (*c == KEY_F(6)) // Avanza TMS (PgDn)
    {
      if((*tms_disp) < TOTAL_DISP_TMS)
      {
        (*tms_disp)++;
      }
      print_tms(gui -> inner_tms, *tms, *tms_disp);
    }
    else if (*c == KEY_IC) // Retrocede TMP (ins)
    {}
    else if (*c == KEY_DC) // Avanza TMP (supr)
    {}
    else if (*c == KEY_F(7)) // Retrocede SWAP (F7)
    {
      if((*swap_disp) > 0){
        (*swap_disp)--;
      }
      print_swap(gui->inner_swap, *swap, *swap_disp);
    }
    else if (*c == KEY_F(8)) // Avanza SWAP (F8)
    {
      if((*swap_disp) < TOTAL_DISP_SWAP){
        (*swap_disp)++;
      }
      print_swap(gui->inner_swap, *swap, *swap_disp);
      
    }
    else if (*c == ESC) // Acceso rápido para salir del programa con ESC
    {
      /* Se limpia área de mensajes con wclear para que redibuje todo
         y no queden residuos de carácteres */
      wclear(gui->inner_msg);
      // Se confirma si quiere salir del programa
      if (end_simulation(gui->inner_msg))
      {
        // Se liberan todas las colas
        free_queues(execution, ready, finished);
        return 1; // Indica que salió del programa
      }
      // Se limpia área de mensajes
      werase(gui->inner_msg);
      // Se coloca el cursor en su lugar
      wmove(gui->inner_prompt, 0, PROMPT_START);
      // Se refresca la subventana de mensajes
      wrefresh(gui->inner_msg);
    }
    else // Si se presionó cualquier otra tecla
    {
      if (*index < BUFFER_SIZE - 1)
      {
        // Concatena la tecla presionada al buffer para su posterior impresión
        buffers[0][(*index)++] = *c;
        mvwaddch(gui->inner_prompt, 0, (PROMPT_START - 1) + (*index), *c);
      }
    }
  }
  (*timer)++; // Se incrementa el temporizador
  return 0;   // Indica que no salió del programa
}

// Evalúa los comandos ingresados por el usuario
int evaluate_command(GUI *gui, char *buffer, Queue *execution, Queue *ready, Queue *finished, Queue *new,
                     TMS *tms, FILE **swap, int *swap_disp, int * tmp_disp)
{
  /* TOKENS */
  char command[256] = {0};    // Almacena el comando ingresado
  char parameter1[256] = {0}; // Almacena el nombre del archivo a cargar o el nombre del proceso a matar
  char parameter2[256] = {0}; // Almacena el id de usuario, se analiza con is_numeric
  int value_par2 = 0;         // Almacena el uid como valor numerico
  int pid_to_search = 0;      // Variable para almacenar el pid del pcb a matar
  FILE *file = NULL;          // Almacena el puntero al archivo a cargar
  int tmp_size = 0;           // Almacena el número de marcos del archivo a cargar
  int lines = 0;              // Almacena el número de líneas del archivo a cargar
  int KCPUxU = 0;             // Almacena el valor de KCPUxU del usuario
  int pid_brother_process = 0;// Almacena el pid del proceso que ya tiene el mismo archivo cargado en SWAP

  // Se separa en tokens el comando leído de prompt
  sscanf(buffer, "%s %s %s", command, parameter1, parameter2); // Que pasa si como segundo parámetro es una a?

  // Se convierte el comando a mayúsculas
  str_upper(command);

  // Se limpia el área de mensajes
  werase(gui->inner_msg);

  // Se verifica si el comando es EXIT y no tiene parámetros
  if (!(strcmp(command, "EXIT")) && !parameter1[0])
  {
    /* Se limpia área de mensajes con wclear para que redibuje todo
       y no queden residuos de carácteres */
    wclear(gui->inner_msg);
    // Se confirma si quiere salir del programa
    if (end_simulation(gui->inner_msg))
    {
      // Se liberan todas las colas
      free_queues(execution, ready, finished);
      return 1; // Indica que salió del programa
    }
    // Se limpia área de mensajes
    werase(gui->inner_msg);
    // Se coloca el cursor en su lugar
    wmove(gui->inner_prompt, 0, PROMPT_START);
    // Se refresca la subventana de mensajes
    wrefresh(gui->inner_msg);
  }
  else if (!strcmp(command, "LOAD")) // Si el comando es LOAD
  {
    if (parameter1[0]) // Se indicó un archivo a cargar
    {
      // Se verifica si se especificó un id de usuario valido (entero)
      if (is_numeric(parameter2))
      {
        value_par2 = atoi(parameter2);
        // Se abre el archivo en modo lectura
        file = fopen(parameter1, "r");
        if (file)
        {
          // Total de lineas en el archivo ignorando líneas vacías
          lines = count_lines(file);
          // Se calculan el número de marcos/páginas que ocupará el proceso en el SWAP
          tmp_size = (int)ceil((double)lines / PAGE_SIZE);

          // Se crea el proceso inicializando sus atributos
          PCB *new_pcb = create_pcb(ready->pid, parameter1, &file, value_par2, tmp_size);
          if (!new_pcb)
          {
            mvwprintw(gui->inner_msg, 1, 0, "Error: no pudo crear el proceso %d [%s].", ready->pid, parameter1);
            wrefresh(gui->inner_msg);
            return 0;
          }

          // Incremento de pid para el siguiente proceso a crear
          (ready->pid)++;

          /*
            Se verifica si el usuario del nuevo proceso ya existe o no (tiene algún proceso en Ejecución o Listos).
            Si no existe el usuario, se incrementa el NumUs.
            Si existe, el KCPUxU se empareja con el resto de procesos del usuario.
          */
          if (!search_uid(new_pcb->UID, *execution) && !search_uid(new_pcb->UID, *ready))
          {
            NumUs++;
          }
          /* El usuario ya tiene procesos, se actualiza KCPUxU en común al usuario,
             Se verifica primero si hay algún proceso en Listos para actualizarlo*/
          else if ((KCPUxU = get_KCPUxU(new_pcb->UID, *ready)) != -1)
          {
            new_pcb->KCPUxU = KCPUxU;
          }
          else // Si no hay un proceso del usuario en Listos, seguramente está en Ejecución
          {
            new_pcb->KCPUxU = execution->head->KCPUxU;
          }

          // Se actualiza el peso W una vez creado el nuevo proceso y actualizado el NumUs
          if (NumUs) W = 1.0 / NumUs;

          /* Buscar si el programa, ya se encuentra previamente cargado por algún otro proceso
            del mismo usuario. Ya sea en Listos o en Ejecución */
          if ( (pid_brother_process = search_process(new_pcb->UID, new_pcb->file_name, *execution)) )
          {
            // De haberse encontrado en ejecución, asignar la misma TMP al nuevo proceso
            //mvwprintw(gui->inner_msg, 1, 0, "Proceso hermano: %d", pid_brother_process);
            set_same_tmp(*execution, new_pcb, pid_brother_process);

          }
          else if( (pid_brother_process= search_process(new_pcb->UID, new_pcb->file_name, *ready)))
          {
            // De haberse encontrado en ready, asignar la misma TMP al nuevo proceso
            //mvwprintw(gui->inner_msg, 1, 0, "Proceso hermano: %d", pid_brother_process);
            set_same_tmp(*ready, new_pcb, pid_brother_process);
          }
          
          // Flujo de acción para procesos sin hermanos
          //Verificar que el proceso sea de menor tamaño que la SWAP
          else if (lines <= SWAP_SIZE) 
          {
            // Si la cantidad de marcos libres en SWAP alcanzan para cargar el proceso
            if (new_pcb->TmpSize <= tms->available_pages) {
              // Inserta el nodo en la cola Listos despues de verificar las condiciones de SWAP
              enqueue(new_pcb, ready);
              // Se imprime mensaje de proceso creado 
              mvwprintw(gui->inner_msg, 0, 0, "Proceso: %d [%s] UID: [%d]", new_pcb->pid, new_pcb->file_name, new_pcb->UID);
              mvwprintw(gui->inner_msg, 1, 0, "Creado correctamente.");

              // Se reserva espacio para la tmp del proceso
              new_pcb->TMP = (int *) malloc(sizeof(int) * tmp_size);
              // Se imprime la tmp del proceso creado
              print_tmp(gui->inner_tmp, *new_pcb, *tmp_disp);

              // Cargar instrucciones en swap y registrar en TMS los marcos ocupados por el proceso
              load_to_swap(new_pcb, tms, swap, lines, gui);
              
              // Se cierra el archivo una vez que se ha cargado en la swap
              fclose(file);
              // Se evita puntero colgante
              new_pcb->program = NULL;
            }
            else { // De lo contrario, inserta el proceso al final de la lista Nuevos (puede entrar después)
              // Insertar el nuevo proceso en la cola Nuevos
              enqueue(new_pcb, new);
              // Se imprime mensaje de proceso creado e insertado en Nuevos
              mvwprintw(gui->inner_msg, 0, 0, "Proceso: %d [%s] UID: [%d] -> Nuevos", new_pcb->pid, new_pcb->file_name, new_pcb->UID);
              mvwprintw(gui->inner_msg, 1, 0, "Sin swap disponible");
            }
          }
          else // La swap no es suficiente para cargar el proceso
          {
            // Enviar el proceso directamente a Terminados, indicando el motivo
            enqueue(create_pcb(ready->pid, parameter1, &file, value_par2, tmp_size), finished);
            mvwprintw(gui->inner_msg, 0, 0, "Proceso: %d [%s] UID: [%d] -> Terminados", new_pcb->pid, new_pcb->file_name, new_pcb->UID);
            mvwprintw(gui->inner_msg, 1, 0, "El programa excede el tamaño del SWAP. Actualice el sistema.");
            // Se cierra el archivo dado que no se cargó en la swap
            fclose(file);
            // Se evita puntero colgante
            new_pcb->program = NULL;
          }
          print_swap(gui->inner_swap, *swap, *swap_disp);
        }
        else // Si el archivo no existe
        {
          mvwprintw(gui->inner_msg, 0, 0, "Error: archivo %s no existe.", parameter1);
        }
      }
      // No metió un id de usuario
      else if (parameter2[0] == '\0')
      {
        mvwprintw(gui->inner_msg, 0, 0, "Error: debes ingresar un id de usuario.");
      }
      // Metió otra cosa que no es un entero
      else
      {
        mvwprintw(gui->inner_msg, 0, 0, "Error: id de usuario debe ser un entero.");
      }
    }
    else // Si no se especificó un archivo
    {
      mvwprintw(gui->inner_msg, 0, 0, "Error: faltan parámetros.");
      mvwprintw(gui->inner_msg, 1, 0, "Sintaxis: LOAD «archivo» «uid».");
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
        PCB *pcb_extracted = extract_by_pid(pid_to_search, execution); // Hay 2 opciones, que el pcb a matar este en ejecución o en ready, comenzamos con ejecución
        if (pcb_extracted)                                             // El pcb se encontró en la cola de Ejecución
        {
          // Es necesario cerrar el archivo antes de pasar a la cola de Ejecución
          fclose(pcb_extracted->program);
          // Evita puntero colgante
          pcb_extracted->program = NULL;
          // Ya no es necesario buscar el usuario en la cola de Ejecución
          if (!search_uid(pcb_extracted->UID, *ready))
          {
            NumUs--;
          }
          // Se actualiza el valor de W
          if (NumUs)
          {
            W = 1.0 / NumUs;
          }
          else
          {
            W = 0.0; // No hay ningún usuario
          }
          // Se encola el pcb en Terminados
          enqueue(pcb_extracted, finished);
          // Se imprime procesador vacío y se muestra mensaje
          empty_processor(gui->inner_cpu);
          mvwprintw(gui->inner_msg, 0, 0, "El pcb con pid [%d] ha sido terminado.", pid_to_search);

          /* Si solo había un proceso en Ejecución y nada en Listos,
             se debe de refrescar la impresión de las colas para que
             se vea que el proceso está en la cola Finalizados */
          if (!ready->head)
          {
            print_queues(gui->inner_queues, *execution, *ready, *finished, *new);
          }
        }
        else if ((pcb_extracted = extract_by_pid(pid_to_search, ready))) // El pcb se encontró en la cola de Listos
        {
          // Es necesario cerrar el archivo antes de pasar a la cola de Ejecución
          fclose(pcb_extracted->program);
          // Evita puntero colgante
          pcb_extracted->program = NULL;
          // Se verifica si todavía hay procesos del mismo usuario
          if (!search_uid(pcb_extracted->UID, *ready) && !search_uid(pcb_extracted->UID, *execution))
          {
            NumUs--;
          }
          // Se actualiza el valor de W
          if (NumUs)
          {
            W = 1.0 / NumUs;
          }
          else
          {
            W = 0.0; // No hay ningún usuario
          }
          // Se encola el pcb en Terminados
          enqueue(pcb_extracted, finished);
          // Se imprime mensaje de terminación
          mvwprintw(gui->inner_msg, 0, 0, "El pcb con pid [%d] ha sido terminado.", pid_to_search);
        }
        else // El pcb no se encontró en ninguna cola
        {
          mvwprintw(gui->inner_msg, 0, 0, "Error: no se pudo encontrar el pcb con pid [%d].", pid_to_search); // No se encontro el índice
        }
      }
      else // Si el parámetro no es numérico
      {
        mvwprintw(gui->inner_msg, 0, 0, "Error: parámetro incorrecto \"%s\".", parameter1); // El parámetro no es un número
      }
    }
    else // Si no se especificó un pid
    {
      mvwprintw(gui->inner_msg, 0, 0, "Error: faltan parámetros.");
      mvwprintw(gui->inner_msg, 1, 0, "Sintaxis: KILL «índice»");
    }
  }
  else // Si no es un comando válido
  {
    mvwprintw(gui->inner_msg, 0, 0, "Error: comando \"%s\" no encontrado.", command); // Comando no encontrado
  }
  // Refrescar subventana de mensajes
  wrefresh(gui->inner_msg);
  return 0; // Indica que no salió del programa
}

// Lee una línea del archivo y la almacena en un buffer de línea
int read_line(FILE **f, char *line)
{
  // Se lee una línea del archivo
  fgets(line, INSTRUCTION_SIZE, *f);
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
int interpret_instruction(GUI *gui, char *line, PCB *pcb)
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
    return 2; // No se encontró instrucción

  //  Se comprueba si la instrucción es END
  if (!strcmp(instruction, "END"))
  {
    /* Se limpia área de mensajes con wclear para que redibuje todo
       y no queden residuos de carácteres */
    wclear(gui->inner_msg);
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
    /* Se limpia área de mensajes con wclear para que redibuje todo
       y no queden residuos de carácteres */
    wclear(gui->inner_msg);
    mvwprintw(gui->inner_msg, 0, 0, "Error: registro inválido \"%s\".", p1);
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
      /* Se limpia área de mensajes con wclear para que redibuje todo
         y no queden residuos de carácteres */
      wclear(gui->inner_msg);
      mvwprintw(gui->inner_msg, 0, 0, "Error: registro inválido \"%s\".", p2);
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
        /* Se limpia área de mensajes con wclear para que redibuje todo
           y no queden residuos de carácteres */
        wclear(gui->inner_msg);
        mvwprintw(gui->inner_msg, 0, 0, "Error: división por 0 inválida.");
        return -1; // Instrucción no encontrada
      }
    }
    else // Si la instrucción no es MOV, ADD, SUB, MUL, DIV
    {
      /* Se limpia área de mensajes con wclear para que redibuje todo
         y no queden residuos de carácteres */
      wclear(gui->inner_msg);
      // Si la instrucción es INC o DEC, se indica que hay exceso de parámetros
      if (!strcmp(instruction, "INC") || !strcmp(instruction, "DEC"))
        mvwprintw(gui->inner_msg, 0, 0, "Error: instrucción \"%s\" execeso de parámetros.", instruction);
      else // Si no, se indica que la instrucción no fue encontrada
        mvwprintw(gui->inner_msg, 0, 0, "Error: instrucción inválida \"%s\".", instruction);
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
      /* Se limpia área de mensajes con wclear para que redibuje todo
         y no queden residuos de carácteres */
      wclear(gui->inner_msg);
      // Si la instrucción es MOV, ADD, SUB, MUL, DIV, se indica que faltan parámetros
      if (!strcmp(instruction, "MOV") ||
          !strcmp(instruction, "ADD") ||
          !strcmp(instruction, "SUB") ||
          !strcmp(instruction, "MUL") ||
          !strcmp(instruction, "DIV"))
      {
        mvwprintw(gui->inner_msg, 0, 0, "Error: instrucción \"%s\" faltan parámetros.", instruction);
      }
      else // Si no, se indica que la instrucción no fue encontrada
        mvwprintw(gui->inner_msg, 0, 0, "Error: instrucción inválida \"%s\".", instruction);
      return -1; // Sintaxis de instrucción incorrecta
    }
  }
  // Refrescar subventana de mensajes
  wrefresh(gui->inner_msg);
  return 1; // Instrucción ejecutada correctamente
}