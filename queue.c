#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <limits.h>
#include <stdbool.h>
#include <unistd.h>
// Bibliotecas propias
#include "queue.h"
#include "pcb.h" // Se ocupa para la variable PBase

// Inicializa la cola
void initialize_queue(Queue *queue)
{
  queue->head = NULL;
  queue->elements = 0;
}

// Crea un PCB y lo retorna
PCB *create_pcb(int pid, char *file_name, FILE **program, int uid, int TmpSize, int lines)
{
  PCB *pcb = NULL; // Nodo a retornar

  pcb = (PCB *)malloc(sizeof(PCB));
  if (pcb != NULL) // Hay memoria disponible
  {
    pcb->pid = pid;
    pcb->AX = 0;
    pcb->BX = 0;
    pcb->CX = 0;
    pcb->DX = 0;
    pcb->PC = 0;
    strcpy(pcb->file_name, file_name); // No se puede asignar cadenas diréctamente
    pcb->program = *program;
    pcb->next = NULL;

    // Nuevas variables para el Fair Share Schedule (equitativo)
    pcb->UID = uid;
    pcb->P = PBase;
    pcb->KCPU = 0;
    pcb->KCPUxU = 0;

    // Inicialización de parámetros de memoria
    pcb->TmpSize = TmpSize;
    pcb->tmp.inSWAP = NULL;
    pcb->lines = lines;
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

// Elimina la cola de procesos sin cerrar archivos
void kill_queue(Queue *queue)
{
  while (queue->head)
  {
    PCB *temp = queue->head;
    queue->head = queue->head->next; // La cabecera almacena el nodo adelante
    free(temp);
  }
}

// Libera las colas de Ejecución, Listos y Terminados
void free_queues(Queue *execution, Queue *ready, Queue *finished, Queue *new)
{
  // Se liberan las colas de Ejecución y Listos
  kill_queue(execution);
  kill_queue(ready);
  kill_queue(finished);

  // Los procesos en Nuevos necesitan cerrar sus archivos también
  while (new->head)
  {
    PCB *temp = new->head;
    new->head = new->head->next;
    remove_pcb(&temp);
  }
}

// Busca un PCB en la cola de acuerdo a su pid y lo extrae
PCB *extract_by_pid(int pid, Queue *queue) // Busca el pcb con el id especificado y lo regresa
{
  PCB *current = NULL; // Nodo actual de la cola
  PCB *before = NULL;  // Nodo anterior al actual
  int found = false;   // Indica si se encontró el nodo

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

// Extrae un proceso hermano de la cola de acuerdo a su UID y nombre de archivo
PCB *extract_brother_process(int uid, char *filename, Queue *queue)
{
  PCB *current = NULL;
  int found = false;

  current = queue->head;
  while (current && !found)
  {
    // Se comprueba que el archivo y el id usuario coincidan con el dueño del nodo actual
    found = !strcmp(current->file_name, filename) && (current->UID == uid);
    if (!found)
    {
      // Se avanza al siguiente nodo de la lista
      current = current->next;
    }
  }
  if (current)
  {
    return extract_by_pid(current->pid, queue); // Se encontró el nodo hermano en la cola
  }
  return NULL; // La cola está vacía o no se encontró otro proceso hermano
}

// Busca un PCB en la cola de acuerdo a su prioridad y lo extrae
PCB *extract_by_priority(int priority, Queue *queue) // Busca el pcb con el id especificado y lo regresa
{
  PCB *current = NULL; // Nodo actual de la cola
  PCB *before = NULL;  // Nodo anterior al actual
  int found = false;   // Indica si se encontró el nodo

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

// Verifica si el usuario con uid tiene algún proceso en la cola
int is_user_in_queue(int uid, Queue queue)
{
  PCB *current = NULL;
  int found = false;

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
    return true; // Se encontró el usuario en la cola
  }
  return false; // La cola está vacía o no se encontró el usuario
}

// Busca el primer proceso que encaje en la SWAP y extrae el nodo
PCB *search_process_fits_swap(Queue *new, int available_pages)
{
  PCB *current = NULL;
  int found = false;

  current = new->head;
  while (current && !found)
  {
    // Se comprueba que el proceso quepa en la SWAP
    found = (current->TmpSize <= available_pages);
    if (!found)
    {
      // Se avanza al siguiente nodo de la lista
      current = current->next;
    }
  }
  if (current)
  {
    return extract_by_pid(current->pid, new); // Se encontró el proceso que cabe en la SWAP
  }
  return NULL; // La cola está vacía o no se encontró ningún proceso a cargar desde nuevos
}

// Busca un proceso hermano en la cola de acuerdo a su UID y nombre de archivo y regresa el nodo sin extraerlo
PCB *search_brother_process(int uid, char *filename, Queue queue)
{
  PCB *current = NULL;
  int found = FALSE;

  current = queue.head;
  while (current && !found)
  {
    // Se comprueba que el archivo y el id usuario coincidan con el dueño del nodo actual
    found = !strcmp(current->file_name, filename) && (current->UID == uid);
    if (!found)
    {
      // Se avanza al siguiente nodo de la lista
      current = current->next;
    }
  }
  if (current)
  {
    return current; // Se encontró el nodo hermano en la cola
  }
  return NULL; // La cola está vacía o no se encontró otro proceso hermano
}

// Busca la menor prioridad entre todos los procesos de la cola
int get_minor_priority(Queue queue)
{
  PCB *aux = queue.head; // Nodo auxiliar para recorrer la cola

  // Si la cola está vacía, se regresa 0
  if (!aux)
  {
    return 0; // Para que no pueda ser considerado como la menor prioridad
  }

  int min = aux->P; // Se considera la prioridad del primer nodo como la menor

  /* Se recorre la cola y va verificando si hay una menor prioridad
   que la que se consideró inicialmente */
  while (aux)
  {
    // Verifica si la prioridad del nodo actual es menor que la que se tiene almacenada
    if (aux->P < min)
    {
      min = aux->P; // Se actualiza la menor prioridad hasta el momento
    }
    aux = aux->next; // Se avanza al siguiente nodo
  }

  return min; // Se retorna la menor proridad
}

// Obtiene el KCPUxU del primer proceso del usuario especificado que encuentre
int get_KCPUxU(int uid, Queue queue)
{
  PCB *current = NULL;
  int found = false;

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
    return current->KCPUxU; // Se encontró el usuario en la cola
  }
  return -1; // La cola está vacía o no se encontró el usuario
}

// Actualiza el TMP de un proceso después de que se ha desalojado de la RAM
bool update_tmp_after_eviction(int clock, int pid, Queue *queue)
{
  PCB *current = NULL;
  int found = false;

  current = queue->head;
  while (current && !found)
  {
    // Se comprueba que el id usuario coincida con el dueño del nodo actual
    found = (current->pid == pid);
    if (!found)
    {
      // Se avanza al siguiente nodo de la lista
      current = current->next;
    }
  }
  if (current)
  {
    /* Buscar el proceso que estaba ocupando ese marco en RAM e indicar en su TMP,
       que ese marco ya no se encuentra cargado en RAM (desalojado). */
    for (int i = 0; i < current->TmpSize; i++)
    {
      if (current->tmp.inRAM[i] == clock)
      {
        current->tmp.ram_presence[i] = 0; // Presencia = 0
        current->tmp.inRAM[i] = -1;       // Marco en RAM = -1
        return true;
      }
    }
  }
  return false;
}