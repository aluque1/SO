#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

typedef struct
{
	pthread_t id;
	int userNum;
	char priority;
} thread_info;

void *thread_usuario(void *arg)
{
	thread_info *tinfo = arg;
	pthread_t id = tinfo->id;
	int userNum = tinfo->userNum;
	char priority = tinfo->priority;
	printf("Hello, my thread id is %lu, my userNumer is %d, and my priority is %c\n", id, userNum, priority);
	return NULL;
}

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
	}

	nThreads = argc - 1;
	tInfo = malloc(sizeof(thread_info) * nThreads);

	for (i = 0; i < nThreads; ++i)
	{
		if (sscanf(argv[i + 1], "%d", &tInfo[i].userNum) == EOF)
		{
			printf("Argument: %s could not be read", argv[i + 1]);
			exit(EXIT_FAILURE);
		}

		tInfo[i].priority = (tInfo[i].userNum % 2) ? 'P' : 'N';
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

	for (i = 0; i < nThreads; ++i)
	{
		if (pthread_join(tInfo[i].id, &res))
			perror("Error joining thread\n");

		free(res);
	}

	free(tInfo);

	return EXIT_SUCCESS;
}
