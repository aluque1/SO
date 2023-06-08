#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#define CAPACITY 5
#define VIPSTR(vip) ((vip) ? "  vip  " : "not vip")
 


/*
Variables globales

int aforoActual = n
int turnoVipActual = 0
int turnoVipSigiente = 0
int vipsEsperando = 0

int turnoNormalActual = 0
int turnoNormalSiguiente = 0

mutex m

condvar v => TurnoVipActual == turnoLocal && aforoActual < N
condvar n => TurnoNormalActual == turnoLocal && vipsEsperando == 0 && aforoActual < N
*/

typedef struct {
	pthread_t id;
	bool isvip;
} client_info;

int aforoActual = 0;
int turnoVipActual = 0;
int turnoVipSiguiente = 0;
int vipsEsperando = 0;

int turnoNormalActual = 0;
int turnoNormalSiguiente = 0;

pthread_mutex_t m;
pthread_cond_t v;
pthread_cond_t n;

void enter_normal_client(int id);
void enter_vip_client(int id);
void disco_exit(int id, int isvip);
void *client(void *arg);
client_info* parseClients(char* filename, unsigned int* nThreads);

int main(int argc, char *argv[])
{
	unsigned int nThreads, i;
	void *res;
	pthread_attr_t attr;
	client_info *cInfo = NULL;

	if (argc != 2)
	{
		printf("Usage: %s <clients file>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	cInfo = parseClients(argv[1], &nThreads);

	if (pthread_attr_init(&attr))
	{
		perror("Thread attribute could not be created\n");
		exit(EXIT_FAILURE);
	}

	pthread_mutex_init(&m, NULL);
	pthread_cond_init(&v, NULL);
	pthread_cond_init(&n, NULL);

	for (i = 0; i < nThreads; ++i)
	{
		if (pthread_create(&cInfo[i].id, &attr, client, &cInfo[i]))
		{
			perror("Error creating thread");
			exit(EXIT_FAILURE);
		}
	}

	for (i = 0; i < nThreads; ++i)
	{
		if (pthread_join(cInfo[i].id, &res))
			perror("Error joining thread\n");

		free(res);
	}

	if (pthread_attr_destroy(&attr))
	{
		perror("Error destroying attr");
		exit(EXIT_FAILURE);
	}

	free(cInfo);

	return EXIT_SUCCESS;
}


client_info* parseClients(char* filename, unsigned int* nThreads)
{
	FILE* file;
	client_info *cInfo;
	unsigned int i;

	file = fopen(filename, "r");
	if (file == NULL)
	{
		perror("Error opening file");
		exit(EXIT_FAILURE);
	}

	if (fscanf(file, "%u", nThreads) == EOF)
	{
		perror("Error reading file");
		exit(EXIT_FAILURE);
	}

	cInfo = malloc(sizeof(client_info) * *nThreads);
	int isvip;
	for (i = 0; i < *nThreads; ++i)
	{
		if (fscanf(file, "%d", &isvip) == EOF)
		{
			perror("Error reading clients");
			exit(EXIT_FAILURE);
		}
		cInfo[i].isvip = isvip;
	}

	fclose(file);
	return cInfo;
}

/*
int turno
lock(m)
turno = turnoNormalSiguiente++;
while(turno != turnoNormalActual || vipsEsperando > 0 || aforoActual >= N){
	cond_wait(n, m);
}

turnoNormalActual++;
aforoActual++;
cond_broadcast(n);
cond_broadcast(v);
unlock(m);
*/
void enter_normal_client(int id)
{
	int turno;
	pthread_mutex_lock(&m);
	turno = turnoNormalSiguiente++;
	while(turno != turnoNormalActual || vipsEsperando > 0 || aforoActual >= CAPACITY){
		pthread_cond_wait(&n, &m);
	}

	printf("Client %d (%s) enters the disco\n", id, VIPSTR(false));

	turnoNormalActual++;
	aforoActual++;
	pthread_cond_broadcast(&n);
	pthread_cond_broadcast(&v);
	pthread_mutex_unlock(&m);
}

/*
int turno
lock(m)
turno = turnoVipSiguiente++;
vipsEsperando++;
while(turno != TurnoVipActual || aforoActual >= N){
	cond_wait(v,m);
}

turnoVipActual++;
aforoActual++;
vipsEsperando--;
cond_broadcast(n);
cond_broadcast(v);
unlock(m):
*/
void enter_vip_client(int id)
{
	int turno;
	pthread_mutex_lock(&m);
	turno = turnoVipSiguiente++;
	vipsEsperando++;
	while(turno != turnoVipActual || aforoActual >= CAPACITY){
		pthread_cond_wait(&v, &m);
	}

	printf("Client %d (%s) enters the disco\n", id, VIPSTR(true));

	turnoVipActual++;
	aforoActual++;
	vipsEsperando--;
	pthread_cond_broadcast(&n);
	pthread_cond_broadcast(&v);
	pthread_mutex_unlock(&m);
}

void dance(int id, int isvip)
{
	printf("Client %2d (%s) dancing in disco\n", id, VIPSTR(isvip));
	sleep(((unsigned) rand() % 3) + 1);
}

/*
lock(m)
aforoActual--;
if(vipsEsperando > 0)
cond_broadcast(V);

else
cond_broadcast(N);
unlock(m);
*/
void disco_exit(int id, int isvip)
{
	pthread_mutex_lock(&m);
	aforoActual--;

	printf("Client %d (%s) leaving disco\n", id, VIPSTR(isvip));

	if(vipsEsperando > 0){ 
		pthread_cond_broadcast(&v);
	}
	else{
		pthread_cond_broadcast(&n);
	}

	pthread_mutex_unlock(&m);
}

void *client(void *arg)
{
	client_info *info = (client_info*)arg;
	int id = (int)info->id;
	bool isvip = info->isvip;

	if(isvip){
		enter_vip_client(id);
	}
	else{
		enter_normal_client(id);
	}

	dance(id, isvip);
	disco_exit(id, isvip);

	return NULL;
}

