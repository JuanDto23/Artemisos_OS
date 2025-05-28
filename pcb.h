#ifndef _PCB_H
#define _PCB_H

#include "memory.h"

// Declaración adelantada de Queue y GUI
typedef struct queue Queue;
typedef struct gui GUI;

// Valores numéricos obtenidos de kbwhat.c
#define ENTER 10
#define ESC 27

// Número de buffers, tamaño de buffer y tamaño de línea
#define NUMBER_BUFFERS 3
#define BUFFER_SIZE 66
#define PROMPT_START 12

// Valores booleanos
#define TRUE 1
#define FALSE 0

// Intervalo de tiempo antes de la siguiente instrucción
#define MAX_TIME 500000

// Número máximo de niveles
#define MAX_LEVEL 15

/* Sirve de referencia para indicar la cantidad
   máxima de instrucciones consecutivas que se
   podrán ejecutar por cada proceso */
#define MAXQUANTUM 4

// Creación del bloque de control de procesos
typedef struct pcb
{
  unsigned pid;
  long AX;
  long BX;
  long CX;
  long DX;
  unsigned PC;
  char IR[INSTRUCTION_SIZE];
  FILE *program;
  char file_name[256];
  struct pcb *next;

  // Nuevas variables para el Fair Share Schedule (equitativo)
  int UID;    // Id de usuario
  int P;      // Prioridad
  int KCPU;   // Uso de CPU por proceso
  int KCPUxU; // Uso de CPU por usuario

  // Parámetros de la memoria
  int TmpSize; // Tamaño de la TMP (cantidad de marcos del proceso)
  int *TMP;    // Tabla de mapa/marcos de proceso
  int lines;   // Cantidad de líneas del programa del proceso
} PCB;

// Se declaran las variables usadas en pcb.c, pero no se reserva memoria
extern int NumUs;
extern int IncCPU;
extern double W;
extern const int PBase;

/*-------------------PROTOTIPOS -------------------*/

// FUNCIONES DE INICIALIZACIÓN
void initialize_buffer(char *buffer, int *index);

// FUNCIONES AUXILIARES
void str_upper(char *str);
int is_numeric(char *str);
int search_register(char *p);
int value_register(PCB *pcb, char r);
int end_simulation(WINDOW *inner_msg);
void print_history(char buffers[NUMBER_BUFFERS][BUFFER_SIZE], WINDOW *inner_prompt);
int count_lines(FILE *file);

// FUNCIONES PRINCIPALES
int command_handling(GUI *gui, char buffers[NUMBER_BUFFERS][BUFFER_SIZE],
                     int *c, int *index, int *index_history,
                     Queue *execution, Queue *ready, Queue *finished, Queue *new,
                     unsigned *timer, unsigned *init_timer, int *speed_level, TMS * tms, FILE ** swap,
                     int *swap_disp, int *tms_disp, int * tmp_disp, PCB *execution_pcb);

int evaluate_command(GUI *gui, char *buffer, Queue *execution, Queue *ready, Queue *finished, Queue *new,
                     TMS * tms, FILE **swap, int *swap_disp, int *tms_disp, int *tmp_disp);

void read_line_from_swap(FILE *swap, char *line, PCB *execution_pcb);
int interpret_instruction(GUI *gui, char *line, PCB *pcb);
#endif