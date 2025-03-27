#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "kbhit.h"

// Valores numéricos obtenidos de kbwhat.c
#define ENTER 10
#define BACKSPACE 127

void print_buffer(int cnt, char *);
char buff[200];

int main() {

	int cnt_p = 0;
	char c = 0;
	int index_b = 0;

	while(1) {
		/* Imprime en pantalla el prompt, junto a lo que esté
		   almacenado en el buffer de comando */
		printf("[%d]Artemisos>", cnt_p%11);
		print_buffer(index_b, buff);
		printf("\n");
		
		if(kbhit()) { // ¿Se ha presionado alguna tecla?

			c = getchar(); // toma la tecla desde el buffer de entrada

			if(c == ENTER) { //¿Se presionó enter?
				buff[index_b+1] = '\0';
				if(!strcmp(buff, "EXIT")) { // ¿El comando es EXIT?
					exit(0);
				}
                else {
					/* Imprime en una nueva línea
					   el comando guardado en el buffer */
					printf("\n\n\n");
					printf("[%d]Artemisos>", cnt_p%11);
					print_buffer(index_b, buff);
					printf("\n\n\n");
					sleep(3);
					// Se limpia el buffer
					index_b = 0;
					buff[index_b] = '\0';
				}
			} 
			else if(c == BACKSPACE) { // ¿Se presionó backspace?
                // Se comprueba de que haya carácteres en el buffer
				if(index_b > 0) {
					buff[index_b] = '\0';
					index_b--;
				}
				else if(index_b == 0) {
					buff[index_b] = '\0';
				}
			}
			else {
				buff[index_b]=c;
				index_b++;
			}		
		}	
		cnt_p++;
	}

	return 0;
}

void print_buffer(int cnt, char *buff) {
	for(int i=0; i<= cnt; i++) {
		printf("%c", buff[i]);
	}
}
