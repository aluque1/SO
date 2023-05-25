#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* programa que crea N hijos siguiendo un grado de dependencias. Usa las
 * versiones de execl y execlp */

int main(int argc, char **argv)
{
	pid_t p1 = fork();
	if (p1 == 0) {
		execlp("echo", "echo", "P1", NULL);
		exit(1);
	}
	waitpid(p1, NULL, 0);

	pid_t p2 = fork();
	if (p2 == 0) {
		execl("/bin/echo", "/bin/echo", "P2" , NULL);
		exit(1);
	}
	pid_t p5 = fork();
	if (p5 == 0) {
		execlp("echo", "echo", "P5", NULL);
		exit(1);
	}
	pid_t p7 = fork();
	if (p7 == 0) {
		execlp("echo", "echo", "P7", NULL);
		exit(1);
	}

	waitpid(p2, NULL, 0);

	pid_t p3 = fork();
	if (p3 == 0) {
		execlp("echo", "echo", "P3", NULL);
		exit(1);
	}
	pid_t p4 = fork();
	if (p4 == 0) {
		execl("/bin/echo", "/bin/echo", "P4" , NULL);
		exit(1);
	}

	waitpid(p4, NULL, 0);
	waitpid(p5, NULL, 0);

	pid_t p6 = fork();
	if (p6 == 0) {
		execl("/bin/echo", "/bin/echo", "P6" , NULL);
		exit(1);
	}

	waitpid(p3, NULL, 0);
	waitpid(p6, NULL, 0);
	waitpid(p7, NULL, 0);

	pid_t p8 = fork();
	if (p8 == 0) {	
		execl("/bin/echo", "/bin/echo", "P8" , NULL);
		exit(1);
	}

	waitpid(p8, NULL, 0);
	return 0;
}
