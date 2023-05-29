#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

int children_pid;

/*programa que temporiza la ejecución de un proceso hijo */

/**
 * @brief Función que se encarga de la temporización del proceso hijo
 * @param sig Señal que se recibe
 * @return void
 */
void temporizador(int sig)
{
	kill(children_pid, SIGTERM);
}

/**
 * @brief Programan recibe una orden y sus argumentos, crea un proceso hijo que usa execvp
 * el proceso padre
 * @param argc Número de argumentos
 * @param argv Argumentos
 * @return int
 */
int main(int argc, char **argv)
{
	int status;
	struct sigaction sa;

	if (argc < 2)
	{
		printf("Error en el número de argumentos\n");
		exit(-1);
	}

	children_pid = fork();

	if (children_pid == 0) // Proceso hijo
	{
		execvp(argv[1], argv + 1);
	}
	else if (children_pid > 0) // Proceso padre
	{
		sa.sa_handler = temporizador;
		sa.sa_flags = SA_RESTART;
		sigaction(SIGALRM, &sa, NULL);
		alarm(5);
		wait(&status);
		if (WIFEXITED(status))
		{
			printf("El proceso hijo ha finalizado con exito\n");
		}
		else if (WIFSIGNALED(status))
		{
			printf("El proceso hijo ha finalizado por una señal\n");
		}
	}
	else // Error
	{
		printf("Error al crear el proceso hijo\n");
		exit(EXIT_FAILURE);
	}
	return 0;
}
