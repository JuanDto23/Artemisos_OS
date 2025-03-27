#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include "queue.h"

PCB *queue = NULL;  // Se declara la cola vacía
int pcb_count;	//Variable auxiliar para contabilizar el número de pcbs

PCB *create_pcb(int *pid, char *file_name, FILE **program)
{
	PCB *pcb = NULL; // Nodo a retornar
	
	pcb = (PCB *) malloc(sizeof(PCB));
	if (pcb != NULL) { // Hay memoria disponible
		pcb -> pid = *pid;
		pcb -> AX = 0;
		pcb -> BX = 0;
		pcb -> CX = 0;
		pcb -> DX = 0;
		strcpy(pcb->file_name, file_name);	//No se puede asignar cadenas diréctamente
		pcb -> program = *program;
		pcb -> next = NULL;
	}
	else {
		mvprintw(14,4,"Error: PCB no pudo ser creado.");
	}
	return pcb;	
} 

void enqueue(PCB *pcb)
{
	PCB *current; // PCB actual
	if (queue == NULL) {
		queue = pcb;
	}
	else {
		current = queue;
		while (current -> next != NULL) {
			current = current -> next;
		}	// Se encontró el pcb final
		current -> next= pcb;	
	}
	pcb_count++;
}

PCB *dequeue(void)
{
	PCB *pcb = NULL;
	
	if (queue != NULL) {
		pcb = queue;
		queue = queue -> next;
		pcb -> next = NULL;
		pcb_count--;
		return pcb;
	}
	return NULL;
}

void remove_pcb(PCB **pcb)
{
	if (*pcb != NULL) {
        // Cerrar el archivo si está abierto
        if ((*pcb)->program != NULL) {
            fclose((*pcb)->program);
            (*pcb)->program = NULL;  // Evitar punteros colgantes
        }

        // Liberar la memoria de la estructura
        free(*pcb);
        *pcb = NULL;  // Evitar acceso a memoria liberada
    }
}
	
void print_queue(void)
{
	PCB *aux = queue;
	int row = 21; // Renglón inicial de impresión de cola
	while (aux != NULL) {
		mvprintw(row,2,"PID:[%d] FILE:[%s] AX:[%d] BX:[%d] CX:[%d] DX:[%d] PC:[%d] IR:[%s]", 
			aux -> pid, aux->file_name, aux->AX, aux->BX, aux->CX, aux->DX, aux->PC, aux->IR);
		aux = aux -> next;
		row++;
	}
}

PCB *search_pcb(int pid)
{
	PCB * current = NULL;
	PCB * before = NULL;
	int found = FALSE;
	
	current = queue;
	before = current;
	while (current && !found) {
		found = (current -> pid == pid); // Válida si se busca el primer nodo
		if (!found) {
			before = current;
			current = current -> next;
		}
	}
	if (current) {
		// pcb a matar encontrado
		if (current == queue) {	// Se extrae el primer elemento
			queue = current -> next; // Se mueve la cabecera al segundo elemento
		}
		else {
			before -> next = current -> next; // Se ligan los nodos del hueco
		}
		current -> next = NULL;
		pcb_count--;
		return current;
	}
	return NULL; // La cola está vacía
}

void kill_queue(void)
{
	/*PCB *current = NULL;
	PCB *next = NULL;
	current = queue;
	// c n
	// 1 2 3 4 5
	while (current) {
		next = current->next;
		remove_pcb(&current);
		current = next;
	} */
	int i = 30;
	while(queue) {
		PCB *temp = queue;
		queue = queue -> next;
		mvprintw(i,2,"pid=%d", temp->pid);
		remove_pcb(&temp);
		i++;
		refresh();
	}
}
