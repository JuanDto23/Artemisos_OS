#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// Bibliotecas propias
#include "memory.h"

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
  memset(tms -> table, 0, MAX_PAGES); 
  tms -> available_pages = MAX_PAGES;
}
