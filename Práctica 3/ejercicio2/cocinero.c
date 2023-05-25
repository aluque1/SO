#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <stdbool.h>
#include "shared.h"


#define M 10 // numero de raciones que se pueden poner en el caldero
#define TAM 160 // tamaño del buffer

const char* misionero[M] = {"nariz", "lengua", "pantorrilla", "ojo", "oreja", "corazon", "cerebro", "mano", "pie", "muslo"};

// Variables globales:



sharedData *data; // puntero a la estructura de datos compartida

int finish = 0; // indica si el cocinero ha terminado de cocinar

sem_t *caldero; // indica si el caldero esta cogido
sem_t *cocinero; // representa la cola de concineros esperando a que el caldero este libre
sem_t *salvajes; // representa la cola de salvajes esperando a que el caldero este libre

char* shm; // shared memory pointer
sharedData* shm2; // shared memory pointer

// Funciones:
void putServingsSafe(int servings);
void putServingsInPot(int servings);
void cook(void);
void handler(int signo);

int main(int argc, char *argv[])
{
	struct sigaction sa;
	int shd; // shared memory descriptor
	int shd2; // shared memory descriptor


	// inicializacion de señales
	sa.sa_flags = SA_SIGINFO;
	sa.sa_handler = handler;
	if(sigaction(SIGINT, &sa, NULL) == -1) {
		perror("sigaction creando cocinero");
		exit(EXIT_FAILURE);
	}


	// inicializacion de la memoria compartida
	shd = shm_open("Caldero", O_CREAT | O_RDWR, 0666);
	if (shd == -1){
		perror("shm_open");
		exit(EXIT_FAILURE);
	}
	if (ftruncate(shd, TAM) == -1){
		perror("ftruncate");
		exit(EXIT_FAILURE);
	}

	shm = (char*) mmap(NULL, TAM, PROT_READ | PROT_WRITE, MAP_SHARED, shd, 0);
	if (shm == MAP_FAILED){
		perror("mmap");
		exit(EXIT_FAILURE);
	}

	shd2 = shm_open("Datos", O_CREAT | O_RDWR, 0666);
	if (shd2 == -1){
		perror("shm_open");
		exit(EXIT_FAILURE);
	}
	if (ftruncate(shd2, sizeof(sharedData)) == -1){
		perror("ftruncate");
		exit(EXIT_FAILURE);
	}

	shm2 = (sharedData*) mmap(NULL, sizeof(sharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shd2, 0);
	if (shm2 == MAP_FAILED){
		perror("mmap");
		exit(EXIT_FAILURE);
	}


	// inicializacion de los datos compartidos
	shm2->numServings = 0;
	shm2->numSalvajesWaiting = 0;
	shm2->numCoockers = 0;
	shm2->pos = 0;

	// inicializacion de los punteros a la memoria compartida
	data = shm2;

	// inicializacion de los semaforos
	caldero = sem_open("Caldero", O_CREAT, 0666, 1);
	if (caldero == SEM_FAILED){
		perror("sem_open");
		exit(EXIT_FAILURE);
	}
	cocinero = sem_open("Cocinero", O_CREAT, 0666, 0);
	if (cocinero == SEM_FAILED){
		perror("sem_open");
		exit(EXIT_FAILURE);
	}
	salvajes = sem_open("Salvajes", O_CREAT, 0666, 0);
	if (salvajes == SEM_FAILED){
		perror("sem_open");
		exit(EXIT_FAILURE);
	}

	cook(); // empieza a cocinar

	munmap(shm, TAM); // desvincula la memoria compartida
	munmap(shm2, sizeof(sharedData)); // desvincula la memoria compartida

	close(shd); // cierra el descriptor de la memoria compartida
	close(shd2); // cierra el descriptor de la memoria compartida

	sem_close(caldero); // cierra el semaforo del caldero
	sem_close(cocinero); // cierra el semaforo del cocinero
	sem_close(salvajes); // cierra el semaforo de los salvajes

	sem_unlink("Caldero"); // elimina el semaforo del caldero
	sem_unlink("Cocinero"); // elimina el semaforo del cocinero
	sem_unlink("Salvajes"); // elimina el semaforo de los salvajes

	shm_unlink("Caldero"); // elimina la memoria compartida
	shm_unlink("Datos"); // elimina la memoria compartida

	return 0;
}


/*
wait(caldero)
while(numServings > 0){
	++numCoockers
	signal(caldero)
	wait(cocinero)
	if(finish) return
	wait(caldero)
}

putServingsInPot(M)
numServings = M

while(numSalvajesWaiting > 0){
	--numSalvajesWaiting
	signal(salvajes)
}

signal(caldero)
*/
void putServingsSafe(int servings)
{
	sem_wait(caldero); // coge el caldero

	while(data->numServings > 0){ // si hay raciones en el caldero, se pone a la cola para cocinar
		++(data->numCoockers); // aumenta el numero de cocineros
		sem_post(caldero); // libera el caldero
		printf("Cocinero %d se va a dormir\n", getpid());
		sem_wait(cocinero); // cocinero se echa a dormir
		printf("Cocinero %d despertado\n", getpid());
		if(finish) return; // si se ha recibido la senal de finalizacion, se sale
		sem_wait(caldero); // se pone a la cola para coger el caldero
	}

	putServingsInPot(M); // pone las raciones en el caldero
	data->numServings = M; // actualiza el numero de raciones que hay en el caldero

	while(data->numSalvajesWaiting > 0){ // despierta a los salvajes que estan esperando
		sem_post(salvajes); // despierta a un salvaje
		data->numSalvajesWaiting--; // actualiza el numero de salvajes que estan esperando
	}

	data->pos = 0; // actualiza la posicion del caldero

	sem_post(caldero); // libera el caldero
}

void putServingsInPot(int servings)
{
	int i, j, posArr = 0;
	for(i = 0; i < servings; i++){
		j = rand() % M;
		printf("Cocinero %d pone %s en el caldero\n", getpid(), misionero[j]);

		strcpy(&shm[posArr], misionero[j]);
		posArr += (int)strlen(misionero[j]) + 1;
	}
}

void cook(void)
{
	while(!finish) {
		putServingsSafe(M);
	}
}

void handler(int signo)
{
	printf("\nCapturada la señal\n");
	finish = 1;
	sem_post(cocinero);
}
