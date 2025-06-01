#ifndef _PCB_H
#define _PCB_H

#include "memory.h" // Para INSTRUCTION_SIZE

// Definiciones adelantadas de tipos
typedef struct queue Queue;
typedef struct gui GUI;

/* Sirve de referencia para indicar la cantidad
   máxima de instrucciones consecutivas que se
   podrán ejecutar por cada proceso */
#define MAXQUANTUM 5

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

// Funciones de actualización de parámetros de planificación
void update_KCPUxU_per_process(int uid, Queue *queue);
void update_parameters(Queue *queue);
void update_users(int uid, Queue queue);

// Funciones para la interpretación de instrucciones leídas de un proceso
int search_register(char *p);
int value_register(PCB *pcb, char r);
int interpret_instruction(GUI *gui, char *line, PCB *pcb);

// Función para la gestión de un proceso cuando termina
void handle_process_termination(GUI *gui, PCB *current_process, Queue *execution, Queue *ready,
                                Queue *new, TMS *tms, int tms_disp, FILE **swap);

// Función para recalcular prioridades
void recalculate_priorities(GUI *gui, Queue ready, int *minor_priority);
#endif