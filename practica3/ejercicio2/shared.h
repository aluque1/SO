#ifndef SHARED_H
#define SHARED_H

typedef struct {
	int numServings; // numero de raciones que hay en el caldero
	int numSalvajesWaiting; // numero de salvajes que estan esperando a que el cocinero cocine
	int numCoockers; // indica el numero de cocineros que estan cocinando
	int pos; //indica la posicion de lectura del buffer
} sharedData;

#endif