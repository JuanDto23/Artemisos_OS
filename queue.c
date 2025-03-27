#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include "queue.h"

// Inicializa la cola
void initialize_queue(Queue *queue)
{
  queue->head = NULL;
  queue->elements = 0;
  queue->pid = 1;
}

// Crea un PCB y lo retorna
PCB *create_pcb(int *pid, char *file_name, FILE **program)
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
  }
  else
  {
    mvprintw(14, 4, "Error: PCB no pudo ser creado.");
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

// Imprime las colas de procesos
void print_queues(Queue execution, Queue ready, Queue finished)
{
  int row = 6;
  int col = 86;
  int i = 0;

  // Se imprime la plantilla de las colas
  mvprintw(row, col, "------------------------------EJECUCION----------------------------------          ");
  for (row = 7, i = 0; execution.head != NULL; i++, row++)
  {
    mvprintw(row, col, "PID:[%u] FILE:[%s] AX:[%ld] BX:[%ld] CX:[%ld] DX:[%ld] PC:[%u] IR:[%s]             ",
             execution.head->pid, execution.head->file_name, execution.head->AX, execution.head->BX,
             execution.head->CX, execution.head->DX, execution.head->PC, execution.head->IR);
    execution.head = execution.head->next;
  }
  mvprintw(row, col, "--------------------------------LISTOS-----------------------------------          ");
  for (i = 0, row = row + 1; ready.head != NULL; i++, row++)
  {
    mvprintw(row, col, "PID:[%u] FILE:[%s] AX:[%ld] BX:[%ld] CX:[%ld] DX:[%ld] PC:[%u] IR:[%s]             ",
             ready.head->pid, ready.head->file_name, ready.head->AX, ready.head->BX,
             ready.head->CX, ready.head->DX, ready.head->PC, ready.head->IR);
    ready.head = ready.head->next;
  }
  mvprintw(row, col, "-----------------------------TERMINADOS----------------------------------          ");
  for (i = 0, row = row + 1; finished.head != NULL; i++, row++)
  {
    mvprintw(row, col, "PID:[%u] FILE:[%s] AX:[%ld] BX:[%ld] CX:[%ld] DX:[%ld] PC:[%u] IR:[%s]             ",
             finished.head->pid, finished.head->file_name, finished.head->AX, finished.head->BX,
             finished.head->CX, finished.head->DX, finished.head->PC, finished.head->IR);
    finished.head = finished.head->next;
  }
  // Se actualiza la pantalla
  refresh();
}

// Busca un PCB en la cola y lo extrae
PCB *search_pcb(int pid, Queue *queue) // Busca el pcb con el id especificado y lo regresa
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

// Verifica si el archivo se encuentra en la cola pasada como parámetro
int search_file(char *file_name, Queue queue)
{
  PCB *current = NULL;
  int found = FALSE;

  current = queue.head;
  while (current && !found)
  {
    // Se comprueba que el archivo coincida con el archivo del nodo actual
    found = !(strcmp(current->file_name, file_name));
    if (!found)
    {
      // Se avanza al siguiente nodo de la lista
      current = current->next;
    }
  }
  if (current)
  {
    return TRUE; // Se encontró el archivo en la cola
  }
  return FALSE; // La cola está vacía o no se encontró el archivo
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
  while (finished->head) {
    PCB *temp = finished->head;
    finished->head = finished->head->next;
    free(temp);
  }
}
