#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
// Bibliotecas propias
#include "kbhit.h"
#include "pcb.h"
#include "queue.h"
#include "gui.h"
#include "prompt.h"
#include "utils.h"

// Inicializa el buffer de prompt y el índice
void initialize_buffer(char *buffer, int *index)
{
  // Se limpia el buffer
  for (int i = 0; i < PROMPT_SIZE; i++)
  {
    buffer[i] = 0;
  }
  // Se reinicia el índice
  *index = 0;
}

// Crea el historial
void create_history(GUI *gui, char buffers[HISTORY_SIZE][PROMPT_SIZE], int *index, int *index_history)
{
  // Se copia el buffer actual al primer buffer de historial
  for (int i = HISTORY_SIZE - 1; i >= 0; i--)
    strcpy(buffers[i], buffers[i - 1]);

  // Se Reinicia índice de historial
  *index_history = 0;
  // Se limpia el buffer
  initialize_buffer(buffers[0], index);
  // Se limpia la prompt
  clear_prompt(gui->inner_prompt, 0);
}

// Manejo de comandos ingresados por el usuario en la línea de comandos
void command_handling(GUI *gui, int *exited, char buffers[HISTORY_SIZE][PROMPT_SIZE],
                      int *index, int *index_history,
                      Queue *execution, Queue *ready, Queue *finished, Queue *new,
                      unsigned *timer, unsigned *init_timer, int *speed_level, TMS *tms, FILE **swap,
                      int *swap_disp, int *tms_disp, int *tmp_disp, int * lists_disp, PCB *execution_pcb)
{
  int c = 0; // Carácter introducido por el usuario
  // Si se presionó una tecla en la terminal (kbhit)
  if (kbhit())
  {
    // Se obtiene la tecla presionada en la subventana de prompt
    c = wgetch(gui->inner_prompt);
    if (c == ENTER) // Si se presionó ENTER
    {
      // Se le coloca carácter nulo para finalizar la cadena prompt
      buffers[0][*index] = '\0';
      // Se evalua el comando en buffer
      evaluate_command(gui, exited, buffers[0], execution, ready, finished,
                       new, tms, swap, swap_disp, tms_disp, tmp_disp, lists_disp);
      // Se crea historial
      create_history(gui, buffers, index, index_history);
      // Se imprimen los buffers de historial
      print_history(buffers, gui->inner_prompt);
      // Se coloca el cursor en su lugar
      wmove(gui->inner_prompt, 0, PROMPT_START); 
      // Significa que escribió el comando EXIT
      if (exited)                                
        return; // Sale del gestor de comandos
    }
    else if (c == KEY_BACKSPACE) // Retrocede un carácter
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
    else if (c == KEY_UP) // Avanza a buffers superiores
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
    else if (c == KEY_DOWN) // Comando anterior (avanza a buffers inferiores)
    {
      /*  Se comprueba que no se salga del arreglo de buffers y que el
      el comando anterior no sea nulo */
      if (*index_history < HISTORY_SIZE - 1 && buffers[(*index_history) + 1][0] != '\0')
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
    else if (c == KEY_LEFT) // Disminuye la velocidad de escritura
    {
      if (*speed_level > 1) // Que sea mayor que el nivel 1 de velocidad
      {
        (*init_timer) -= MAX_TIME / ((int)pow(2, *speed_level - 1)); // Se decrementa el temporizador
        (*speed_level)--;                                            // Se decrementa el nivel de velocidad
      }
    }
    else if (c == KEY_RIGHT) // Aumenta la velocidad de escritura
    {
      if (*speed_level < MAX_LEVEL) // Que no sobrepase el límite máximo de niveles
      {
        (*init_timer) += MAX_TIME / ((int)pow(2, *speed_level)); // Se incrementa el temporizador
        (*speed_level)++;                                        // Se incrementa el nivel de velocidad
      }
    }
    else if (c == KEY_PPAGE) // Retrocede TMS (PgUp)
    {
      if ((*tms_disp) > 0)
      {
        (*tms_disp)--;
      }
      print_tms(gui->inner_tms, *tms, *tms_disp);
    }
    else if (c == KEY_NPAGE) // Avanza TMS (PgDn)
    {
      if ((*tms_disp) < TOTAL_DISP_TMS)
      {
        (*tms_disp)++;
      }
      print_tms(gui->inner_tms, *tms, *tms_disp);
    }
    else if (c == KEY_IC) // Retrocede TMP (ins)
    {
      if ((*tmp_disp) > 0)
      {
        (*tmp_disp)--;
      }
      print_tmp(gui->inner_tmp, execution_pcb, *tmp_disp);
    }
    else if (c == KEY_DC) // Avanza TMP (supr)
    {
      // Total de desplazamientos de TMP
      int total_disp_tmp = (execution_pcb) ? execution_pcb->TmpSize / (DISPLAYED_ADRESSES_TMP) : 0;
      if ((*tmp_disp) < total_disp_tmp)
      {
        (*tmp_disp)++;
      }
      print_tmp(gui->inner_tmp, execution_pcb, *tmp_disp);
    }
     else if (c == KEY_F(1)) // Retrocede Colas (F1)
    {
      if ((*lists_disp) > 0)
      {
        (*lists_disp)--;
      }
      print_queues(gui->inner_queues, *execution, *ready, *new, *finished, *lists_disp);
    }
    else if (c == KEY_F(2)) // Avanza Colas (F2)
    {
      // Se calcula el total de deslazamientos de la ventana de colas en función de los elementos que haya
      int total_lists_disp = (execution->elements + ready->elements + new->elements + finished->elements + 4) / (HEIGHT_QUEUES - 2);
      if ((*lists_disp) < total_lists_disp)
      {
        (*lists_disp)++;
      }
      print_queues(gui->inner_queues, *execution, *ready, *new, *finished, *lists_disp);
    }
    else if (c == KEY_F(5)) // Retrocede RAM (F5)
    {
    }
    else if (c == KEY_F(6)) // Avanza RAM (F6)
    {
    }
    else if (c == KEY_F(7)) // Retrocede SWAP (F7)
    {
      if ((*swap_disp) > 0)
      {
        (*swap_disp)--;
      }
      print_swap(gui->inner_swap, *swap, *swap_disp);
    }
    else if (c == KEY_F(8)) // Avanza SWAP (F8)
    {
      if ((*swap_disp) < TOTAL_DISP_SWAP)
      {
        (*swap_disp)++;
      }
      print_swap(gui->inner_swap, *swap, *swap_disp);
    }
    else if (c == ESC) // Acceso rápido para salir
    {
      exit_command(exited, gui, execution, ready, new, finished);
    }
    else // Si se presionó cualquier otra tecla
    {
      if (*index < PROMPT_SIZE - 1)
      {
        // Concatena la tecla presionada al buffer para su posterior impresión
        buffers[0][(*index)++] = c;
        mvwaddch(gui->inner_prompt, 0, (PROMPT_START - 1) + (*index), c);
      }
    }
  }
  (*timer)++; // Se incrementa el temporizador
}

void load_command(char *parameter1, char *parameter2, Queue *execution, Queue *ready, Queue *new, Queue *finished,
                  TMS *tms, FILE **swap, int tms_disp, int swap_disp, int lists_disp, GUI *gui)
{
  FILE *file = NULL;  // Almacena el puntero al archivo a cargar
  int value_par2 = 0; // Almacena el uid como valor numerico
  int tmp_size = 0;   // Almacena el número de marcos del archivo a cargar
  int lines = 0;      // Almacena el número de líneas del archivo a cargar
  int KCPUxU = 0;     // Almacena el valor de KCPUxU del usuario

  // Se limpia el área de mensajes
  werase(gui->inner_msg);

  // Si no se especificó un archivo a cargar
  if (!parameter1[0])
  {
    mvwprintw(gui->inner_msg, 0, 0, "Error: faltan parámetros.");
    mvwprintw(gui->inner_msg, 1, 0, "Sintaxis: LOAD «archivo» «uid».");
    wrefresh(gui->inner_msg);
    return;
  }

  // No metió un id de usuario
  if (parameter2[0] == '\0')
  {
    mvwprintw(gui->inner_msg, 0, 0, "Error: debes ingresar un id de usuario.");
    wrefresh(gui->inner_msg);
    return;
  }
  // Si no se especificó un id de usuario válido (entero)
  else if (!is_numeric(parameter2))
  {
    mvwprintw(gui->inner_msg, 0, 0, "Error: el id de usuario debe ser un número entero.");
    wrefresh(gui->inner_msg);
    return;
  }

  // Se obtiene valor numérico del parámetro dos
  value_par2 = atoi(parameter2);
  // Se abre el archivo en modo lectura
  file = fopen(parameter1, "r");
  if (!file)
  {
    mvwprintw(gui->inner_msg, 1, 0, "Error: no se pudo abrir el archivo %s.", parameter1);
    wrefresh(gui->inner_msg);
    return;
  }

  // Total de lineas en el archivo ignorando líneas vacías
  lines = count_lines(file);
  // Se calculan el número de marcos/páginas que ocupará el proceso en el SWAP
  tmp_size = (int)ceil((double)lines / PAGE_SIZE);

  // Se crea el proceso inicializando sus atributos
  PCB *new_process = create_pcb(pid, parameter1, &file, value_par2, tmp_size, lines);
  if (!new_process)
  {
    mvwprintw(gui->inner_msg, 1, 0, "Error: no pudo crear el proceso %d [%s].", pid, parameter1);
    wrefresh(gui->inner_msg);
    return;
  }

  // Incremento de pid para el siguiente proceso a crear
  pid++;

  /*
    Se verifica si el usuario del nuevo proceso ya existe o no (tiene algún proceso en Ejecución o Listos).
    Si no existe el usuario, se incrementa el NumUs.
    Si existe, el KCPUxU se empareja con el resto de procesos del usuario.
  */
  if (!is_user_in_queue(new_process->UID, *execution) && !is_user_in_queue(new_process->UID, *ready))
    NumUs++;
  /* El usuario ya tiene procesos, se actualiza KCPUxU en común al usuario,
     Se verifica primero si hay algún proceso en Listos para actualizarlo*/
  else if ((KCPUxU = get_KCPUxU(new_process->UID, *ready)) != -1)
    new_process->KCPUxU = KCPUxU;
  else // Si no hay un proceso del usuario en Listos, seguramente está en Ejecución
    new_process->KCPUxU = execution->head->KCPUxU;

  // Se actualiza el peso W una vez creado el nuevo proceso y actualizado el NumUs
  if (NumUs)
    W = 1.0 / NumUs;

  /* Buscar si el programa, ya se encuentra previamente cargado por algún otro proceso
    del mismo usuario. Ya sea en Listos o en Ejecución */
  PCB *brother_process = NULL;
  if ((brother_process = search_brother_process(new_process->UID, new_process->file_name, *execution))) // Brother en Ejecución
  {
    // Asignar la misma TMP al nuevo proceso
    new_process->TMP = brother_process->TMP;
    // Inserta el nodo en la cola Listos
    enqueue(new_process, ready);
    // Se imprime mensaje de proceso copiado
    mvwprintw(gui->inner_msg, 0, 0, "Proceso: %d [%s] UID: [%d], encontrado en SWAP.", brother_process->pid, brother_process->file_name, brother_process->UID);
    mvwprintw(gui->inner_msg, 1, 0, "Programa [%s] ya cargado. Se comparte TMP.", brother_process->file_name);
    // Se cierra el archivo una vez que se ha cargado en la swap
    fclose(file);
    // Se evita puntero colgante
    new_process->program = NULL;
  }
  else if ((brother_process = search_brother_process(new_process->UID, new_process->file_name, *ready))) // Brother en Listos
  {
    // Asignar la misma TMP al nuevo proceso
    new_process->TMP = brother_process->TMP;
    // Inserta el nodo en la cola Listos
    enqueue(new_process, ready);
    // Se imprime mensaje de proceso copiado
    mvwprintw(gui->inner_msg, 0, 0, "Proceso: %d [%s] UID: [%d], encontrado en SWAP.", brother_process->pid, brother_process->file_name, brother_process->UID);
    mvwprintw(gui->inner_msg, 1, 0, "Programa [%s] ya cargado. Se comparte TMP.", brother_process->file_name);
    // Se cierra el archivo una vez que se ha cargado en la swap
    fclose(file);
    // Se evita puntero colgante
    new_process->program = NULL;
  }
  // Flujo de acción para procesos sin hermanos
  // Verificar que el proceso sea de menor tamaño que la SWAP
  else if (lines <= SWAP_SIZE)
  {
    // Si la cantidad de marcos libres en SWAP alcanzan para cargar el proceso
    if (new_process->TmpSize <= tms->available_pages)
    {
      // Cargarlo a Listos y en SWAP
      load_to_ready(new_process, ready, tms, swap);

      // Se imprime mensaje de proceso creado
      mvwprintw(gui->inner_msg, 0, 0, "Proceso: %d [%s] UID: [%d]", new_process->pid, new_process->file_name, new_process->UID);
      mvwprintw(gui->inner_msg, 1, 0, "Creado correctamente.");

      // Se imprimen la SWAP y TMS dado que son procesos con instrucciones nuevas
      print_swap(gui->inner_swap, *swap, swap_disp);
      print_tms(gui->inner_tms, *tms, tms_disp);
    }
    // De lo contrario, inserta el proceso al final de la lista Nuevos (puede entrar después)
    else
    {
      // Insertar el nuevo proceso en la cola Nuevos
      enqueue(new_process, new);
      // Se imprime mensaje de proceso creado e insertado en Nuevos
      mvwprintw(gui->inner_msg, 0, 0, "Proceso: %d [%s] UID: [%d] -> Nuevos", new_process->pid, new_process->file_name, new_process->UID);
      mvwprintw(gui->inner_msg, 1, 0, "Sin swap disponible");
    }
  }
  else // La swap no es suficiente para cargar el proceso
  {
    // Enviar el proceso directamente a Terminados, indicando el motivo
    enqueue(new_process, finished);
    mvwprintw(gui->inner_msg, 0, 0, "Proceso: %d [%s] UID: [%d] -> Terminados", new_process->pid, new_process->file_name, new_process->UID);
    mvwprintw(gui->inner_msg, 1, 0, "El programa excede el tamaño del SWAP. Actualice el sistema.");
    // Se cierra el archivo dado que no se cargó en la swap
    fclose(file);
    // Se evita puntero colgante
    new_process->program = NULL;
  }

  // Se imprimen las colas para que se refleje los nodos que vayamos ingresando de forma instántanea
  print_queues(gui->inner_queues, *execution, *ready, *new, *finished, lists_disp);
  // Se refresca el área de mensajes
  wrefresh(gui->inner_msg); 
}

void kill_command(char *parameter1, Queue *execution, Queue *ready, Queue *new, Queue *finished,
                  TMS *tms, FILE **swap, int tms_disp, int lists_disp, GUI *gui)
{
  int pid_to_search = 0; // Variable para almacenar el pid del proceso a matar

  // Se limpia el área de mensajes
  werase(gui->inner_msg);

  // Si no se especificó un pid a matar
  if (!parameter1[0])
  {
    mvwprintw(gui->inner_msg, 0, 0, "Error: faltan parámetros.");
    mvwprintw(gui->inner_msg, 1, 0, "Sintaxis: KILL «pid».");
    wrefresh(gui->inner_msg);
    return;
  }

  // Si el parámetro no es numérico
  if (!is_numeric(parameter1))
  {
    mvwprintw(gui->inner_msg, 0, 0, "Error: el pid debe ser un número entero.");
    wrefresh(gui->inner_msg);
    return;
  }

  // Se convierte el parámetro a valor numérico
  pid_to_search = atoi(parameter1);

  PCB *process_extracted = NULL; // Puntero para almacenar el proceso extraído

  // El proceso se encontró en la cola de Ejecución
  if ((process_extracted = extract_by_pid(pid_to_search, execution)))
  {
    // Se actualiza el número de usuarios (NumUs) y el peso W
    update_users(process_extracted->UID, *ready);
    // El proceso termina y se realiza la gestión necesaria
    handle_process_termination(gui, process_extracted, execution, ready, new, tms, tms_disp, swap);
    // Se encola el proceso en Terminados
    enqueue(process_extracted, finished);
    // Se imprime procesador vacío y se muestra mensaje
    empty_processor(gui->inner_cpu);
    mvwprintw(gui->inner_msg, 3, 0, "El process con pid [%d] ha sido terminado.", pid_to_search);
  }
  // El proceso se encontró en la cola de Listos
  else if ((process_extracted = extract_by_pid(pid_to_search, ready)))
  {
    // Se actualiza el número de usuarios (NumUs) y el peso W
    update_users(process_extracted->UID, *ready);
    // El proceso termina y se realiza la gestión necesaria
    handle_process_termination(gui, process_extracted, execution, ready, new, tms, tms_disp, swap);
    // Se encola el pcb en Terminados
    enqueue(process_extracted, finished);
    // Se imprime mensaje de terminación
    mvwprintw(gui->inner_msg, 3, 0, "El pcb con pid [%d] ha sido terminado.", pid_to_search);
  }
  // El proceso no se encontró en ninguna cola
  else
    mvwprintw(gui->inner_msg, 0, 0, "Error: no se pudo encontrar el pcb con pid [%d].", pid_to_search); // No se encontro el índice

  // Se imprimen las colas después de los cambios realizados
  print_queues(gui->inner_queues, *execution, *ready, *new, *finished, lists_disp);

  // Se refresca el área de mensajes
  wrefresh(gui->inner_msg);
}

void exit_command(int *exited, GUI *gui, Queue *execution, Queue *ready, Queue *new, Queue *finished)
{
  /* Se limpia área de mensajes con wclear para que redibuje todo
    y no queden residuos de carácteres */
  wclear(gui->inner_msg);

  // Se confirma si quiere salir del programa
  if (confirm_exit(gui))
  {
    // Se liberan todas las colas
    free_queues(execution, ready, finished, new);
    *exited = true;
    return;
  }
  // Se limpia área de mensajes
  werase(gui->inner_msg);
  // Se coloca el cursor en su lugar
  wmove(gui->inner_prompt, 0, PROMPT_START);
  // Se refresca la subventana de mensajes
  wrefresh(gui->inner_msg);
}

// Pregunta si quiere salir del simulador
int confirm_exit(GUI *gui)
{
  // Se imprime mensaje de confirmación
  mvwprintw(gui->inner_msg, 0, 0, "¿Seguro que quieres salir? [S/N]:");
  // Se obtiene la confirmación
  char confirmation = toupper(wgetch(gui->inner_msg));
  if (confirmation == 'S')
  {
    return true;
  }
  return false;
}

// Evalúa los comandos ingresados por el usuario
void evaluate_command(GUI *gui, int *exited, char *buffer, Queue *execution, Queue *ready, Queue *finished, Queue *new,
                      TMS *tms, FILE **swap, int *swap_disp, int *tms_disp, int *tmp_disp, int *lists_disp)
{
  /* TOKENS */
  char command[256] = {0};    // Almacena el comando ingresado
  char parameter1[256] = {0}; // Almacena el nombre del archivo a cargar o el nombre del proceso a matar
  char parameter2[256] = {0}; // Almacena el id de usuario, se analiza con is_numeric

  // Se separa en tokens el comando leído de prompt
  sscanf(buffer, "%s %s %s", command, parameter1, parameter2); // Que pasa si como segundo parámetro es una a?

  // Se convierte el comando a mayúsculas
  str_upper(command);

  // Se verifica si el comando es EXIT y no tiene parámetros
  if (!(strcmp(command, "EXIT")) && !parameter1[0])
  {
    exit_command(exited, gui, execution, ready, new, finished);
  }
  else if (!strcmp(command, "LOAD")) // Si el comando es LOAD
  {
    load_command(parameter1, parameter2, execution, ready, new, finished,
                 tms, swap, *tms_disp, *swap_disp, *lists_disp, gui);
  }
  else if (!strcmp(command, "KILL")) // Si el comando es KILL
  {
    kill_command(parameter1, execution, ready, new, finished,
                 tms, swap, *tms_disp, *lists_disp, gui);
  }
  else // Si no es un comando válido
  {
    // Comando no encontrado
    mvwprintw(gui->inner_msg, 0, 0, "Error: comando \"%s\" no encontrado.", command);
    // Se refresca el área de mensajes
    wrefresh(gui->inner_msg);
  }
}