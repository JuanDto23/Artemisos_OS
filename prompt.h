#ifndef _PROMPT_H
#define _PROMPT_H

// Declaraciones adelantadas de tipos
typedef struct queue Queue;
typedef struct gui GUI;
typedef struct tms TMS;
typedef struct pcb PCB;

// Valores numéricos obtenidos de kbwhat.c
#define ENTER 10
#define ESC 27

// Tmaño de historial, tamaño de prompt e inicio de prompt
#define HISTORY_SIZE 3
#define PROMPT_SIZE 66
#define PROMPT_START 12

// Intervalo de tiempo antes de la siguiente instrucción
#define MAX_TIME 500000

// Número máximo de niveles
#define MAX_LEVEL 15

// Función de inicialización de buffer de prompt
void initialize_buffer(char *buffer, int *index);

// Función para crear historial
void create_history(GUI *gui, char buffers[HISTORY_SIZE][PROMPT_SIZE], int *index, int *index_history);

// Función del gestor de comandos de la prompt
void command_handling(GUI *gui, int *exited, char buffers[HISTORY_SIZE][PROMPT_SIZE],
                     int *index, int *index_history,
                     Queue *execution, Queue *ready, Queue *finished, Queue *new,
                     unsigned *timer, unsigned *init_timer, int *speed_level, TMS * tms, FILE ** swap,
                     int *swap_disp, int *tms_disp, int * tmp_disp,  int * lists_disp, int *ram_disp,  PCB *execution_pcb);

// Funciones para la interpretación de comando de la prompt
void load_command(char *parameter1, char *parameter2, Queue *execution, Queue *ready, Queue *new, Queue *finished,
                  TMS *tms, FILE **swap, int tms_disp, int swap_disp, int lists_disp, GUI *gui);
void kill_command(char *parameter1, Queue *execution, Queue *ready, Queue *new, Queue *finished,
                  TMS *tms, FILE **swap, int tms_disp, int lists_disp, GUI *gui);
void exit_command(int *exited, GUI *gui, Queue *execution, Queue *ready, Queue *new, Queue *finished);
int confirm_exit(GUI *gui);
void evaluate_command(GUI *gui, int *exited, char *buffer, Queue *execution, Queue *ready, Queue *finished, Queue *new,
                     TMS *tms, FILE **swap, int *swap_disp, int *tms_disp, int *tmp_disp, int *lists_disp,int *ram_disp);

#endif