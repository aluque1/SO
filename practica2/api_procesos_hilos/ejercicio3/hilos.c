#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

/**
 * struct thread_info - Estructura que contiene la información de un hilo
 * @id: identificador del hilo
 * @userNum: número de usuario
 * @priority: prioridad del hilo
 */
typedef struct
{
	pthread_t id;
	int userNum;
	char priority;
} thread_info;

/**
 * Función que ejecuta cada hilo. Tiene que ser de tipo void * y tener un único argumento de tipo void * arg
 * @param arg puntero a la estructura thread_info
 * @return NULL
 */
void *thread_usuario(void *arg)
{
	thread_info *tinfo = arg;
	pthread_t id = tinfo->id;
	int userNum = tinfo->userNum;
	char priority = tinfo->priority;
	printf("Hello, my thread id is %lu, my userNumer is %d, and my priority is %c\n", id, userNum, priority);
	return NULL;
}

/**
 * Programa que crea N hilos (N = argc - 1), 
 * cada uno de los cuales imprime su identificador de hilo, su número de usuario y su prioridad.
 * @param argc número de argumentos
 * @param argv vector de argumentos
 * @return EXIT_SUCCESS si no ha habido errores, EXIT_FAILURE en caso contrario
*/
int main(int argc, char *argv[])
{
	int nThreads, i;
	void *res;
	pthread_attr_t attr;
	thread_info *tInfo;
	struct sched_param param;

	if (pthread_attr_init(&attr))
	{
		perror("Thread attribute could not be created\n");
		exit(EXIT_FAILURE);

	nThreads = argc - 1;
	tInfo = malloc(sizeof(thread_info) * nThreads);

	for (i = 0; i < nThreads; ++i)
	{	
		// Setea el userNum de cada hilo
		if (sscanf(argv[i + 1], "%d", &tInfo[i].userNum) == EOF)
		{
			printf("Argument: %s could not be read", argv[i + 1]);
			exit(EXIT_FAILURE);
		}
		// Setea la prioridad de los hilos priorizando los pares
		tInfo[i].priority = (tInfo[i].userNum % 2) ? 'P' : 'N';
		
		// Create thread
		if (pthread_create(&tInfo[i].id, &attr, &thread_usuario, &tInfo[i]))
		{
			perror("Error creating thread");
			exit(EXIT_FAILURE);
		}
	}

	if (pthread_attr_destroy(&attr))
	{
		perror("Error destroying attr");
		exit(EXIT_FAILURE);
	}

	// Join threads (cierre)
	for (i = 0; i < nThreads; ++i)
	{
		if (pthread_join(tInfo[i].id, &res))
			perror("Error joining thread\n");

		free(res);
	}

	free(tInfo);

	return EXIT_SUCCESS;
}
