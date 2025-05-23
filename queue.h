#ifndef _QUEUE_H
#define _QUEUE_H

// Declaración adelantada de tipo PCB 
typedef struct pcb PCB;

// Estructura de cola con cabecera
typedef struct queue
{
  PCB *head;             // Puntero a pcb, donde se ligan todos los nodos de la cola
  unsigned int elements; // Contador de nodos de la cola
  int pid;               // identifica cada nodo de forma única
} Queue;

void initialize_queue(Queue *queue);
PCB *create_pcb(int *pid, char *file_name, FILE **program, int iud);
void enqueue(PCB *pcb, Queue *queue);
PCB *dequeue(Queue *queue);
void remove_pcb(PCB **pcb);
PCB *extract_by_pid(int pid, Queue *queue);
void kill_queue(Queue *queue);
int search_uid(int uid, Queue queue);
void free_queues(Queue *execution, Queue *ready, Queue *finished);
int get_minor_priority(Queue queue);  
PCB *extract_by_priority(int priority, Queue * queue);
void update_KCPUxU_per_process(int uid, Queue *queue);
void update_parameters(Queue *queue);
#endif
