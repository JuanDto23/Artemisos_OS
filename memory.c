#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
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
  int k = 0;
  int pages_needed = new_pcb->TmpSize;
  char buffer[INSTRUCTION_SIZE];

  for (int i = 0; i < MAX_PAGES; i++)
  {
    if (!pages_needed)
      break;

    if (tms->table[i] == 0) 
    {
      // Se lee la instrucción desde archivo de programa
      for (int j = 0; j < PAGE_SIZE; j++)
      {
        if (lines)
        {
          read_line_from_file(new_pcb->program, buffer);
          if (buffer[0] != '\0')
          {
            lines--;
            fseek(*swap, i*0x200 | j*0x20, SEEK_SET);
            // Se escribe la instrucción del buffer a la swap
            fwrite(buffer, sizeof(char), strlen(buffer), *swap);
            tms->table[i] = new_pcb->pid; // Se marca la página ocupada en la TMS con el pid
            pages_needed--;
            // Registrar lista de direcciones de los múltiples marcos que use el nuevo proceso en su TMP
            new_pcb->TMP[k++] = i;
          }
          else {
            j--;
          }
        }
        else
          break;
      }

    }
  }
}