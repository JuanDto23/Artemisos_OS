#ifndef _QUEUE_H
#define _QUEUE_H

// Definiciones adelantadas de tipos
typedef struct pcb PCB;

// Estructura de cola con cabecera
typedef struct queue
{
  PCB *head;             // Puntero a pcb, donde se ligan todos los nodos de la cola
  unsigned int elements; // Contador de nodos de la cola
} Queue;

// Función de inicialización de la cola
void initialize_queue(Queue *queue);

// Función para crear un PCB
PCB *create_pcb(int pid, char *file_name, FILE **program, int iud, int TmpSize, int lines);

// Funciones de manipulación de la cola con política FIFO
void enqueue(PCB *pcb, Queue *queue);
PCB *dequeue(Queue *queue);

// Funciones de liberación de memoria
void remove_pcb(PCB **pcb);
void kill_queue(Queue *queue);
void free_queues(Queue *execution, Queue *ready, Queue *finished, Queue *new);

// Funciones de extracción de nodos de la cola
PCB *extract_by_pid(int pid, Queue *queue);
PCB *extract_brother_process(int uid, char *filename, Queue *queue);
PCB *extract_by_priority(int priority, Queue *queue);

// Funciones de búsqueda de nodos en la cola
int is_user_in_queue(int uid, Queue queue);
PCB *search_process_fits_swap(Queue *new, int avalible_pages);
PCB *search_brother_process(int uid, char *filename, Queue queue);
int get_minor_priority(Queue queue);
int get_KCPUxU(int uid, Queue queue);
bool update_tmp_after_eviction(int clock, int pid, Queue *queue);

#endif