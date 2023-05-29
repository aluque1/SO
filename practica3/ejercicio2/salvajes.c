#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/mman.h>
#include "shared.h"

#define NUMITER 3 //numero de veces que comerá del caldero
#define M 10 // numero de raciones que se pueden poner en el caldero
#define TAM 150 // tamaño del buffer

sem_t *caldero; // misionero.c
sem_t *cocinero; // misionero.c
sem_t *salvajes; // misionero.c

sharedData* data; // puntero a la estructura de datos compartida

char* shm; // shared memory pointer
sharedData* shm2; // shared memory pointer

// Funciones:
void eat(char* part);
void savages(void);
void getServingSafe();
int getServingsFromPot(void);


int main(int argc, char *argv[])
{
	int shd; // shared memory descriptor
	int shd2; // shared memory descriptor

	// inicializacion de la memoria compartida
	shd = shm_open("Caldero", O_RDONLY, 0666);
	if (shd == -1){
		perror("shm_open");
		exit(EXIT_FAILURE);
	}
	shm = (char*) mmap(NULL, TAM, PROT_READ, MAP_SHARED, shd, 0);
	if (shm == MAP_FAILED){
		perror("mmap");
		exit(EXIT_FAILURE);
	}

	shd2 = shm_open("Datos", O_RDWR, 0666);
	if (shd2 == -1){
		perror("shm_open2");
		exit(EXIT_FAILURE);
	}
	if (ftruncate(shd2, sizeof(sharedData)) == -1){
		perror("ftruncate2");
		exit(EXIT_FAILURE);
	}
	shm2 = (sharedData*) mmap(NULL, sizeof(sharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shd2, 0);
	if (shm2 == MAP_FAILED){
		perror("mmap2");
		exit(EXIT_FAILURE);
	}

	data = malloc(sizeof(sharedData)); 
	data = shm2; // inicializacion de la estructura de datos compartida

	// inicializacion de los semaforos
	caldero = sem_open("Caldero", O_RDONLY);
	if (caldero == SEM_FAILED){
		perror("sem_open");
		exit(EXIT_FAILURE);
	}
	cocinero = sem_open("Cocinero", O_RDONLY);
	if (cocinero == SEM_FAILED){
		perror("sem_open");
		exit(EXIT_FAILURE);
	}
	salvajes = sem_open("Salvajes", O_RDONLY);
	if (salvajes == SEM_FAILED){
		perror("sem_open");
		exit(EXIT_FAILURE);
	}

	savages();

	close(shd2); // cierra el descriptor de la memoria compartida
	close(shd); // cierra el descriptor de la memoria compartida

	sem_close(caldero); // cierra el semaforo del caldero
	sem_close(cocinero); // cierra el semaforo del cocinero
	sem_close(salvajes); // cierra el semaforo de los salvajes

	return 0;
}

int getServingsFromPot(void)
{
	int i;
	char* part = (char*) malloc(sizeof(char) * 20);
	strcpy(part, &shm[data->pos]);
	eat(part);
	data->pos += (int)(strlen(part) + 1);

}
void savages(void)
{
	int i;
	for (i = 0; i < NUMITER; i++)
	{
		getServingSafe();
		printf("Quedan %d raciones en el caldero\n", data->numServings);
	}
	
	
}

/*
wait(caldero);

while(data->numServings == 0){
	if(data->numCoockers > 0){
		signal(cocinero);
		--(data->numCoockers);
	}
	++(data->numSalvajesWaiting);
	signal(caldero);
	wait(salvajes);
	wait(caldero);
}

getServingsFromPot();
--(data->numServings);

post(caldero);

*/
void getServingSafe(){
	sem_wait(caldero); /* lock */

	while (data->numServings == 0)
	{
		/* cond_signal */
		if (data->numCoockers > 0)
		{
			printf("Despertando a un cocinero\n");
			sem_post(cocinero);
			--(data->numCoockers);
		}
		/* cond_wait */
		++(data->numSalvajesWaiting);
		sem_post(caldero); // libera el caldero
		printf("Salvaje %d esperando\n", (int)getpid());
		sem_wait(salvajes); // se pone a la cola para coger el caldero
		sem_wait(caldero); // coge el caldero
	}

	getServingsFromPot();
	--(data->numServings);

	sem_post(caldero);

}

void eat(char* part)
{
	unsigned long id = (unsigned long) getpid();
	printf("Savage %lu eating misionero's %s : ", id, part);
	sleep((unsigned)rand() % 2);
}