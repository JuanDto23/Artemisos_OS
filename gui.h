#ifndef _GUI_H
#define _GUI_H

// Declaración adelantada de PCB y Queue
typedef struct pcb PCB;
typedef struct queue Queue;

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
#define HEIGHT_QUEUES 31

// INFORMACIÓN GENERAL
#define STARTX_GINFO 86
#define STARTY_GINFO 0
#define WIDTH_GINFO 120
#define HEIGHT_GINFO 3

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
}GUI;

void initialize_gui(GUI *gui);
void print_prompt(WINDOW *inner_prompt, int row);
void print_processor(WINDOW *inner_cpu, PCB pcb);
void print_queues(WINDOW *inner_queues, Queue execution, Queue ready, Queue finished);
void print_ginfo(WINDOW *inner_ginfo, Queue execution);
void empty_processor(WINDOW *inner_cpu);
void clear_prompt(WINDOW *inner_prompt, int row);

#endif