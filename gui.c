#include <ncurses.h>
#include <string.h>
// Bibliotecas propias
#include "gui.h"
#include "pcb.h"
#include "queue.h"

// Inicialización de parámetros de las diferentes cajas
void initialize_gui(GUI *gui)
{
  // Prompt
  gui->prompt = newwin(HEIGHT_PROMPT, WIDTH_PROMPT, STARTY_PROMPT, STARTX_PROMPT);
  gui->inner_prompt = derwin(gui->prompt, HEIGHT_PROMPT - 2, WIDTH_PROMPT - 2, 1, 1);
  // CPU
  gui->cpu = newwin(HEIGHT_CPU, WIDTH_CPU, STARTY_CPU, STARTX_CPU);
  gui->inner_cpu = derwin(gui->cpu, HEIGHT_CPU - 2, WIDTH_CPU - 2, 1, 1);
  // Mensajes
  gui->msg = newwin(HEIGHT_MSG, WIDTH_MSG, STARTY_MSG, STARTX_MSG);
  gui->inner_msg = derwin(gui->msg, HEIGHT_MSG - 2, WIDTH_MSG - 2, 1, 1);
  // Colas
  gui->queues = newwin(HEIGHT_QUEUES, WIDTH_QUEUES, STARTY_QUEUES, STARTX_QUEUES);
  gui->inner_queues = derwin(gui->queues, HEIGHT_QUEUES - 2, WIDTH_QUEUES - 2, 1, 1);
  // Información general
  gui->ginfo = newwin(HEIGHT_GINFO, WIDTH_GINFO, STARTY_GINFO, STARTX_GINFO);
  gui->inner_ginfo = derwin(gui->ginfo, HEIGHT_GINFO - 2, WIDTH_GINFO - 2, 1, 1);
  // SWAP
  gui->swap = newwin(HEIGHT_SWAP, WIDTH_SWAP, STARTY_SWAP, STARTX_SWAP);
  gui->inner_swap = derwin(gui->swap, HEIGHT_SWAP - 2, WIDTH_SWAP - 2, 1, 1);
  // TMS
  gui->tms = newwin(HEIGHT_TMS, WIDTH_TMS, STARTY_TMS, STARTX_TMS);
  gui->inner_tms = derwin(gui->tms, HEIGHT_TMS - 2, WIDTH_TMS - 2, 1, 1);
  // TMP
  gui->tmp = newwin(HEIGHT_TMP, WIDTH_TMP, STARTY_TMP, STARTX_TMP);
  gui->inner_tmp = derwin(gui->tmp, HEIGHT_TMP - 2, WIDTH_TMP - 2, 1, 1);
  // KEYS
  gui->keys = newwin(HEIGHT_KEYS, WIDTH_KEYS, STARTY_KEYS, STARTX_KEYS);
  gui->inner_keys = derwin(gui->keys, HEIGHT_KEYS - 2, WIDTH_KEYS - 2, 1, 1);

  // Dibujar bordes de ventanas con caracteres "bonitos" (0,0)
  box(gui->prompt, 0, 0);
  box(gui->cpu, 0, 0);
  box(gui->msg, 0, 0);
  box(gui->queues, 0, 0);
  box(gui->ginfo, 0, 0);
  box(gui->swap, 0, 0);
  box(gui->tms, 0, 0);
  box(gui->tmp, 0, 0);
  box(gui->keys, 0, 0);

  // Escribir títulos en las cajas de PROMPT, CPU, Mensajes e Información General
  // Línea 0 es el borde superior
  mvwprintw(gui->prompt, 0, (WIDTH_PROMPT - strlen("PROMPT")) / 2, "%s", "PROMPT");
  mvwprintw(gui->cpu, 0, (WIDTH_CPU - strlen("PROCESADOR")) / 2, "%s", "PROCESADOR");
  mvwprintw(gui->msg, 0, (WIDTH_MSG - strlen("MENSAJES")) / 2, "%s", "MENSAJES");
  mvwprintw(gui->ginfo, 0, (WIDTH_GINFO - strlen("INFO GENERAL")) / 2, "%s", "INFO GENERAL");
  mvwprintw(gui->swap, 0, (WIDTH_SWAP - strlen("SWAP")) / 2, "%s", "SWAP");
  mvwprintw(gui->tms, 0, (WIDTH_TMS - strlen("TMS")) / 2, "%s", "TMS");
  mvwprintw(gui->tmp, 0, (WIDTH_TMP - strlen("TMP")) / 2, "%s", "TMP");
  mvwprintw(gui->keys, 0, (WIDTH_KEYS - strlen("TECLAS ESPECIALES")) / 2, "%s", "TECLAS ESPECIALES");

  // Escribir línea estática en la ventanaKEYS
  mvwprintw(gui->inner_keys, 0, 0, "[Up][Down]:History  [Left][Right]:Speed  [PgUp][PgDwn]:TMS  [Ins][Supr]:TMP  [F5][F6]:RAM  [F7][F8]:SWAP  [Alt-][Alt+]:Lists");

  // Refrescar ventanas
  wrefresh(gui->prompt);
  wrefresh(gui->cpu);
  wrefresh(gui->msg);
  wrefresh(gui->queues);
  wrefresh(gui->ginfo);
  wrefresh(gui->swap);
  wrefresh(gui->tms);
  wrefresh(gui->tmp);
  wrefresh(gui->keys);
}

// Se imprime prompt con colores
void print_prompt(WINDOW *inner_prompt, int row)
{
  // Solo color de texto con fondo transparente
  init_pair(1, COLOR_GREEN, -1);
  init_pair(2, COLOR_BLUE, -1);

  // Color verde
  wattron(inner_prompt, COLOR_PAIR(1) | A_BOLD);
  mvwprintw(inner_prompt, row, 0, "artemisos");
  wattroff(inner_prompt, COLOR_PAIR(1) | A_BOLD);

  // Color del sistema
  mvwprintw(inner_prompt, row, 9, ":");

  // Color azul
  wattron(inner_prompt, COLOR_PAIR(2) | A_BOLD);
  mvwprintw(inner_prompt, row, 10, "~");
  wattroff(inner_prompt, COLOR_PAIR(2) | A_BOLD);

  // Color del sistema
  mvwprintw(inner_prompt, row, 11, "$");
}

// Imprime los parámetros del proceso que se encuentre en ejecución
void print_processor(WINDOW *inner_cpu, PCB *pcb)
{
  if (!pcb)
    return;

  // Se limpia la subventana del CPU
  werase(inner_cpu);
  // Parte izquierda
  mvwprintw(inner_cpu, 0, 0, "AX:[%ld]", pcb->AX);
  mvwprintw(inner_cpu, 1, 0, "BX:[%ld]", pcb->BX);
  mvwprintw(inner_cpu, 2, 0, "CX:[%ld]", pcb->CX);
  mvwprintw(inner_cpu, 3, 0, "DX:[%ld]", pcb->DX);
  mvwprintw(inner_cpu, 4, 0, "P:[%d]", pcb->P);
  mvwprintw(inner_cpu, 5, 0, "KCPU:[%d]", pcb->KCPU);
  // Parte derecha
  mvwprintw(inner_cpu, 0, WIDTH_CPU / 2, "PC:[%d]=[0x%04X] ", pcb->PC, pcb->PC);
  mvwprintw(inner_cpu, 1, WIDTH_CPU / 2, "IR:[%s]", pcb->IR);
  mvwprintw(inner_cpu, 2, WIDTH_CPU / 2, "PID:[%d]", pcb->pid);
  mvwprintw(inner_cpu, 3, WIDTH_CPU / 2, "NAME:[%s]", pcb->file_name);
  mvwprintw(inner_cpu, 4, WIDTH_CPU / 2, "UID:[%d]", pcb->UID);
  mvwprintw(inner_cpu, 5, WIDTH_CPU / 2, "KCPUxU:[%d]", pcb->KCPUxU);

  // Refrescar subventana del CPU
  wrefresh(inner_cpu);
}

// Imprime los procesos de las colas
void print_queues(WINDOW *inner_queues, Queue execution, Queue ready, Queue new, Queue finished)
{
  // Se limpia la subventana de las Colas
  werase(inner_queues);

  int row = 0; // Renglón que permite impresión dinámica

  // SECCIÓN EJECUCIÓN
  for (int i = 0; i < WIDTH_QUEUES; i++)
  {
    // ACS_HLINE: caracter de línea horizontal "bonita"
    mvwaddch(inner_queues, row, i, ACS_HLINE);
  }
  mvwprintw(inner_queues, row, (WIDTH_QUEUES - strlen("EJECUCION")) / 2, "%s[%d]", "EJECUCION", execution.elements);
  for (++row; execution.head != NULL; row++)
  {
    mvwprintw(inner_queues, row, 0, "PID:[%u] UID:[%d] P:[%d] KCPU:[%d] KCPUxU:[%d] FILE:[%s] AX:[%ld] BX:[%ld] CX:[%ld] DX:[%ld] PC:[%u] IR:[%s]",
              execution.head->pid, execution.head->UID, execution.head->P, execution.head->KCPU, execution.head->KCPUxU, execution.head->file_name,
              execution.head->AX, execution.head->BX, execution.head->CX, execution.head->DX, execution.head->PC, execution.head->IR);
    execution.head = execution.head->next;
  }

  // SECCIÓN LISTOS
  for (int i = 0; i < WIDTH_QUEUES; i++)
  {
    // ACS_HLINE: caracter de línea horizontal "bonita"
    mvwaddch(inner_queues, row, i, ACS_HLINE);
  }
  mvwprintw(inner_queues, row, (WIDTH_QUEUES - strlen("LISTOS")) / 2, "%s[%d]", "LISTOS", ready.elements);
  for (++row; ready.head != NULL; row++)
  {
    mvwprintw(inner_queues, row, 0, "PID:[%u] UID:[%d] P:[%d] KCPU:[%d] KCPUxU:[%d] FILE:[%s] AX:[%ld] BX:[%ld] CX:[%ld] DX:[%ld] PC:[%u] IR:[%s]",
              ready.head->pid, ready.head->UID, ready.head->P, ready.head->KCPU, ready.head->KCPUxU, ready.head->file_name, ready.head->AX, ready.head->BX,
              ready.head->CX, ready.head->DX, ready.head->PC, ready.head->IR);
    ready.head = ready.head->next;
  }

  // SECCIÓN NUEVOS
  for (int i = 0; i < WIDTH_QUEUES; i++)
  {
    // ACS_HLINE: caracter de línea horizontal "bonita"
    mvwaddch(inner_queues, row, i, ACS_HLINE);
  }
  mvwprintw(inner_queues, row, (WIDTH_QUEUES - strlen("NUEVOS")) / 2, "%s[%d]", "NUEVOS", new.elements);
  for (++row; new.head != NULL; row++)
  {
    mvwprintw(inner_queues, row, 0, "PID:[%u] UID:[%d] P:[%d] KCPU:[%d] KCPUxU:[%d] FILE:[%s] AX:[%ld] BX:[%ld] CX:[%ld] DX:[%ld] PC:[%u] IR:[%s]",
              new.head->pid, new.head->UID, new.head->P, new.head->KCPU, new.head->KCPUxU, new.head->file_name, new.head->AX, new.head->BX,
              new.head->CX, new.head->DX, new.head->PC, new.head->IR);
    new.head = new.head->next;
  }

  // SECCIÓN TERMINADOS
  for (int i = 0; i < WIDTH_QUEUES; i++)
  {
    // ACS_HLINE: caracter de línea horizontal "bonita"
    mvwaddch(inner_queues, row, i, ACS_HLINE);
  }
  mvwprintw(inner_queues, row, (WIDTH_QUEUES - strlen("TERMINADOS")) / 2, "%s[%d]", "TERMINADOS", finished.elements);
  for (++row; finished.head != NULL; row++)
  {
    mvwprintw(inner_queues, row, 0, "PID:[%u] UID:[%d] P:[%d] KCPU:[%d] KCPUxU:[%d] FILE:[%s] AX:[%ld] BX:[%ld] CX:[%ld] DX:[%ld] PC:[%u] IR:[%s]",
              finished.head->pid, finished.head->UID, finished.head->P, finished.head->KCPU, finished.head->KCPUxU, finished.head->file_name,
              finished.head->AX, finished.head->BX, finished.head->CX, finished.head->DX, finished.head->PC, finished.head->IR);
    finished.head = finished.head->next;
  }

  // Se refresca la subventana de Colas
  wrefresh(inner_queues);
}

// Imprime información general
void print_ginfo(WINDOW *inner_ginfo, Queue execution)
{
  // Se limpia la subventana de Información General
  werase(inner_ginfo);

  mvwprintw(inner_ginfo, 0, (WIDTH_GINFO / 3) - 25, "Usuarios:[%d]", NumUs);
  // Se valida que haya algo en ejecución
  if (execution.head)
  {
    mvwprintw(inner_ginfo, 0, ((2 * WIDTH_GINFO) / 3) - 25, "MinP:[%d]", execution.head->P);
  }
  else
  {
    mvwprintw(inner_ginfo, 0, ((2 * WIDTH_GINFO) / 3) - 25, "MinP:[%d]", 0);
  }
  mvwprintw(inner_ginfo, 0, WIDTH_GINFO - 25, "W:[%.2f]", W);
  // Se refresca la subventana de Información General
  wrefresh(inner_ginfo);
}

// Imprime el contenido de la SWAP con desplazamiento
void print_swap(WINDOW *inner_swap, FILE *swap, int swap_disp)
{
  // Se limpia la subventana de Información General de la SWAP
  werase(inner_swap);

  // Muestra información general fija de la SWAP
  mvwprintw(inner_swap, 0, (WIDTH_SWAP - strlen("[65536] Inst en [4096] Marcos de [16] Inst de [32] Bytes c/u = [2097152] Bytes")) / 2, "[65536] Inst en [4096] Marcos de [16] Inst de [32] Bytes c/u = [2097152] Bytes");

  int instructions_per_disp = PAGE_SIZE * DISPLAYED_PAGES_SWAP; // Instrucciones que llenan toda una ventana de SWAP
  char buffer[INSTRUCTION_SIZE] = {0};                          // Buffer que almacena instrucción leída desde archivo SWAP

  // Posiciona el puntero de SWAP en la primera instrucción del marco n, en función del desplazamiento swap_disp
  fseek(swap, instructions_per_disp * INSTRUCTION_JUMP * swap_disp, SEEK_SET);

  // Se imprimen las instrucciones de los 6 marcos conforme al desplazamiento
  for (int page = 0; page < DISPLAYED_PAGES_SWAP; page++)
  { // Recorrer paginas (columnas)
    // Evita imprimir mas marcos de los especificados
    if (swap_disp == TOTAL_DISP_SWAP && page == 4)
      break;
    // Recorrer instrucciones (renglones)
    for (int instruction = 0; instruction < PAGE_SIZE; instruction++)
    {
      fread(buffer, sizeof(char), INSTRUCTION_JUMP, swap);
      // Imprime las instrucciones guardadas en la SWAP junto con su dirección
      mvwprintw(inner_swap, instruction + 1, ((page + 1) * WIDTH_SWAP / 6) - 20, "[%04X] %.12s", (swap_disp * instructions_per_disp + page * PAGE_SIZE) | instruction, buffer);
    }
  }

  // Refresca subventana de swap
  wrefresh(inner_swap);
}

// Imprime el contenido de la TMS con desplazamiento
void print_tms(WINDOW *inner_tms, TMS tms, int tms_disp)
{
  // Se limpia la subventana de la TMS
  werase(inner_tms);

  // Muestra el mensaje de "Marco-PID"
  mvwprintw(inner_tms, 0, 0, "Marco-PID");

  // Se localiza el índice de la tabla tms según el número de desplazamiento
  int index_tms = tms_disp * DISPLAYED_PAGES_TMS;

  // Imprime las páginas de la TMS conforme al desplazamiento
  for (int page = 0; page < DISPLAYED_PAGES_TMS; page++)
  {
    mvwprintw(inner_tms, page + 1, 0, "%03X - %d", index_tms, tms.table[index_tms]);
    index_tms++;
  }

  // Refresca la subventana de TMS
  wrefresh(inner_tms);
}

// Imprime el contenido de la TMP con desplazamiento
void print_tmp(WINDOW *inner_tmp, PCB *pcb, int tmp_disp)
{
  // Se limpia la subventana de la TMP
  werase(inner_tmp);

  // Muestra el mensaje de "Mrc  EnSWAP"
  mvwprintw(inner_tmp, 0, 0, "Mrc  EnSWAP");

  // Si el PCB es nulo, no se imprime nada y se limpió la lsita de direcciones
  if (!pcb)
  {
    // Refresca la subventana de TMP
    wrefresh(inner_tmp);
    return;
  }

  // Se localiza el índice de la tabla TMP según el número de desplazamiento
  int index_tmp = tmp_disp * (DISPLAYED_ADRESSES_TMP);

  // Imprime las direcciones de la TMP conforme al desplazamiento
  for (int address = 0; (tmp_disp * (DISPLAYED_ADRESSES_TMP) + address) < pcb->TmpSize && address < DISPLAYED_ADRESSES_TMP; address++)
  {
    mvwprintw(inner_tmp, address + 1, 0, "%03X,  %03X", index_tmp, pcb->TMP[index_tmp]);
    index_tmp++;
  }

  // Refresca la subventana de TMP
  wrefresh(inner_tmp);
}

// Imprime el historial de comandos en la subventana de prompt
void print_history(char buffers[HISTORY_SIZE][PROMPT_SIZE], WINDOW *inner_prompt)
{
  // Se imprimen los buffers de historial
  for (int i = 1; i < HISTORY_SIZE; i++)
  {
    if (buffers[i][0] != 0) // Si el buffer no está vacío
    {
      clear_prompt(inner_prompt, i);
      print_prompt(inner_prompt, i);
      mvwprintw(inner_prompt, i, PROMPT_START, "%s", buffers[i]);
    }
  }
}

// Se imprime el procesador vacío
void empty_processor(WINDOW *inner_cpu)
{
  PCB empty_pcb = {0}; // Inicializa todos los campos del PCB a 0 o valores equivalentes
  print_processor(inner_cpu, &empty_pcb);
}

// Se limpia la línea de prompt que se especifique
void clear_prompt(WINDOW *inner_prompt, int row)
{
  for (int i = 0; i < PROMPT_SIZE; i++)
  {
    mvwaddch(inner_prompt, row, PROMPT_START + i, ' ');
  }
}