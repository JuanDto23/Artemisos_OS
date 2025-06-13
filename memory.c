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

char RAM[RAM_SIZE][INSTRUCTION_SIZE] = {0};

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
  memset(tms->table, 0, sizeof(int) * MAX_PAGES_SWAP);
  tms->available_pages = MAX_PAGES_SWAP;
}

// Inicializa la tabla de marcos de memoria (TMM)
void initialize_tmm(TMM *tmm)
{
  memset(tmm->table, 0, sizeof(int) * RAM_SIZE / PAGE_SIZE);
  memset(tmm->referenced, 0, sizeof(int) * RAM_SIZE / PAGE_SIZE);
  tmm->available_pages = RAM_SIZE / PAGE_SIZE;
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

// Lee una instrucción de la SWAP y la almacena en un buffer de instrucción
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
  int page_from_swap = execution_pcb->tmp.inSWAP[index_page_from_tms];

  // Se obtiene la dirección real en SWAP (DRS)
  int drs = page_from_swap * PAGE_JUMP | offset * INSTRUCTION_JUMP;

  // Se posiciona el puntero de la SWAP en la instrucción apuntada por PC
  fseek(swap, drs, SEEK_SET);
  // Se almacena la siguiente instrucción en el buffer
  fread(instruction, sizeof(char), INSTRUCTION_JUMP, swap);
}

/* Lee una instrucción de la RAM, almacena la instrucción en el buffer instruction y realiza
   las acciones necesarias cuando la página de la instrucción no se encuentra en RAM (fallo de pagión) */
void read_inst_from_ram(GUI *gui, int ram_disp, char *instruction, PCB *execution_pcb, TMM *tmm, FILE *swap, int *clock)
{
  Address address_instruction = {0};
  if (!execution_pcb)
    return;
  // La Dirección Virtual (PC), ahora deberá traducirse a la Dirección Real de la instrucción correspondiente en RAM
  address_instruction = address_traduction(execution_pcb);
  // Verificar que la dirección real apunte a un marco que se encuentre cargado en RAM (En SWAP siempre debería estar cargado).
  // Para verificar que el marco se encuentre cargado en RAM, deberá apoyarse en la TMP (Valor positivo para Marco en RAM y Presencia en 1).
  if (execution_pcb->tmp.inRAM[address_instruction.base_page] > -1  && execution_pcb->tmp.ram_presence[address_instruction.base_page] == 1)
  {
    // Copiar la instrucción al buffer (instruction)
    strcpy(instruction, RAM[execution_pcb->tmp.inRAM[address_instruction.base_page]*PAGE_SIZE + address_instruction.offset]);
    return;
  }
  else // Fallo de paginación
  {
    // Deberá buscar el primer marco libre en RAM, para cargar el proceso.
    load_to_ram(execution_pcb, tmm, swap, address_instruction, clock);
    // Se llama porque en este punto ya existe un mapeo de marco de la instrucción a ejecutar en RAM
    read_inst_from_ram(gui, ram_disp, instruction, execution_pcb, tmm, swap,clock);

    // Se actualizan la impresión RAM y TMM
    print_ram(gui->inner_ram, ram_disp);
    print_tmm(gui->inner_tmm, *tmm);
  }
}

void load_to_ram(PCB *pcb_execution, TMM *tmm, FILE *swap, Address address, int * clock)
{
  // Recorre la TMM en busca del primer marco libre en RAM
  for (int i = 0; i < MAX_PAGES_RAM; i++)
  {
    // Página disponible en RAM
    if (!tmm->table[i])
    {
      int page_from_swap = pcb_execution->tmp.inSWAP[address.base_page];

      // Copiar el marco en cuestión de la SWAP a la RAM con una sola instrucción fread.
      fseek(swap, page_from_swap * PAGE_JUMP, SEEK_SET); // Se ubica el puntero en la swap para leer el marco en la ram
      fread(RAM[i*PAGE_SIZE], sizeof(char), PAGE_JUMP, swap); // Lee 512 bytes = 1 página y se almacena en el buffer
      
      // Actualizar la TMM con el PID del proceso que ahora ocupa el marco de RAM
      tmm->table[i] = pcb_execution->pid;

      // Establecer el indicador de Referencia en 1 (dado que se acaba de utilizar ese marco).
      tmm->referenced[i] = 1;
      tmm->available_pages--; // Pagina ocupada

      // Actualizar la TMP del proceso, indicando en que marco de la RAM se encuentra el marco actual del proceso.
      pcb_execution->tmp.inRAM[address.base_page] = i;

      // Y establecer en 1 el valor de presencia (Ya se cargó, ya está presente).
      pcb_execution->tmp.ram_presence[address.base_page] = 1;
      
      // El reloj solo se mueve despues del primer desalojo (al menos las 12 primeras páginas no las mueve)
      if(*clock != -1)
      {
        (*clock)++;
      }
      return ;
    } 
  }
  // Si no hay marcos libres, deberá desalojar alguno, auxiliado de la TMM y el algoritmo de reloj.
  clock_algorithm(tmm, clock); // Establece la presencia de algun marco en la tmm como 0 para poder cargar la página que se necesita
  return;               // Ocupa de una llamada recursiva, solo prepara las condiciones para que despues entre la página
}

// Cargar instrucciones en swap y registrar en TMS los marcos ocupados por el proceso
void load_to_swap(PCB *new_process, TMS *tms, FILE **swap, int lines)
{
  int k = 0;                               // Iterador para TMP del nuevo proceso
  int pages_needed = new_process->TmpSize; // Páginas necesitadas por el proceso
  char buffer[INSTRUCTION_SIZE] = {0};     // Buffer para la instrucción leída del archivo
  char clear_instruction_space[INSTRUCTION_SIZE] = {0};

  // Recorre la TMS en busca de marcos disponibles
  for (int i = 0; i < MAX_PAGES_SWAP; i++)
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
      new_process->tmp.inSWAP[k++] = i;
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

  // Se reserva espacio para la TMP del proceso y se inicializa inRAM y ram_presence
  process->tmp.inSWAP = (int *)malloc(sizeof(int) * process->TmpSize);
  process->tmp.inRAM = (int *)malloc(sizeof(int) * process->TmpSize);
  process->tmp.ram_presence = (int *)malloc(sizeof(int) * process->TmpSize);
  memset(process->tmp.inRAM, -1, process->TmpSize* sizeof(int));  // -1 = La pagina no ha sido mapeada a una dirección real en RAM
  memset(process->tmp.ram_presence, 0, process->TmpSize * sizeof(int)); 

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
    tms->table[process_finished->tmp.inRAM[i]] = 0;
    (tms->available_pages)++;
  }
}

// Actualiza la tabla TMS con la UID de un proceso hermano
void update_pages_from_tms(PCB *brothe_process, TMS *tms)
{
  for (int i = 0; i < brothe_process->TmpSize; i++)
  {
    tms->table[brothe_process->tmp.inRAM[i]] = brothe_process->pid;
  }
}

// "Libera" una página de la RAM para poder cargar la página apuntada por PC
// Cuando encuentra un 1, pone en 0 los 16 renglones de la TMM comenzando por el primer marco donde encontró el primer 1
// de tal forma el el marco donde encontró el 1 será el que alojará la nuevo paǵina que ocasionó el deshalojo (le da la vuelta con %)
void clock_algorithm(TMM * tmm, int * clock)
{
  if(*clock == -1)  // Apenas se producirá el primer desalojo
    *(clock)++;
  int count = 0;  // Cuenta el número de renglones modificados en la TMM

  // Primer renglón se fuerza el cambio a 1, a partir de ese 15 mas como consecuencia
  while(true)
  {
    if(tmm ->referenced[(*clock)%PAGE_SIZE]) // Marca 16 0's comenzando por el marco con el primer 1 encontrado
    {
      tmm -> referenced[(*clock)%PAGE_SIZE] = 0;  // Se marca la primera página como disponible
      count++;  // Leva un cambio en la TMM
      (*clock)++; // Los 15 cambios restantes comienzan en el siguiente renglón de la tmm
      break;
    }
    (*clock)++;
  }
  for(;count<MAX_PAGES_RAM; count++)
  {
    tmm -> referenced[(*clock) % PAGE_SIZE] = 0;  // Se marca la primera página como disponible
    (*clock)++;
  }
  // Tengo duda si el clock pudiera desbordarse
}