#ifndef _GUI_H
#define _GUI_H

// Declaración adelantada de PCB y Queue
typedef struct pcb PCB;
typedef struct queue Queue;
typedef struct tms TMS;

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

// TMS
#define STARTX_TMS 2
#define STARTY_TMS 19
#define WIDTH_TMS 12
#define HEIGHT_TMS 19

// TMP
#define STARTX_TMP 16
#define STARTY_TMP 19
#define WIDTH_TMP 16
#define HEIGHT_TMP 8

// KEYS
#define STARTX_KEYS 2
#define STARTY_KEYS 38
#define WIDTH_KEYS 126
#define HEIGHT_KEYS 3

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
  WINDOW *tms;
  WINDOW *inner_tms;
  WINDOW *tmp;
  WINDOW *inner_tmp;
  WINDOW *keys;
  WINDOW *inner_keys;
}GUI;

void initialize_gui(GUI *gui);
void print_prompt(WINDOW *inner_prompt, int row);
void print_processor(WINDOW *inner_cpu, PCB pcb);
void print_queues(WINDOW *inner_queues, Queue execution, Queue ready, Queue finished, Queue new);
void print_ginfo(WINDOW *inner_ginfo, Queue execution);
void print_swap(WINDOW *inner_swap, FILE *swap, int swap_disp);
void print_tms(WINDOW *inner_tms, TMS tms, int tms_disp);
void print_tmp(WINDOW *inner_tmp, PCB pcb, int tmp_disp);
void empty_processor(WINDOW *inner_cpu);
void clear_prompt(WINDOW *inner_prompt, int row);

#endif