#ifndef _QUEUE_H
#define _QUEUE_H

#include "pcb.h" // Se incluye para saber el tipo de dato PCB

typedef struct queue
{
   PCB *head;             // Puntero a pcb, donde se ligan todos los nodos de la cola
   unsigned int elements; // Contador de nodos de la cola
   int pid;               // identifica cada nodo de forma única
} Queue;

void initialize_queue(Queue *queue);
PCB *create_pcb(int *, char *, FILE **);
void enqueue(PCB *, Queue *);
PCB *dequeue(Queue *);   // Desliga el nodo pcb de la cola y lo retorna
void remove_pcb(PCB **); // Libera la memoria del nodo
void print_queue(Queue);
PCB *search_pcb(int, Queue *queue);
void kill_queue(Queue *queue);

#endif