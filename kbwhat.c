#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include "kbhit.h"

int main();

int main(void)
{
	char c;
	char prompt[80];
	
	int i=1;
	
	strcpy(prompt,"Presoina una tecla:");

	while (i) {
		printf("%s\n",prompt);
		
		
		//Se presionó una tecla???
		if (kbhit()) {
			printf("\n");
			printf("Presionaste: ");
			do{
				c=getchar();	// Cuanto vale la tecla presionada?
				printf("[%d].\n", c);
			}while(kbhit());
			
			i=0;
		}		
		
	}		
}
