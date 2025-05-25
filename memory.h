#ifndef _MEMORY_H
#define _MEMORY_H

// Definición adelantada del PCB
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
typedef struct tms {
  int table[MAX_PAGES];
  int available_pages;
}TMS;

FILE *create_swap(void);
void initialize_tms(TMS *tms);
void get_available_pages(TMS * tms);
void read_line_from_file(FILE *file, char *buffer);
void load_to_swap(PCB * new_pcb, TMS *tms, FILE **swap, int lines, GUI *gui);


#endif