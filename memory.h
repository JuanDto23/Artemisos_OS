#ifndef _MEMORY_H
#define _MEMORY_H

// Definiciones adelantadas de tipos
typedef struct queue Queue;
typedef struct pcb PCB;
typedef struct gui GUI;

// Parámetros de memoria
#define MAX_PAGES 4096
#define PAGE_SIZE 16
#define INSTRUCTION_SIZE 32
#define SWAP_SIZE 65536

// Se definen los offsets para el acceso a la memoria
#define PAGE_JUMP 0x200
#define INSTRUCTION_JUMP 0x20

// Definición de la tabla de memoria swap (TMS)
typedef struct tms
{
  int table[MAX_PAGES];
  int available_pages;
} TMS;

// Función para crear un archivo de memoria de intercambio (SWAP)
void create_swap(FILE **swap);

// Función para inicializar la tabla de memoria swap (TMS)
void initialize_tms(TMS *tms);

// Funciones para leer desde un archivo o desde la SWAP
void read_line_from_file(FILE *file, char *buffer);
void read_inst_from_swap(FILE *swap, char *line, PCB *execution_pcb);

// Función para cargar instrucciones en la SWAP y registrar en TMS los marcos ocupados por el proceso
void load_to_swap(PCB *new_process, TMS *tms, FILE **swap, int lines);

// Función para cargar un proceso a la cola de Listos
void load_to_ready(PCB *process, Queue *ready, TMS *tms, FILE **swap);

// Función para liberar las páginas ocupadas por un proceso en la TMS
void free_pages_from_tms(PCB *pcb_finished, TMS * tms);

// Función para actualizar las páginas ocupadas por un proceso hermano en la TMS
void update_pages_from_tms(PCB *brother_process, TMS * tms);
#endif