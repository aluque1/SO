#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define NBYTES 512


void copy(int fdo, int fdd);
void copy_regular(char *orig, char *dest);
void copy_link(char *orig, char *dest);

/**
 * Función principal
 * @param argc número de argumentos
 * @param argv vector de argumentos
 * @return EXIT_SUCCESS si no ha habido errores, EXIT_FAILURE en caso contrario
*/
int main(int argc, char *argv[])
{

	if (argc == 3)
	{
		struct stat *stat_buff = malloc(sizeof(struct stat));

		lstat(argv[1], stat_buff); // Usa lstat y mete el campo stat en el buffer declarado justo arriba

		// S_ISREG(m) es una funcion que devuelve true si el modo de stat es equivalente a de un fichero regular
		if (S_ISREG(stat_buff->st_mode) == 1)
		{ // si es un fichero regular  TODO Esto esta todo en <sys/stat.h>
			copy_regular(argv[1], argv[2]);
			return EXIT_SUCCESS;
		}
		else if (S_ISLNK(stat_buff->st_mode) == 1) // S_ISLNK(m) es una funcion que devuelve true si el modo de stat es equivalente a de un enlace simbolico
		{ // si es un enlace simbolico
			copy_link(argv[1], argv[2]);
			return EXIT_SUCCESS;
		}
		else
		{
			printf("El fichero origen tiene que ser un fichero regular o un enlace simbolico");
			return EXIT_FAILURE;
		}
	}
	else
		return EXIT_FAILURE;
}

/**
 * Función que copia el contenido de un archivo en otro
 * @param fdo descriptor de archivo origen
 * @param fdd descriptor de archivo destino
 * @return EXIT_SUCCESS si no ha habido errores, EXIT_FAILURE en caso contrario
*/
void copy(int fdo, int fdd)
{
	char buffer[NBYTES];
	int nbytesRead;
	while ((nbytesRead = read(fdo, buffer, NBYTES)) > 0)
	{
		if (write(fdd, buffer, nbytesRead) < nbytesRead)
		{
			perror("Error en la escritura\n");
			close(fdd);
			close(fdo);
			exit(EXIT_FAILURE);
		}
	}
	if (nbytesRead < 0)
	{
		perror("Error en la lectura\n");
		close(fdd);
		close(fdo);
		exit(EXIT_FAILURE);
	}
	close(fdd);
	close(fdo);
}

/**
 * Función que copia el contenido de un archivo regular en otro
 * @param orig nombre del archivo origen
 * @param dest nombre del archivo destino
 * @return EXIT_SUCCESS si no ha habido errores, EXIT_FAILURE en caso contrario
*/
void copy_regular(char *orig, char *dest)
{
	int fdo, fdd;
	fdo = open(orig, O_RDONLY);
	if (fdo < 0)
	{
		perror("No se ha podido leer el archivo origen\n");
		exit(EXIT_FAILURE);
	}

	fdd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fdd < 0)
	{
		perror("No se ha podido abrir o crear el archivo destino\n");
		close(fdo);
		exit(EXIT_FAILURE);
	}
	copy(fdo, fdd);
}

/**
 * Función que copia el contenido de un enlace simbolico en otro
 * @param orig nombre del enlace simbolico origen
 * @param dest nombre del enlace simbolico destino
 * 
 * primero vamos a tener que reservar memoria, para saber el nBytes vamos a usar lstat()
 * hacemos + 1 a esto ya que el caracter de fin de cadena no se guarda
 *
 * despues vamos a copiar al buffer la ruta del fichero apuntado haciendo uso de
 * readlink(). Tenemos que añadir manualmente el '\0'
 *
 * por ultimo creamos un enlace simbolico con los datos obtenidos usando la funcion
 * symlink()
*/
void copy_link(char *orig, char *dest)
{
	struct stat *stat_buff = malloc(sizeof(struct stat));
	char *data_buff;
	lstat(orig, stat_buff);
	data_buff = malloc(stat_buff->st_size + 1);
	int tam = readlink(orig, data_buff, sizeof(data_buff) + 1);
	data_buff[tam] = '\0';
	symlink(data_buff, dest);
	free(data_buff);
}