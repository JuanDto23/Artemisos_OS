#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <limits.h> // Para usar el int_max
// Bibliotecas propias
#include "queue.h"
#include "pcb.h" // Se ocupa para la variable PBase

// Inicializa la cola
void initialize_queue(Queue *queue)
{
  queue->head = NULL;
  queue->elements = 0;
  queue->pid = 1;
}

// Crea un PCB y lo retorna
PCB *create_pcb(int *pid, char *file_name, FILE **program, int uid)
{
  PCB *pcb = NULL; // Nodo a retornar

  pcb = (PCB *)malloc(sizeof(PCB));
  if (pcb != NULL) // Hay memoria disponible
  {
    pcb->pid = *pid;
    pcb->AX = 0;
    pcb->BX = 0;
    pcb->CX = 0;
    pcb->DX = 0;
    pcb->PC = 0;
    strcpy(pcb->file_name, file_name); // No se puede asignar cadenas diréctamente
    pcb->program = *program;
    pcb->next = NULL;
    // Se inicializan los campos recién agregados
    pcb->UID = uid;
    pcb->P = PBase;
    pcb->KCPU = 0;
    pcb->KCPUxU = 0;
    // Actualizar el valor de la variable global W = 1/NumUs
    W = 1.0 / NumUs;
  }
  
  return pcb; // Se retorna el nodo creado
}

// Inserta un PCB en la cola
void enqueue(PCB *pcb, Queue *queue)
{
  PCB *current = NULL;     // PCB actual
  if (queue->head == NULL) // Si la cola está vacía se inserta el primer pcb
  {
    queue->head = pcb;
  }
  else // Si no, se inserta al final
  {
    current = queue->head;
    while (current->next != NULL) // Se recorre la cola hasta el final
    {
      current = current->next;
    }
    current->next = pcb;
  }
  (queue->elements)++; // Se incrementa el número de elementos
}

// Extrae un PCB de la cola
PCB *dequeue(Queue *queue)
{
  PCB *pcb = NULL; // PCB a retornar
  if (queue->head) // Si la cola no está vacía
  {
    pcb = queue->head;               // Se extrae el primer pcb
    queue->head = queue->head->next; // Se mueve la cabecera al siguiente
    pcb->next = NULL;                // Se desliga el nodo de la cola a extraer
    (queue->elements)--;             // Se decrementa el número de elementos
    return pcb;                      // Se retorna el pcb extraído
  }
  return NULL; // La cola está vacía
}

// Elimina un PCB
void remove_pcb(PCB **pcb)
{
  if (*pcb != NULL)
  {
    // Cerrar el archivo si está abierto
    if ((*pcb)->program != NULL)
    {
      fclose((*pcb)->program);
      (*pcb)->program = NULL; // Evitar punteros colgantes
    }

    // Liberar la memoria de la estructura
    free(*pcb);
    *pcb = NULL; // Evitar acceso a memoria liberada
  }
}

// Busca un PCB en la cola de acuerdo a su pid y lo extrae
PCB * extract_by_pid(int pid, Queue *queue) // Busca el pcb con el id especificado y lo regresa
{
  PCB *current = NULL; // Nodo actual de la cola
  PCB *before = NULL;  // Nodo anterior al actual
  int found = FALSE;   // Indica si se encontró el nodo

  // Se inicializan los nodos auxiliares
  current = queue->head;
  before = current;

  // Se busca el nodo con el pid especificado
  while (current && !found)
  {
    found = (current->pid == pid); // Válida si se busca el primer nodo
    if (!found)                    // Si no se ha encontrado el nodo avanza al siguiente
    {
      before = current;
      current = current->next;
    }
  }

  if (current) // Si se encontró el nodo
  {
    if (current == queue->head) // Si es el primer nodo
    {
      queue->head = current->next; // Se mueve la cabecera al segundo elemento
    }
    else // Si no es el primer nodo
    {
      before->next = current->next; // Se ligan los nodos del hueco
    }
    current->next = NULL; // Se desliga el nodo de la cola
    (queue->elements)--;  // Se decrementa el número de elementos
    return current;       // Se retorna el nodo encontrado
  }
  return NULL; // La cola está vacía
}

// Elimina la cola de procesos
void kill_queue(Queue *queue)
{
  while (queue->head)
  {
    PCB *temp = queue->head;
    queue->head = queue->head->next; // La cabecera almacena el nodo adelante
    remove_pcb(&temp);
  }
}

// Verifica si el usuario es dueño de un proceso en la cola pasada como parámetro
int search_uid(int uid, Queue queue)
{
  PCB *current = NULL;
  int found = FALSE;

  current = queue.head;
  while (current && !found)
  {
    // Se comprueba que el id usuario coincida con el dueño del nodo actual
    found = (current->UID == uid);
    if (!found)
    {
      // Se avanza al siguiente nodo de la lista
      current = current->next;
    }
  }
  if (current)
  {
    return TRUE; // Se encontró el usuario en la cola
  }
  return FALSE; // La cola está vacía o no se encontró el usuario
}

// Libera las colas de Ejecución, Listos y Terminados
void free_queues(Queue *execution, Queue *ready, Queue *finished)
{
  // Se liberan las colas de Ejecución y Listos
  kill_queue(execution);
  kill_queue(ready);
  /*
   * No se invoca kill_queue(finished) porque
   * la función kill_queue cierra los archivos
   * de los nodos, y en la cola Terminados, se
   * supone que el archivo de cada nodo ya se
   * encuentra cerrado.
   *
   * Por lo tanto, se recorre la cola Terminados
   * donde solo se hace uso de la función free
   * para liberar cada pcb. */
  while (finished->head)
  {
    PCB *temp = finished->head;
    finished->head = finished->head->next;
    free(temp);
  }
}

// Busca la menor prioridad entre todos los procesos de la cola
int get_minor_priority(Queue queue)
{
  PCB *aux = queue.head; // Nodo auxiliar para recorrer la cola

  // Si la cola está vacía, se regresa 0
  if (!aux)
  {
    return INT_MAX;  // Para que no pueda ser considerado como la menor prioridad
  }

  int min = aux->P; // Se considera la prioridad del primer nodo como la menor

  /* Se recorre la cola y va verificando si hay una menor prioridad
   que la que se consideró inicialmente */
  while (aux)
  {
    // Verifica si la prioridad del nodo actual es menor que la que se tiene almacenada
    if (aux -> P < min)
    {
      min = aux->P; // Se actualiza la menor prioridad hasta el momento
    }
    aux = aux->next; // Se avanza al siguiente nodo
  }

  return min; // Se retorna la menor proridad
}

// Busca un PCB en la cola de acuerdo a su prioridad y lo extrae
PCB * extract_by_priority(int priority, Queue *queue) // Busca el pcb con el id especificado y lo regresa
{
  PCB *current = NULL; // Nodo actual de la cola
  PCB *before = NULL;  // Nodo anterior al actual
  int found = FALSE;   // Indica si se encontró el nodo

  // Se inicializan los nodos auxiliares
  current = queue->head;
  before = current;

  // Se busca el nodo con la prioridad especificada
  while (current && !found)
  {
    found = (current->P == priority); // Válida si se busca el primer nodo
    if (!found)                       // Si no se ha encontrado el nodo avanza al siguiente
    {
      before = current;
      current = current->next;
    }
  }

  if (current) // Si se encontró el nodo
  {
    if (current == queue->head) // Si es el primer nodo
    {
      queue->head = current->next; // Se mueve la cabecera al segundo elemento
    }
    else // Si no es el primer nodo
    {
      before->next = current->next; // Se ligan los nodos del hueco
    }
    current->next = NULL; // Se desliga el nodo de la cola
    (queue->elements)--;  // Se decrementa el número de elementos
    return current;       // Se retorna el nodo encontrado
  }
  return NULL; // La cola está vacía, o se llegó al final sin encontrarlo
}

// Actualiza los contadores de uso del CPU para todos los procesos (no Terminados) del usuario dueño del proceso de la cola
void update_KCPUxU_per_process(int uid, Queue *queue)
{
  PCB *current = queue->head; // Nodo actual

  // Se actualiza KCPUxU de cada proceso de la cola que sea del usuario UID
  while (current)
  {
    // El proceso es del usuario UID
    if (current->UID == uid)
    {
      current->KCPUxU += IncCPU; // Se actualiza KCPUxU
    }
    current = current->next; // Se avanza al siguiente nodo
  }
}

// Actualiza los parámetros de planificación, para todos los nodos de la cola
void update_parameters(Queue *queue)
{
  PCB *current = queue->head; // Nodo actual

  // Se actualizan los parámetros de los nodos de la cola

  // 2 procesos de un usuario. W = 1
  while (current) {
    current -> KCPU /= 2;
    current -> KCPUxU /= 2;
    // Se deben actualizar todos al mismo tiempo???
    current -> P = PBase + (current -> KCPU)/2  + (current -> KCPUxU)/(4 * W);
    current = current->next; // Se avanza al siguiente nodo
  }
}