#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define NBYTES 512

/**
 * Función que copia el contenido de un archivo en otro
 * @param fdo descriptor de archivo origen
 * @param fdd descriptor de archivo destino
 */
void copy(int fdo, int fdd){
	char buffer[NBYTES];
	int nbytesRead;
	while ((nbytesRead = read(fdo, buffer, NBYTES)) > 0){
		if(write(fdd, buffer, nbytesRead) < nbytesRead){
			perror("Error en la escritura\n");
			close(fdd); close(fdo);
			exit(EXIT_FAILURE);
		}
	}
	if(nbytesRead < 0){
		perror("Error en la lectura\n");
		close(fdd); close(fdo);
		exit(EXIT_FAILURE);
	}
	close(fdd); close(fdo);
}

/**
 * Función principal
 * @param argc número de argumentos
 * @param argv vector de argumentos
 * @return EXIT_SUCCESS si no ha habido errores, EXIT_FAILURE en caso contrario
 */
int main(int argc, char *argv[]){
	int fdo, fdd;
	
	fdo = open(argv[1], O_RDONLY);
	if (fdo < 0){
		perror("No se ha podido leer el archivo origen\n");
		exit(EXIT_FAILURE);
	}

	fdd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fdd < 0){
		perror("No se ha podido abrir o crear el archivo destino\n");
		close(fdo);
		exit(EXIT_FAILURE);
	}
	copy(fdo, fdd);
	
	return EXIT_SUCCESS;
}
