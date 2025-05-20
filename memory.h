#ifndef _MEMORY_H
#define _MEMORY_H

// Parámetros de memoria
#define MAX_PAGES 4096
#define PAGE_SIZE 16
#define INSTRUCTION_SIZE 32
#define TOTAL_INSTRUCTIONS 65536

typedef struct tms {
  int table[MAX_PAGES];
  int available_pages;
}TMS;

FILE *create_swap(void);
void get_available_pages(TMS * tms);

#endif