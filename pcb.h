#ifndef _PCB_H
#define _PCB_H

// Valores numéricos obtenidos de kbwhat.c
#define ENTER 10
#define BACKSPACE 127
#define ESC 27
#define SPACE 32
#define UNDERSCORE 95

// Número de buffers, tamaño de buffer y tamaño de línea
#define NUMBER_BUFFERS 6
#define SIZE_BUFFER 256
#define SIZE_LINE 32

/* Delay de N milisegundos que se detendrá
* la ejecución del programa entre cada iteración */
#define N 300

// Valores booleanos
#define TRUE 1
#define FALSE 0

// Creación del bloque de control de procesos
typedef struct pcb
{
   int pid;
   int AX;
   int BX;
   int CX;
   int DX;
   int PC;
   char IR[100];
	FILE *program;
	char file_name[256];
	struct pcb *next;
}PCB;

/*-------------------PROTOTIPOS -------------------*/

// FUNCIONES DE INICIALIZACIÓN
void initialize_pcb(PCB * pcb);
void initialize_buffer(char * buffer, int * index);

// FUNCIONES AUXILIARES
void str_upper(char * str);
int is_numeric(char *str);
void clear_prompt(int row);
void clear_messages(void);
void clear_queue(void);
int search_register(char *p);            
int value_register(PCB *pcb, char r);

// FUNCIONES PRINCIPALES
int command_handling(char buffers[NUMBER_BUFFERS][SIZE_BUFFER],
   int *read_flag, int *c, int *index, FILE **f, int *index_history, 
   int *pid, int *processor_busy, PCB **detached_pcb);

int evaluate_command(char *buff, int *index, int *read_flag, 
   FILE **f, int *pid, int *processor_busy, PCB **detached_pcb);
               
int read_line(FILE **program, char *line);
int interpret_instruction(char * line, PCB *pcb);
void processor_template(void);
void messages_template(void); 
void queue_template(void);
void print_history(char buffers[NUMBER_BUFFERS][SIZE_BUFFER]);
void print_registers(PCB *pcb);

#endif