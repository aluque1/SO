#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>

void printFile(int fdo, int nBytes, bool modoE);

int main(int argc, char *argv[])
{
	int fdo;
	int opt;
	int nBytes = 0;
	bool modoE = false;

	/* Parse command-line options */
	while ((opt = getopt(argc, argv, "hf:")) != -1)
	{
		switch (opt)
		{
		case 'f':
			if ((fdo = open(optarg, O_RDONLY)) < 0){
				fprintf(stderr, "The input file %s could not be opened: ", optarg);
				perror(NULL);
				exit(EXIT_FAILURE);
			}

			while ((opt = getopt(argc, argv, "n:e")) != -1)
			{
				switch (opt)
				{
				case 'n':
					if ((sscanf(optarg, "%d", &nBytes) == EOF)){
						fprintf(stderr, "Couldn't parse n argument %s for number of bytes", optarg);
						exit(EXIT_FAILURE);
					}
					break;
				case 'e':
					modoE = true;
					break;
				default:
					break;
				}
			}
			break;
		case 'h':
			fprintf(stderr, "Usage: %s [-h | -f <input file> | -n <N> | -e ]\n", argv[0]);
			exit(0);
			break;
		default:
			exit(EXIT_FAILURE);
		}
	}

	printFile(fdo, nBytes, modoE);
	return 0;
}

void printFile(int fdo, int nBytes, bool modoE){
	if(modoE) lseek(fdo, -nBytes, SEEK_END);
	else lseek(fdo, nBytes, SEEK_SET);

	int n;
	char* c = malloc(sizeof(char));
	while ((n = read(fdo, c, 1)) > 0)
	{
		if(n < 0){
			perror("Error en la lectura");
			exit(EXIT_FAILURE);
		}

		if (write(1, c, 1) < 0){
			perror("Error en la escritura");
			exit(EXIT_FAILURE);
		}
	}
	write(1, "\n", 1);
}