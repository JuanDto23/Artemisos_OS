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
	PCB *current = NULL; // PCB actual
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
PCB *dequeue(Queue * queue)
{
	PCB *pcb = NULL; // PCB a retornar
	if (queue->head) // Si la cola no está vacía
	{
		pcb = queue->head; // Se extrae el primer pcb
		queue->head = queue->head->next; // Se mueve la cabecera al siguiente
		pcb->next = NULL; // Se desliga el nodo de la cola a extraer
		(queue->elements)--; // Se decrementa el número de elementos
		return pcb; // Se retorna el pcb extraído
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

// Imprime la cola
void print_queue(Queue queue)
{
	PCB *aux = queue.head; // Nodo auxiliar para recorrer la cola
	int row = 30; // Renglón inicial de impresión de cola
	while (aux != NULL) // Se recorre la cola
	{
		mvprintw(row, 2, "PID:[%ud] FILE:[%s] AX:[%ld] BX:[%ld] CX:[%ld] DX:[%ld] PC:[%ud] IR:[%s]",
				 aux->pid, aux->file_name, aux->AX, aux->BX, aux->CX, aux->DX, aux->PC, aux->IR);
		aux = aux->next;
		row++;
	}
}

// Busca un PCB en la cola y lo extrae
PCB *search_pcb(int pid, Queue *queue) // Busca el pcb con el id especificado y lo regresa
{
	PCB *current = NULL; // Nodo actual de la cola
	PCB *before = NULL; // Nodo anterior al actual
	int found = FALSE; // Indica si se encontró el nodo

	// Se inicializan los nodos auxiliares
	current = queue->head;
	before = current;

	// Se busca el nodo con el pid especificado
	while (current && !found)
	{
		found = (current->pid == pid); // Válida si se busca el primer nodo
		if (!found) // Si no se ha encontrado el nodo avanza al siguiente 
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
		(queue->elements)--; // Se decrementa el número de elementos
		return current; // Se retorna el nodo encontrado
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