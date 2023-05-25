#include <stdio.h>
#include <stdlib.h>
#include <err.h>

/*
Comandos compilación
gcc -g show_file.c -o show_file.o
make
*/


int main(int argc, char* argv[]) {
	FILE* file=NULL;
	int ret;
	short int tam;

	if (argc!=2) {
		fprintf(stderr,"Usage: %s <file_name>\n",argv[0]);
		exit(1);
	}
	

	/* Open file */
	if ((file = fopen(argv[1], "r")) == NULL)
		err(2,"The input file %s could not be opened",argv[1]);
	

	do {
		printf("Escribe el tamaño de bloque a leer: ");
		scanf("%hi", &tam);
		char* str = malloc(tam * sizeof(char));

		/* Read file byte by byte */
		ret = fread(str, tam, 1, file);

		/* Print byte to stdout */
		fwrite(str, tam, 1, stdout);

		printf("\n");

		if (ret < 1){
			fclose(file);
			err(3,"fread failed!!");
		}
	} while (ret >= 1);

	fclose(file);
	return 0;
}
