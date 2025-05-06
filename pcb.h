#ifndef _PCB_H
#define _PCB_H

// Declaración adelantada de Queue y GUI
typedef struct queue Queue;
typedef struct gui GUI;

// Valores numéricos obtenidos de kbwhat.c
#define ENTER 10
#define BACKSPACE 127
#define ESC 27

// Número de buffers, tamaño de buffer y tamaño de línea
#define NUMBER_BUFFERS 3
#define SIZE_BUFFER 66
#define SIZE_LINE 34
#define PROMPT_START 12

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
  // Nuevas variables para el Fair Share Schedule (equitativo)
  int UID;    // Id de usuario
  int P;      // Prioridad
  int KCPU;   // Uso de CPU por proceso
  int KCPUxU; // Uso de CPU por usuario
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
void print_history(char buffers[NUMBER_BUFFERS][SIZE_BUFFER], WINDOW *inner_prompt);

// FUNCIONES PRINCIPALES
int command_handling(GUI *gui, char buffers[NUMBER_BUFFERS][SIZE_BUFFER],
                     int *c, int *index, int *index_history,
                     Queue *execution, Queue *ready, Queue *finished,
                     unsigned *timer, unsigned *init_timer, int *speed_level);

int evaluate_command(GUI *gui, char *buffer, Queue *execution, Queue *ready, Queue *finished);

int read_line(FILE **program, char *line);
int interpret_instruction(GUI *gui, char *line, PCB *pcb);


#endif  