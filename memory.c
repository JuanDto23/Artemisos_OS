#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>
// Bibliotecas propias
#include "memory.h"
#include "pcb.h"
#include "gui.h"
#include "queue.h"

/* Crear un archivo lleno de ceros al inicio del programa,
   para el manejo de memoria de intercambio (swap) */
void create_swap(FILE **swap)
{
  // Crear el archivo binario SWAP en modo lectura/escritura binaria
  *swap = fopen("SWAP", "w+b");
  if (!(*swap))
  {
    endwin();
    exit(1);
  }

  // Determinar el tamaño en bytes del archivo SWAP
  size_t total_bytes = (size_t)(SWAP_SIZE * INSTRUCTION_SIZE);

  // Crear un bloque de memoria completo lleno de 0's
  int *buffer = (int *)malloc(total_bytes);
  if (!buffer)
  {
    fclose(*swap);
    endwin();
    exit(1);
  }
  memset(buffer, 0, total_bytes);

  // Escribir en todo el archivo SWAP los 0's del bloque de memoria inicializada
  if (fwrite(buffer, 1, total_bytes, *swap) != total_bytes)
  {
    fclose(*swap);
    endwin();
    exit(1);
  }

  // Se libera el bloque de memoria inicializada
  free(buffer);
}

// Cuenta los marcos disponibles en SWAP (puede haber marcos dispersos)
void initialize_tms(TMS *tms)
{
  memset(tms->table, 0, sizeof(int) * MAX_PAGES);
  tms->available_pages = MAX_PAGES;
}

// Lee una línea del archivo y la almacena en el buffer
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

// Lee una instrucción de la SWAP y la almacena en un buffer de línea
void read_inst_from_swap(FILE *swap, char *instruction, PCB *execution_pcb)
{
  /*
    PC = 25
    page_from_PC = 25 / 16 = 1
    offset = 25 % 16 = 9
    execution_pcb->TMP[page_from_PC] = 2
    TMP[0] = 0, TMP[1] = 2
    TMS[0] = 1, TMS[1] = 3,  TMS[2]=1
  */

  // Calcular primero a que marco pertenece el PC (PC/Cantidad de instrucciones por marco)
  //  ¿Cuántos marcos ocupa? ¿En que numero de marco de la tmp está la instrucción actual?
  int index_page_from_tms = execution_pcb->PC / PAGE_SIZE;

  // Calcular el desplazamiento (Offset) en el Marco para el PC (PC%16).
  int offset = execution_pcb->PC % PAGE_SIZE;

  // Marco en SWAP de la TMP para el marco calculado (index_page_from_tms)
  int page_from_swap = execution_pcb->TMP[index_page_from_tms];

  // Se obtiene la dirección real en SWAP (DRS)
  int drs = page_from_swap * PAGE_JUMP | offset * INSTRUCTION_JUMP;

  // Se posiciona el puntero de la SWAP en la instrucción apuntada por PC
  fseek(swap, drs, SEEK_SET);
  // Se almacena la siguiente instrucción en el buffer
  fread(instruction, sizeof(char), INSTRUCTION_JUMP, swap);
}

// Cargar instrucciones en swap y registrar en TMS los marcos ocupados por el proceso
void load_to_swap(PCB *new_process, TMS *tms, FILE **swap, int lines)
{
  int k = 0;                           // Iterador para TMP del nuevo proceso
  int pages_needed = new_process->TmpSize; // Páginas necesitadas por el proceso
  char buffer[INSTRUCTION_SIZE] = {0}; // Buffer para la instrucción leída del archivo
  char clear_instruction_space[INSTRUCTION_SIZE] = {0};

  // Recorre la TMS en busca de marcos disponibles
  for (int i = 0; i < MAX_PAGES; i++)
  {
    // Si el proceso ya no necesita más marcos, termina
    if (!pages_needed)
      break;

    // Busca una página disponible que no este ocupada por el pid del proceso
    if (tms->table[i] == 0)
    {
      // Se leen 16 instrucciones desde el archivo del proceso
      for (int j = 0; j < PAGE_SIZE; j++)
      {
        // Si hay líneas válidas por leer
        if (lines)
        {
          // Se lee una línea del programa del proceso
          read_line_from_file(new_process->program, buffer);
          // Verifica que la línea leída no sea una vacía
          if (buffer[0] != '\0')
          {
            // Posiciona el puntero de archivo SWAP en el marco y desplazamiento correspondiente
            fseek(*swap, i * PAGE_JUMP | j * INSTRUCTION_JUMP, SEEK_SET);
            // Se limpia el espacio de instrucción en la SWAP, para después escribirla
            fwrite(clear_instruction_space, sizeof(char), INSTRUCTION_SIZE, *swap);
            // Se reposiciona el puntero después de haber limpiado el espacio de instrucción
            fseek(*swap, i * PAGE_JUMP | j * INSTRUCTION_JUMP, SEEK_SET);
            // Se escribe la instrucción del buffer a la swap
            fwrite(buffer, sizeof(char), strlen(buffer), *swap);
            // Como ya se escribió una línea, se decrementa las líneas dl archivo del proceso
            lines--;
          }
          else
          {
            // Evita hacer un salto de instrucción en la SWAP si se leyó una línea vacía
            j--;
          }
        }
        else
          // Si el archivo ya no cuenta con más líneas por leer
          break;
      }
      // Se marca la página ocupada en la TMS con el pid del proceso
      tms->table[i] = new_process->pid;
      // Registrar dirección del marco del proceso en la SWAP
      new_process->TMP[k++] = i;
      // Como ya se escribió en un marco, se decrementa el número de marcos necesitados por el proceso
      pages_needed--;
      // Se disminuye la cantidad de marcos disponibles
      tms->available_pages--;
    }
    fflush(*swap); // El contenido pendiente del búfer se vacía en el archivo
  }
}

// Carga un proceso a la cola de Listos, reserva espacio para su TMP y carga sus instrucciones en la SWAP
void load_to_ready(PCB *process, Queue *ready, TMS *tms, FILE **swap)
{
  // Cargarlo a Listos
  enqueue(process, ready);

  // Se reserva espacio para la TMP del proceso
  process->TMP = (int *)malloc(sizeof(int) * process->TmpSize);

  // Cargar instrucciones en swap y registrar en TMS los marcos ocupados por el proceso
  load_to_swap(process, tms, swap, process->lines);

  // Se cierra el archivo una vez que se ha cargado en la swap
  fclose(process->program);
  // Se evita puntero colgante
  process->program = NULL;
}

// Marca como disponibles la páginas que ocupaba un proceso en la TMS, usando su TMP
void free_pages_from_tms(PCB *process_finished, TMS *tms)
{
  for (int i = 0; i < process_finished->TmpSize; i++)
  {
    tms->table[process_finished->TMP[i]] = 0;
    (tms->available_pages)++;
  }
}

// Actualiza la tabla TMS con la UID de un proceso hermano
void update_pages_from_tms(PCB *brothe_process, TMS *tms)
{
  for (int i = 0; i < brothe_process->TmpSize; i++)
  {
    tms->table[brothe_process->TMP[i]] = brothe_process->pid;
  }
}