#ifndef _PCB_H
#define _PCB_H


// Valores numéricos obtenidos de kbwhat.c
#define ENTER 10
#define BACKSPACE 127
#define ESC 27


#define NUMBER_BUFFERS 4
#define SIZE_BUFFER 256
#define SIZE_LINE 16


/* Delay de N milisegundos que se detendrá
* la ejecución del programa entre cada iteración */
#define N 300


// Valores booleanos
#define TRUE 1
#define FALSE 0


// Creación del bloque de control de procesos
typedef struct pcb
{
   int AX;
   int BX;
   int CX;
   int DX;
   int PC;
   char IR[100];
}PCB;


typedef struct option{
   char instruction[4];
   char P1[3];
   char P2[3];
}Option;


/*-------------------PROTOTIPOS -------------------*/


// FUNCIONES DE INICIALIZACIÓN


/*
   PROTOTIPO: void initialize_pcb(PCB * pcb);
  
   PROPOSITO: Inicializa los registros a «0»;
  
   ENTRADAS: Apuntador a tipo PCB (struct pcb).


   SALIDAS: «void»  dado que esta funcion solo inicializa
   los registros no se requiere un valor de retorno.
  
   DESCRIPCIÓN: Se utiliza un apuntador para poder asignar el
   valor inicial «0» a los paarametros de la estructura


*/
void initialize_pcb(PCB * pcb);


/*
   PROTOTIPO: void initialize_buffer(char * buffer, int * index);
  
   PROPOSITO: Inicializa el buffer de prompt y el índice del buffer
   a cero.


   ENTRADAS: Arreglo de caracteres de buffer y apuntador a entero.
  
   SALIDAS: «void» dado que solo se inicializa el buffer de prompt
   e índice y no es necesario regresar nadda.


   DESCRIPCIÓN: A través de un ciclo for se asigna a cero cada elemento
   del buffer, y al terminar, se coloca el índice a cero también.
*/
void initialize_buffer(char * buffer, int * index);


// FUNCIONES AUXILIARES
/*
   PROTOTIPO: void strUpper(char * str);
  
   PROPÓSITO: El propósito de esta función es convertir a mayúsculas
   el comando capturado desde la línea de comandos.
  
   ENTRADAS: Recibe un apuntador a una variable de tipo carácter.
  
   SALIDAS: «void» Dado que esta función solo se encarga de convertir
   cada carácter del arreglo de carac
   
   DESCRIPCIÓN: De forma más específica, esta función recibe la dirección de memoria del
   primer elemento del arreglo de caracteres de la línea de comandos.,
*/
void strUpper(char * str);

/*
	PROTOTIPO: int isNumeric(char *str)

	PROPÓSITO: Reconocer si la cadena ingresada es un valor numérico

	ENTRADAS: Puntero a el primer valor de una cadena de caracteres

	SALIDAS: Regresa «TRUE» si la cadena de entrada representa un valor
	numérico, regresa «FALSE» de lo contrario.

	DESCRIPCIÓN: Por medio de varias condiciones 
	va descartando diferentes valores en la cadena como «NULL» 
	y «\0»; después mediante un ciclo while verifica con 
	la función isdigit que los valores de la cadena no sean caracteres.
*/
int isNumeric(char *str);

/*
   PROTOTIPO: void clear_prompt(int row);
  
   PROPÓSITO: Se encarga de limpiar una línea en la ventana de ncurses,
   la cual puede corresponder a la prompt o una línea del historial.


   ENTRADAS: «int row» que significa la línea a limpiar, sin borrar la
   cadena de prompt ("Artemisos>").
  
   SALIDAS: «void» dado que no es necesario regresar nada.


   DESCRIPCIÓN: Para borrar la línea, se considera a borrar después
   la longitud de línea del tamaño de prompt (“Artemisos»”).
*/
void clear_prompt(int row);

/*
   PROTOTIPO: void clear_messages(void);
  
   PROPÓSITO: Se encarga de limpiar la sección de mensajes para 
   la impresión de nuevos mensajes.


   ENTRADAS: «void» Esta función no requiere ninguna variable de entrada.
  
   SALIDAS: «void» dado que no es necesario regresar nada.


   DESCRIPCIÓN: Para limpiar la sección de mensajes, esta función imprime 	
   espacios en blanco en las coordenadas especificadas.
*/
void clear_messages(void);

/*
   PROTOTIPO: int search_register(char * p2);
  
   PROPÓSITO: Se encarga de comparar la cadena recibida por los nombres
   de registros válidos (AX, BX, CX, DX).


   ENTRADAS: «char *p» el cual corresponde a la cadena, ya sea del
   primer o segundo parámetro.
  
   SALIDAS: «int» regresa el valor numérico de la inicial del registro
   en cuestión. Por ejemplo, si el parámetro p coincide la cadena del
   registro “AX”, se retorna el valor numérico del carácter ‘A’. Si no
   coincide con ningún registro, retorna -1.


   DESCRIPCIÓN: Se compara la cadena del parámetro recibido “p” con
   cadenas de registro válidos (AX, BX, CX, DX), mediante la función
   strcmp. Y hay coincidencia, se retorna el valor numérico de la
   inicial del registro en cuestión.
*/
int search_register(char * p);  


/*
   PROTOTIPO: int value_register(const PBC *pbc, char r);


   PROPÓSITO: Esta función permite obtener el valor almacenado 
   en el registro del PBC especificado.


   ENTRADAS: Recibe un apuntador de tipo PBC y una variable de 
   tipo caracter, que representa un registro del procesador.
  
   SALIDAS: «int» regresa el entero almacenado en el registro «r» 


   DESCRIPCIÓN:  Utiliza un apuntador de tipo PCB para acceder a
   a los miembros de la instancia PBC y obtener el valor del miembro
   especificado según el parámetro «r».
*/               
int value_register(const PCB *pcb, char r);


// FUNCIONES PRINCIPALES
/*
   PROTOTIPO: int command_handling(
   charbuffers[NUMBER_BUFFERS][SIZE_BUFFER],
   int *read_flag, int *c, int *index, FILE **f, int *index_history);
  
   PROPÓSITO: Se encarga de la captación carácter por carácter, así
   como también de la gestión de teclas especiales (ENTER, BACKSPACE,
   KEY_UP, KEY_DOWN, ESC).

   ENTRADAS: Se reciben las principales variables de control
   declaradas en la función main.
  
   SALIDAS: Se retorna un valor TRUE solo de confirmación de que todo
   salió bien.

   DESCRIPCIÓN: Mediante la función kbhit se capta cada tecla
   presionada. Si se presiona la tecla de retroceso (KEY_BACKSPACE), 
   se borra el carácter del buffer prompt y también de la ventana
   de ncurses. Si se presiona la tecla abajo (KEY_DOWN), se recupera
   el comando anterior. Si se presiona la tecla arriba (KEY_UP), se
   devuelve al comando que se tenía. Si se presiona la tecla de escape
   (ESC), se refiere a un atajo de salida rápido. Si se presiona la
   tecla enter (ENTER), se coloca carácter nulo ‘\0’ al final del
   buffer prompt para que sea procesado por la función evaluate_command.
   Si no se es ninguna de estas teclas especiales, solo se concatena 
  el carácter al buffer prompt.
*/

int command_handling(char buffers[NUMBER_BUFFERS][SIZE_BUFFER],
   int *read_flag, int *c, int *index, FILE **f, int *index_history);


/*
   PROTOTIPO: int evaluate_command(char * buff, int *index, int *read_flag, FILE **f);

   PROPÓSITO: Esta función se encarga de analizar los comandos
    de entrada que ingrese el usuario en el prompt.


   ENTRADAS: Recibe un arreglo de caracteres de buffer, dos apuntadores
    a tipo entero y un apuntador doble a un tipo FILE.
  
   SALIDAS: Regresa «TRUE» si todo salió bien.


   DESCRIPCIÓN: Convierte la cadena de entrada que ingresó el usuario
   con uno de los comandos predeterminados «EXIT» y «LOAD», si la entrada
   coincide con el primer comando termina el programa y cierra la ventana;
   si la entrada corresponde al segundo comando se le asigna al apuntador
   «*f» la dirección del archivo conjunto.
*/


int evaluate_command(char *buff, int *index, int *read_flag, FILE **f);

/*
   PROTOTIPO: int read_line(FILE **f, char *line);


   PROPÓSITO: Esta función es la encargada de leer línea a línea
   el archivo cargado.


   ENTRADAS: Recibe un doble apuntador de tipo FILE y un apuntador 
   a una variable de tipo caracter que representa el primer elemento
   del arreglo de caracteres donde se almacena la línea leída 
   del archivo.
  
   SALIDAS: «int» regresa un valor entero que sirve para comprobar
   si el archivo cargado está siendo leído o se ha terminado 
   de procesar correctamente.


   DESCRIPCIÓN: Esta función utiliza el operador de desreferenciación
   para acceder al puntero FILE que guarda la dirección de memoria 
   del archivo cargado. Usa la función fgets para extraer una línea
   del archivo. Utiliza la función feof para verificar si encontró el
   caracter de fin de archivo en el archivo abierto. Además, cada
   línea extraída es convertida a mayúsculas para interpretación.
   La función regresa cero si se ha encontrado el fin de archivo,
   para liberar el puntero al archivo ya procesado.
*/               


int read_line(FILE **f, char *line);

/*
   PROTOTIPO: int interpret_instruction(char * line, PCB *pcb);
  
   PROPÓSITO: Se encarga de leer la instrucción leída del archivo
   cargado.


   ENTRADAS: Se recibe la línea actual leída (char *line) y el apuntador
   a nuestro procesador (PCB *pcb) que contiene los registros.
  
   SALIDAS: Retorna 0 si la instrucción leída es END. Retorna -1 si la
   instrucción no es identificada, o si los parámetros son inválidos o
   faltantes. Retorna 1 si la instrucción es correcta.


   DESCRIPCIÓN: Primeramente se divide la instrucción en tokens. Después
   se declara una bandera is_num_p2 que indica si el segundo parámetro
   es numérico o no. Lo primero que se comprueba es si la instrucción
   es END, si lo es, termina la función. Por otro lado, si no es END,
   entonces se comprueba que el primer parámetro sea un registro válido, 
   dado sólo puede ser un registro. Si se cumple lo anterior,
   entonces se comprueba que el segundo parámetro sea registro o
   numérico. Si es registro, entonces se extrae el valor del
   registro indicado en el segundo parámetro; sino, se extrae el
   valor descrito. Una vez comprobado lo anterior, se comprueba de si
   la instrucción es de 3 parámetros o de 2, a través de la variable
   items. Y ya dependiendo de la instrucción coincidente, se realiza la
   operación necesaria.
*/


int interpret_instruction(char * line, PCB *pcb);

/*
   PROTOTIPO: void print_processor(void);
  
   PROPÓSITO: Imprimir el apartado del procesador.


   ENTRADAS: «void» Dado a que la función no tiene el propósito
    de   utilizar ningún parámetro exterior.


   SALIDAS: «void» Esta función solo se encarga de impresiones
    por lo que no requiere un valor de retorno.


   DESCRIPCIÓN: Imprime y posiciona en la ventana el
    apartado donde se representan los registros.
*/
void print_processor(void);


/*
   PROTOTIPO: void print_messages(void);


   PROPÓSITO: Esta función imprime los delimitadores de la sección
   de mensajes.


   ENTRADAS: «void» dado que no trabaja con ninguna variable.
  
   SALIDAS: «void» dado que no se requiere ningún valor control.


   DESCRIPCIÓN: Imprime caracteres que delimitan la sección donde se
   muestran los mensajes para el usuario: mensajes de errores de 
   sintaxis, de error de instrucción, de estado de lectura de archivo, 
   entre otros indicadores.
*/
void print_messages(void); 

/*
   PROTOTIPO: print_history(char buffers[NUMBER_BUFFERS][SIZE_BUFFER]);
  
   PROPÓSITO: Imprimir en la ventana los comandos anteriormente ingresados.


   ENTRADAS: Matriz bidimensional de caracteres, donde 
   «NUMBER_BUFFERS» indica el número de buffers y «SIZE_BUFFER» la longitud de los buffers.


   SALIDAS: «void» dado que no se requiere ningún valor control.


   DESCRIPCIÓN: Imprime la promt «Artemisos>»seguida de cadenas
   de caracteres almacenadas en los buffers de entrada, que representan
   los comandos anteriormente ingresados por el usuario.

*/
void print_history(char buffers[NUMBER_BUFFERS][SIZE_BUFFER]);

/*
   PROTOTIPO: void print_registers(const PCB *pcb);


   PROPÓSITO: Esta función se encarga de imprimir los valores que 
   almacenan los registros del procesador (pcb).


   ENTRADAS: Recibe un apuntador de tipo PCB para acceder a los 
   miembros del pcb.
  
   SALIDAS: «void» dado que solo hace la impresión de los valores.


   DESCRIPCIÓN: Accede a los miembros de la instancia pcb por medio del
   apuntador pcb, para imprimir los valores del almacenados en los 
   miembros de la instancia, y que el usuario vea en tiempo real el 
   estado de los registros del procesador. 
*/
void print_registers(const PCB *pcb);

#endif
