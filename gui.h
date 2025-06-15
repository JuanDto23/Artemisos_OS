#ifndef _GUI_H
#define _GUI_H

#include "prompt.h" // Para las macros HISTORY_SIZE y PROMPT_SIZE

// Definiciones adelantadas de tipos
typedef struct pcb PCB;
typedef struct queue Queue;
typedef struct tms TMS;
typedef struct tmm TMM;

// ============= MACROS =============

// PROMT
#define STARTX_PROMPT 2
#define STARTY_PROMPT 0
#define WIDTH_PROMPT 81 
#define HEIGHT_PROMPT 5

// PROCESADOR
#define STARTX_CPU 2
#define STARTY_CPU 5
#define WIDTH_CPU 81
#define HEIGHT_CPU 8    

// MENSAJES
#define STARTX_MSG 2
#define STARTY_MSG 13
#define WIDTH_MSG 81
#define HEIGHT_MSG 6

// COLAS
#define STARTX_QUEUES 86
#define STARTY_QUEUES 3
#define WIDTH_QUEUES 120
#define HEIGHT_QUEUES 16

// INFORMACIÓN GENERAL
#define STARTX_GINFO 86
#define STARTY_GINFO 0
#define WIDTH_GINFO 120
#define HEIGHT_GINFO 3

// SWAP
#define STARTX_SWAP 86
#define STARTY_SWAP 19
#define WIDTH_SWAP 120
#define HEIGHT_SWAP 19

//RAM
#define STARTX_RAM 28
#define STARTY_RAM 19
#define WIDTH_RAM 55
#define HEIGHT_RAM 19

// TMS
#define STARTX_TMS 2
#define STARTY_TMS 19
#define WIDTH_TMS 12
#define HEIGHT_TMS 19

// TMM
#define STARTX_TMM 15
#define STARTY_TMM 19
#define WIDTH_TMM 12
#define HEIGHT_TMM 19

// TMP
#define STARTX_TMP (2 * WIDTH_CPU / 3 + 6)
#define STARTY_TMP 5
#define WIDTH_TMP 23
#define HEIGHT_TMP 8
 
// KEYS
#define STARTX_KEYS 2
#define STARTY_KEYS 38
#define WIDTH_KEYS 123
#define HEIGHT_KEYS 3

// Parámetros de desplazamientos
#define TOTAL_DISP_SWAP 682 
#define TOTAL_DISP_TMS 255  
#define TOTAL_DISP_RAM 7 
#define DISPLAYED_PAGES_SWAP 6 // Cantidad de páginas que se muestran en la SWAP
#define PAGES_RAM 2 // Cantidad de páginas que se muestran en la RAM
#define DISPLAYED_PAGES_TMS 16 // Cantidad de páginas que se muestran en la TMS
#define DISPLAYED_ADRESSES_TMP HEIGHT_TMP - 3 // Cantidad de direcciones que se muestran en la TMP

// Estructura de la GUI. «inner» significa que es una subventana
typedef struct gui
{
  WINDOW *prompt;
  WINDOW *inner_prompt;
  WINDOW *cpu;
  WINDOW *inner_cpu;
  WINDOW *msg;
  WINDOW *inner_msg;
  WINDOW *queues;
  WINDOW *inner_queues;
  WINDOW *ginfo;
  WINDOW *inner_ginfo;
  WINDOW *swap;
  WINDOW *inner_swap;
  WINDOW *ram;
  WINDOW *inner_ram;
  WINDOW *tms;
  WINDOW *inner_tms;
  WINDOW *tmm;
  WINDOW *inner_tmm;
  WINDOW *tmp;
  WINDOW *inner_tmp;
  WINDOW *keys;
  WINDOW *inner_keys;
}GUI;

// Función para inicializar la GUI
void initialize_gui(GUI *gui);

// Funciones para imprimir contenido en las subventanas de la GUI
void print_prompt(WINDOW *inner_prompt, int row);
void print_processor(WINDOW *inner_cpu, PCB *pcb);
void print_queues(WINDOW *inner_queues, Queue execution, Queue ready, Queue new, Queue finished, int lists_disp);
void print_ginfo(WINDOW *inner_ginfo, Queue execution);
void print_swap(WINDOW *inner_swap, FILE *swap, int swap_disp);
void print_ram(WINDOW *inner_ram, int ram_disp);
void print_tms(WINDOW *inner_tms, TMS tms, int tms_disp);
void print_tmp(WINDOW *inner_tmp, PCB *pcb, int tmp_disp);
void print_tmm(WINDOW *inner_tmm, TMM tmm, int clock);

void print_history(char buffers[HISTORY_SIZE][PROMPT_SIZE], WINDOW *inner_prompt);

// Función para imprimir procesador vacío
void empty_processor(WINDOW *inner_cpu);

// Función para limpiar una línea especifica de la prompt
void clear_prompt(WINDOW *inner_prompt, int row);

// Función para imprimir la traducción de la dirección virtual (PC), por la dirección real en SWAP
void print_traduction(WINDOW *inner_msg, PCB *current_process);
#endif