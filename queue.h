#ifndef _QUEUE_H
#define _QUEUE_H

#include "pcb.h" // Se incluye para saber el tipo de dato PCB (struct pcb)

PCB *create_pcb(int *, char *, FILE **);
void enqueue(PCB *);
PCB *dequeue(void);     // Desliga el nodo pcb de la cola y lo retorna
void remove_pcb(PCB **); // Libera la memoria del nodo
void print_queue(void);
PCB *search_pcb(int pid);
void kill_queue(void);

#endif