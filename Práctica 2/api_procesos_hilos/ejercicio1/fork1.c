#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

/**
 * 	Programa que crea dos hijos: uno no cambia de ejecutable y otro si
 * 	el programa recibe como paremetros el nombre del ejecutable que deberia ejecutrar y los argumentos que necesite para exe 
 *	@param argc número de argumentos
 *	@param argv vector de argumentos
 *	@return EXIT_SUCCESS si no ha habido errores, EXIT_FAILURE en caso contrario
*/
int main(int argc, char *argv[])
{
	int status;
	pid_t pid;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <cmd> <arg1> <arg2> ...\n", argv[0]);
		return EXIT_FAILURE;
	}

	if ((pid = fork()) != 0) // Create a child
	{
		// 1st parent
		waitpid(pid, &status, 0);
		printf("My PID is %d and I am a father, with a child with PID: %d\n", getpid(), (int)pid);
		if ((pid = fork()) != 0){ // 2nd parent
			waitpid(pid, &status, 0);
			printf("My PID is %d and I am a father, with a child with PID: %d\n", getpid(), (int)pid);
		} else if (pid == 0){ // 2nd child
			printf("My PID is %d and I am a child, my father has PID: %d \n", (int)getpid(), (int)getppid());
			execvp(argv[1], &argv[2]);
		} else {
			perror("Fork not created");
			return EXIT_FAILURE;
		}
		
	} else if (pid == 0){ // 1st child
		printf("My PID is %d and I am a child, my father has PID: %d \n", (int)getpid(), (int)getppid());
	} else {
		perror("Fork not created");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
