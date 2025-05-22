#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>
// Bibliotecas propias
#include "memory.h"
#include "pcb.h"
#include "gui.h"

/* Crear un archivo lleno de ceros al inicio del programa,
   para el manejo de memoria de intercambio (swap) */
FILE *create_swap(void)
{
  // Crear el archivo binario SWAP
  FILE *swap = fopen("SWAP", "wb");
  if (!swap)
  {
    endwin();
    exit(1);
  }

  // Determinar el tamaño en bytes del archivo SWAP
  size_t total_bytes = (size_t)TOTAL_INSTRUCTIONS * INSTRUCTION_SIZE;

  // Crear un bloque de memoria completo lleno de 0's
  int *buffer = (int *)malloc(total_bytes);
  if (!buffer)
  {
    fclose(swap);
    endwin();
    exit(1);
  }
  memset(buffer, 0, total_bytes);

  // Escribir en todo el archivo SWAP los 0's del bloque de memoria inicializada
  if (fwrite(buffer, 1, total_bytes, swap) != total_bytes)
  {
    fclose(swap);
    endwin();
    exit(1);
  }

  // Se libera el bloque de memoria inicializada
  free(buffer);
  return swap;
}

// Cuenta los marcos disponibles en SWAP (puede haber marcos dispersos)
void initialize_tms(TMS *tms)
{
  memset(tms->table, 0, sizeof(int) * MAX_PAGES);
  tms->available_pages = MAX_PAGES;
}

// Lee una instrucción del archivo y la almacena en el buffer
void read_line_from_file(FILE *file, char *buffer)
{
  int i = 0;
  int c;

  // Es una línea válida, escribe en el buffer los caracteres antes de salto de línea o retorno de carro
  while ((c = fgetc(file)) != EOF && i < INSTRUCTION_SIZE - 1 && c != '\n' && c != '\r')
  {
    buffer[i++] = c;
  }
  buffer[i] = '\0'; // Marcador para detener la escritura del contenido del buffer, en la SWAP
}

// Cargar instrucciones en swap y registrar en TMS los marcos ocupados por el proceso
void load_to_swap(PCB *new_pcb, TMS *tms, FILE **swap, int lines, GUI *gui)
{
  int k = 0;                           // Iterador para TMP del nuevo proceso
  int pages_needed = new_pcb->TmpSize; // Páginas necesitadas por el proceso
  char buffer[INSTRUCTION_SIZE] = {0};       // Buffer para la instrucción leída del archivo


  // Recorre la TMS en busca de marcos disponibles (i indica el número de páginas a avanzar en la swap)
  for (int i = 0; i < MAX_PAGES; i++)
  {
    // Si el proceso ya no neces
    if (!pages_needed)
      break;
    // Busca una ágina disponible que no este ocupada por el pid del proceso
    if (tms->table[i] == 0)
    {
      // Se lee la instrucción desde archivo de programa (j indica el número de instrucciones a avanzar en la pagina i)
      for (int j = 0; j < PAGE_SIZE; j++)
      {
        if (lines) 
        {
          read_line_from_file(new_pcb->program, buffer);
          if (buffer[0] != '\0')
          {
            wclear(gui->inner_msg);
            mvwprintw(gui->inner_msg, 2, 0, "lines = %d ir = %s", lines, buffer);
            wrefresh(gui->inner_msg);
            sleep(1);
            fseek(*swap, i * PAGE_JUMP | j * INSTRUCTION_SIZE, SEEK_SET);
            // Se escribe la instrucción del buffer a la swap
            fwrite(buffer, sizeof(char), strlen(buffer), *swap);
            mvwprintw(gui->inner_msg, 3, 0, "%lx", ftell(*swap));
            wrefresh(gui->inner_msg);
            sleep(1);
            lines--;
          }
          else
          {
            j--;  // Evita dejar una instrucción vacía en el SWAP si el buffer tiene '\0' en el primer índice
          }
        }
        else
          break;
      }
      tms->table[i] = new_pcb->pid; // Se marca la página ocupada en la TMS con el pid
      // Registrar lista de direcciones de los múltiples marcos que use el nuevo proceso en su TMP
      new_pcb->TMP[k++] = i;
      pages_needed--;
    }   
  }
  /* Impresión de la tms para verificar que si está ocupada
  for(int i = 0; i < 5; i++)
  {
    mvwprintw(gui->inner_msg, 1, i, "%d ",tms->table[i] );

  }
  wrefresh(gui->inner_msg);*/
  // Impresión de la tms para verificar que si está ocupada
  /*
  for(int i = 0; i < 2; i++)
  {
    mvwprintw(gui->inner_msg, i+1, 0, "%d ",new_pcb->TMP[i]);

  }
  wrefresh(gui->inner_msg);*/
}