#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
// Bibliotecas propias
#include "pcb.h"
#include "queue.h"
#include "gui.h"
#include "utils.h"

// Variables globales (menos pbase que se ocupa al crear el proceso)
// Se ocupan para calcular la nueva prioridad y el uso del CPU del proceso y usuario
int IncCPU = 60 / MAXQUANTUM;
int NumUs = 0;
double W = 0.0; // Después el peso es 1/NumUs
const int PBase = 60;

// Contador serial del pid's
int pid = 1;

// Actualiza los contadores de uso del CPU para todos los procesos (no Terminados) del usuario dueño del proceso de la cola
void update_KCPUxU_per_process(int uid, Queue *queue)
{
  PCB *current = queue->head; // Nodo actual

  // Se actualiza KCPUxU de cada proceso de la cola que sea del usuario UID
  while (current)
  {
    // El proceso es del usuario UID
    if (current->UID == uid)
    {
      current->KCPUxU += IncCPU; // Se actualiza KCPUxU
    }
    current = current->next; // Se avanza al siguiente nodo
  }
}

// Actualiza los parámetros de planificación, para todos los nodos de la cola
void update_parameters(Queue *queue)
{
  PCB *current = queue->head; // Nodo actual

  // Se actualizan los parámetros de los nodos de la cola
  while (current)
  {
    current->KCPU /= 2;
    current->KCPUxU /= 2;
    current->P = PBase + (current->KCPU) / 2 + (current->KCPUxU) / (4 * W);
    current = current->next; // Se avanza al siguiente nodo
  }
}

// Actualiza el número de usuarios (NumUs) y el peso W
void update_users(int uid, Queue queue)
{
  // Se verifica si el usuario uid ya tiene algún proceso en la cola
  if (!is_user_in_queue(uid, queue))
    NumUs--;

  // Se actualiza el valor de W
  if (NumUs)
    W = 1.0 / NumUs;
  // No hay ningún usuario
  else
    W = 0.0;
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

// Interpreta y ejecuta la instrucción leída de una línea del archivo de programa de un pcb
int interpret_instruction(GUI *gui, char *instruction, PCB *pcb, TMS *tms, TMM *tmm,
                          Queue *execution, Queue *ready, Queue *finished, Queue *new,
                          int *tms_disp, FILE **swap, int *clock)
{
  /* TOKENS */
  char mnemonic[10] = {0}; // Mnemónico de la instrucción
  char p1[12] = {0};       // Primer parámetro (registro)
  char p2[12] = {0};       // Segundo parámetro (registro o número)
  int number_p2 = 0;       // Valor numérico si p2 es un número

  int is_num_p2 = false; // Bandera que indica si p2 es numérico

  // Se pasa a mayúsculas la instrucción leída
  str_upper(instruction);

  // Se almacena la instrucción en el registro IR del pcb actual
  strcpy(pcb->IR, instruction);

  // Se separa en tokens la línea leída y se calcula cuántos hay
  int items = sscanf(instruction, "%s %s %s", mnemonic, p1, p2);

  //  Se comprueba si la instrucción es END
  if (!strcmp(mnemonic, "END"))
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

  // Se busca el registro del primer parámetro si lo hay
  int reg1 = search_register(p1);

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
    // Se comprueba que el primer parámetro sea un registro válido
    if (reg1 == -1)
    {
      /* Se limpia área de mensajes con wclear para que redibuje todo
         y no queden residuos de carácteres */
      wclear(gui->inner_msg);
      mvwprintw(gui->inner_msg, 0, 0, "Error: registro inválido \"%s\".", p1);
      return -1; // El primer parámetro es un registro inválido
    }

    if (!strcmp(mnemonic, "MOV"))
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
    else if (!strcmp(mnemonic, "ADD"))
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
    else if (!strcmp(mnemonic, "SUB"))
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
    else if (!strcmp(mnemonic, "MUL"))
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
    else if (!strcmp(mnemonic, "DIV"))
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
      // Si la instrucción es INC, DEC o JNZ, se indica que hay exceso de parámetros
      if (!strcmp(mnemonic, "INC") || !strcmp(mnemonic, "DEC") || !strcmp(mnemonic, "JNZ"))
        mvwprintw(gui->inner_msg, 0, 0, "Error: instrucción \"%s\" execeso de parámetros.", mnemonic);
      else // Si no, se indica que la instrucción no fue encontrada
        mvwprintw(gui->inner_msg, 0, 0, "Error: instrucción inválida \"%s\".", mnemonic);
      return -1; // Sintaxis de instrucción incorrecta
    }
  }
  else if (items == 2) // Instrucciones de 2 paŕametros (INC, DEC, JNZ)
  {
    if (!strcmp(mnemonic, "INC"))
    {
      // Se comprueba que el primer parámetro sea un registro válido
      if (reg1 == -1)
      {
        /* Se limpia área de mensajes con wclear para que redibuje todo
           y no queden residuos de carácteres */
        wclear(gui->inner_msg);
        mvwprintw(gui->inner_msg, 0, 0, "Error: registro inválido \"%s\".", p1);
        return -1; // El primer parámetro es un registro inválido
      }

      if (reg1 == 'A')
        pcb->AX += 1;
      else if (reg1 == 'B')
        pcb->BX += 1;
      else if (reg1 == 'C')
        pcb->CX += 1;
      else
        pcb->DX += 1;
    }
    else if (!strcmp(mnemonic, "DEC"))
    {
      // Se comprueba que el primer parámetro sea un registro válido
      if (reg1 == -1)
      {
        /* Se limpia área de mensajes con wclear para que redibuje todo
           y no queden residuos de carácteres */
        wclear(gui->inner_msg);
        mvwprintw(gui->inner_msg, 0, 0, "Error: registro inválido \"%s\".", p1);
        return -1; // El primer parámetro es un registro inválido
      }

      if (reg1 == 'A')
        pcb->AX -= 1;
      else if (reg1 == 'B')
        pcb->BX -= 1;
      else if (reg1 == 'C')
        pcb->CX -= 1;
      else
        pcb->DX -= 1;
    }
    else if (!strcmp(mnemonic, "JNZ"))
    {
      int is_num_p1 = is_numeric(p1);
      // JNZ ## - Brinca a la instrucción ## si el registro CX!=0
      if (pcb->CX != 0 && is_num_p1)
      {
        // Se extrae número del primer parámetro
        int number_p1 = atoi(p1);
        // Se comprueba que no haya violación de segmento
        if (number_p1 > -1 && number_p1 < pcb->lines)
          // Se resta -1 para que no se incremente después de terminar la función (como el caso de las otras instrucciones)
          pcb->PC = number_p1 - 1;
        else
        {
          // Se actualiza el número de usuarios (NumUs) y el peso W
          update_users(pcb->UID, *ready);
          // El proceso termina y se realiza la gestión necesaria
          handle_process_termination(gui, pcb, execution, ready, new, tms, tmm, *tms_disp, swap, clock);
          // Se encola el proceso en Terminados
          enqueue(pcb, finished);
          // Se imprime procesador vacío y se muestra mensaje
          empty_processor(gui->inner_cpu);
          mvwprintw(gui->inner_msg, 3, 0, "Violación de segmento. El proceso [%d] no tiene marco %d.", pcb->pid, number_p1 / PAGE_SIZE);
        }
      }
    }
    else // Si la instrucción no es INC o DEC
    {
      /* Se limpia área de mensajes con wclear para que redibuje todo
         y no queden residuos de carácteres */
      wclear(gui->inner_msg);
      // Si la instrucción es MOV, ADD, SUB, MUL, DIV, se indica que faltan parámetros
      if (!strcmp(mnemonic, "MOV") ||
          !strcmp(mnemonic, "ADD") ||
          !strcmp(mnemonic, "SUB") ||
          !strcmp(mnemonic, "MUL") ||
          !strcmp(mnemonic, "DIV"))
      {
        mvwprintw(gui->inner_msg, 0, 0, "Error: instrucción \"%s\" faltan parámetros.", mnemonic);
      }
      else // Si no, se indica que la instrucción no fue encontrada
        mvwprintw(gui->inner_msg, 0, 0, "Error: instrucción inválida \"%s\".", mnemonic);
      return -1; // Sintaxis de instrucción incorrecta
    }
  }
  // Refrescar subventana de mensajes
  wrefresh(gui->inner_msg);
  return 1; // Instrucción ejecutada correctamente
}

// Gestiona la terminación de un proceso en ejecución, actualizando las colas y la TMS
void handle_process_termination(GUI *gui, PCB *current_process, Queue *execution, Queue *ready,
                                Queue *new, TMS *tms, TMM *tmm, int tms_disp, FILE **swap, int *clock)
{
  PCB *brother_process = NULL; // Puntero para almacenar el hermano del proceso

  // Si no queda ningún proceso hermano ya sea en Listos o en Ejecución
  if (!((brother_process = search_brother_process(current_process->UID, current_process->file_name, *ready)) ||
        (brother_process = search_brother_process(current_process->UID, current_process->file_name, *execution))))
  {
    // Establecer sus marcos de SWAP cómo libres en la TMS
    free_pages_from_tms(current_process, tms);
    /* Indicar en la TMM que los marcos en RAM ya no están siendo
    utilizados por proceso alguno, estableciendo en 0 tanto Proceso cómo Referencia. */
    free_pages_from_tmm(current_process, tmm);
    // Liberar la memoria que ocupe la TMP asociada
    free(current_process->tmp.inSWAP);
    // Se pone a NULL la TMP del proceso para evitar puntero colgante
    current_process->tmp.inSWAP = NULL;
    // Si hay procesos en la lista de Nuevos
    if (new->head)
    {
      // Buscar por orden de llegada, algún proceso que sea de menor tamaño que la SWAP libre
      // Mientras se encuentren procesos que se quepan en swap
      PCB *process_fits = NULL; // Puntero para almacenar el proceso que cabe en la SWAP
      while ((process_fits = search_process_fits_swap(new, tms->available_pages)))
      {
        // Cargar el proceso a Listos
        load_to_ready(process_fits, ready, tms, swap);

        // Se limpia el área de mensajes
        werase(gui->inner_msg);

        // Se imprime mensaje de proceso de Nuevos encolado a Listos
        mvwprintw(gui->inner_msg, 0, 0, "Proceso: %d [%s] UID: [%d]", process_fits->pid, process_fits->file_name, process_fits->UID);
        mvwprintw(gui->inner_msg, 1, 0, "Proceso de Nuevos -> Listos, cargado a la SWAP.");

        /* Una vez cargado en Listos, se busca entre los procesos Nuevos si hay hermanos del proceso recién cargado
           para cargarlo en Listos, pero asignarle la misma TMP */
        while ((brother_process = extract_brother_process(process_fits->UID, process_fits->file_name, new)))
        {
          // Asignar la misma TMP que su hermano
          brother_process->tmp = process_fits->tmp;
          // Se cierra el archivo del proceso hermano
          fclose(brother_process->program);
          // Se evita puntero colgante
          brother_process->program = NULL;
          // Se encola a listos
          enqueue(brother_process, ready);
        }
      }
    }
  }
  else // Si aún hay procesos hermanos
  {
    // Actualiza el PID en TMS para el hermano encontrado que sigue vivo
    update_pages_from_tms(brother_process, tms);
    // Actualizar TMM con el PID del hermano para indicar que la RAM ahora pertenece al él
    update_pages_from_tmm(brother_process, tmm);
  }

  // Se muestran los cambios en la TMS y TMM
  print_tms(gui->inner_tms, *tms, tms_disp);
  print_tmm(gui->inner_tmm, *tmm, *clock);
  // Se refresca la subventana de mensajes
  wrefresh(gui->inner_msg);
}

// Recalcular prioridades de la cola de Listos y mostrar mensaje
void recalculate_priorities(GUI *gui, Queue ready, int *minor_priority)
{
  // Se limpia el área de mensajes
  werase(gui->inner_msg);

  // Se imprime mensaje de recalcular
  mvwprintw(gui->inner_msg, 0, 0, "Recalculando prioridades del planificador...");
  *minor_priority = get_minor_priority(ready);
  // Se imprime la menor prioridad encontrada
  mvwprintw(gui->inner_msg, 1, 0, "Menor prioridad encontrada: [%d]", *minor_priority);
  // Se imprime el tiempo antes de continuar
  mvwprintw(gui->inner_msg, 3, 0, "Continuando...");
  // Se refresca la subventana de mensajes
  wrefresh(gui->inner_msg);
  // Se espera 2 segundos para que el usuario pueda leer el mensaje
  sleep(2);
}

Address address_traduction(PCB *current_process)
{
  Address address = {0};
  if (current_process)
  {
    address.base_page = current_process->PC / PAGE_SIZE;
    address.offset = current_process->PC % PAGE_SIZE;
    address.real = address.base_page | address.offset;
  }
  return address;
}
