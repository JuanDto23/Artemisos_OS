#ifndef _PCB_H
#define _PCB_H

// Declaración adelantada de Queue
typedef struct queue Queue;

// Valores numéricos obtenidos de kbwhat.c
#define ENTER 10
#define BACKSPACE 127
#define ESC 27

// Número de buffers, tamaño de buffer y tamaño de línea
#define NUMBER_BUFFERS 5
#define SIZE_BUFFER 71
#define SIZE_LINE 32

// Valores booleanos
#define TRUE 1
#define FALSE 0

// Intervalo de tiempo antes de la siguiente instrucción
#define MAX_TIME 100000

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
  char IR[100];
  FILE *program;
  char file_name[256];
  struct pcb *next;
} PCB;

#include "queue.h" // Se incluye para saber el tipo de dato Queue

/*-------------------PROTOTIPOS -------------------*/

// FUNCIONES DE INICIALIZACIÓN
void initialize_buffer(char *buffer, int *index);

// FUNCIONES AUXILIARES
void str_upper(char *str);
int is_numeric(char *str);
void clear_prompt(int row);
void clear_messages(void);
int search_register(char *p);
int value_register(PCB *pcb, char r);
void loaded_programs_area(int file_counter);

// FUNCIONES PRINCIPALES
int command_handling(char buffers[NUMBER_BUFFERS][SIZE_BUFFER],
                     int *c, int *index, int *index_history,
                     Queue *execution, Queue *ready, Queue *finished,
                     unsigned *timer, unsigned *init_timer, int *file_counter, int *speed_level);

int evaluate_command(char *buffer, Queue *execution, Queue *ready, Queue *finished,
                     int *file_counter);

int read_line(FILE **program, char *line);
int interpret_instruction(char *line, PCB *pcb);
void processor_template(void);
void messages_template(void);
void print_history(char buffers[NUMBER_BUFFERS][SIZE_BUFFER]);
void print_registers(PCB pcb);

#endif
