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
PCB *create_pcb(int *pid, char *file_name, FILE **program);
void enqueue(PCB *pcb, Queue *queue);
PCB *dequeue(Queue *queue);
void remove_pcb(PCB **pcb);
void print_queues(Queue execution, Queue ready, Queue finished);
PCB *search_pcb(int pid, Queue *queue);
void kill_queue(Queue *queue);
int search_file(char *file_name, Queue queue);
void free_queues(Queue *execution, Queue *ready, Queue *finished);

#endif
