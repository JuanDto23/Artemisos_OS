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

  // Dibujar bordes de ventanas con caracteres "bonitos" (0,0)
  box(gui->prompt, 0, 0);
  box(gui->cpu, 0, 0);
  box(gui->msg, 0, 0);
  box(gui->queues, 0, 0);
  box(gui->ginfo, 0, 0);

  // Escribir títulos en las cajas de CPU y Mensajes
  // Línea 0 es el borde superior
  mvwprintw(gui->prompt, 0, (WIDTH_PROMPT - strlen("PROMPT")) / 2, "%s", "PROMPT");
  mvwprintw(gui->cpu, 0, (WIDTH_CPU - strlen("PROCESADOR")) / 2, "%s", "PROCESADOR");
  mvwprintw(gui->msg, 0, (WIDTH_MSG - strlen("MENSAJES")) / 2, "%s", "MENSAJES");
  // Ventana de Colas no lleva título
  mvwprintw(gui->ginfo, 0, (WIDTH_GINFO - strlen("INFO GENERAL")) / 2, "%s", "INFO GENERAL");

  // Refrescar ventanas
  wrefresh(gui->prompt);
  wrefresh(gui->cpu);
  wrefresh(gui->msg);
  wrefresh(gui->queues);
  wrefresh(gui->ginfo);
}

// Se imprime prompt con colores
void print_prompt(WINDOW *inner_prompt, int row)
{
  // Solo color de texto con fondo transparente
  init_pair(1, COLOR_GREEN, -1);
  init_pair(2, COLOR_WHITE, -1);
  init_pair(3, COLOR_BLUE, -1);

  wattron(inner_prompt, COLOR_PAIR(1) | A_BOLD);
  mvwprintw(inner_prompt, row, 0, "artemisos");
  wattroff(inner_prompt, COLOR_PAIR(1) | A_BOLD);

  wattron(inner_prompt, COLOR_PAIR(2) | A_BOLD);
  mvwprintw(inner_prompt, row, 9, ":"),
      wattroff(inner_prompt, COLOR_PAIR(2) | A_BOLD);

  wattron(inner_prompt, COLOR_PAIR(3) | A_BOLD);
  mvwprintw(inner_prompt, row, 10, "~"),
      wattroff(inner_prompt, COLOR_PAIR(3) | A_BOLD);

  wattron(inner_prompt, COLOR_PAIR(2) | A_BOLD);
  mvwprintw(inner_prompt, row, 11, "$"),
      wattroff(inner_prompt, COLOR_PAIR(2) | A_BOLD);
}

// Imprime los parámetros del proceso que se encuentre en ejecución
void print_processor(WINDOW *inner_cpu, PCB pcb)
{
  // Se limpia la subventana del CPU
  werase(inner_cpu);
  // Parte izquierda
  mvwprintw(inner_cpu, 0, 0, "AX:[%ld]", pcb.AX);
  mvwprintw(inner_cpu, 1, 0, "BX:[%ld]", pcb.BX);
  mvwprintw(inner_cpu, 2, 0, "CX:[%ld]", pcb.CX);
  mvwprintw(inner_cpu, 3, 0, "DX:[%ld]", pcb.DX);
  mvwprintw(inner_cpu, 4, 0, "P:[%d]", pcb.P);
  mvwprintw(inner_cpu, 5, 0, "KCPU:[%d]", pcb.KCPU);
  // Parte derecha
  mvwprintw(inner_cpu, 0, WIDTH_CPU / 2, "PC:[%d]", pcb.PC);
  mvwprintw(inner_cpu, 1, WIDTH_CPU / 2, "IR:[%s]", pcb.IR);
  mvwprintw(inner_cpu, 2, WIDTH_CPU / 2, "PID:[%d]", pcb.pid);
  mvwprintw(inner_cpu, 3, WIDTH_CPU / 2, "NAME:[%s]", pcb.file_name);
  mvwprintw(inner_cpu, 4, WIDTH_CPU / 2, "UID:[%d]", pcb.UID);
  mvwprintw(inner_cpu, 5, WIDTH_CPU / 2, "KCPUxU:[%d]", pcb.KCPUxU);

  // Refrescar subventana del CPU
  wrefresh(inner_cpu);
}

// Imprime los procesos de las colas
void print_queues(WINDOW *inner_queues, Queue execution, Queue ready, Queue finished)
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
  mvwprintw(inner_queues, row, (WIDTH_QUEUES - strlen("EJECUCION")) / 2, "%s", "EJECUCION");
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
  mvwprintw(inner_queues, row, (WIDTH_QUEUES - strlen("LISTOS")) / 2, "%s", "LISTOS");
  for (++row; ready.head != NULL; row++)
  {
    mvwprintw(inner_queues, row, 0, "PID:[%u] UID:[%d] P:[%d] KCPU:[%d] KCPUxU:[%d] FILE:[%s] AX:[%ld] BX:[%ld] CX:[%ld] DX:[%ld] PC:[%u] IR:[%s]",
              ready.head->pid, ready.head->UID, ready.head->P, ready.head->KCPU, ready.head->KCPUxU, ready.head->file_name, ready.head->AX, ready.head->BX,
              ready.head->CX, ready.head->DX, ready.head->PC, ready.head->IR);
    ready.head = ready.head->next;
  }

  // SECCIÓN TERMINADOS
  for (int i = 0; i < WIDTH_QUEUES; i++)
  {
    // ACS_HLINE: caracter de línea horizontal "bonita"
    mvwaddch(inner_queues, row, i, ACS_HLINE);
  }
  mvwprintw(inner_queues, row, (WIDTH_QUEUES - strlen("TERMINADOS")) / 2, "%s", "TERMINADOS");
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
  mvwprintw(inner_ginfo, 0, (WIDTH_GINFO / 3) - 25, "Usuarios:[%d]", NumUs);
  mvwprintw(inner_ginfo, 0, ((2 * WIDTH_GINFO) / 3) - 25, "MinP:[%d]", execution.head->P);
  mvwprintw(inner_ginfo, 0, WIDTH_GINFO - 25, "W:[%.2f]", W);
  // Se refresca la subventana de Información General
  wrefresh(inner_ginfo);
}

// Se imprime el procesador vacío
void empty_processor(WINDOW *inner_cpu)
{
  PCB empty_pcb = {0}; // Inicializa todos los campos del PCB a 0 o valores equivalentes
  print_processor(inner_cpu, empty_pcb);
}

// Se limpia la línea de prompt que se especifique
void clear_prompt(WINDOW *inner_prompt, int row)
{
  for (int i = 0; i < SIZE_BUFFER; i++)
  {
    mvwaddch(inner_prompt, row, PROMPT_START + i, ' ');
  }
}