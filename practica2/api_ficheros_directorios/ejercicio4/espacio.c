#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

int get_size_dir(char *fname, size_t *blocks);
int get_size(char *fname, size_t *blocks);

/**
 * Processes all the files in the command line calling get_size on them to
 * obtain the number of 512 B blocks they occupy and prints the total size in
 * kilobytes on the standard output
 */
int main(int argc, char *argv[]){
	size_t blocks = 0;
	int i = 1;
	while (i < argc)
	{
		get_size(argv[i], &blocks);
		printf("%lu%s %s \n", blocks/2,"K", argv[i]);
		++i;
		blocks = 0;
	}
	return 0;
}

/**
 * Gets in the blocks buffer the size of file fname using lstat. If fname is a
 * directory get_size_dir is called to add the size of its contents.
 *
 * @param fname name of the file to get the size of
 * @param blocks pointer to the buffer where the size will be stored
 * @return EXIT_SUCCESS if no errors occurred, EXIT_FAILURE otherwise
 */
int get_size(char *fname, size_t *blocks){
	struct stat *stat_buff = malloc(sizeof(struct stat));
	lstat(fname, stat_buff);
	
	if (S_ISDIR(stat_buff->st_mode) == 1){
		get_size_dir(fname, blocks);
	} 
	
	*blocks += stat_buff->st_blocks;

	return EXIT_SUCCESS;
}

/**
 * Gets the total number of blocks occupied by all the files in a directory. If
 * a contained file is a directory a recursive call to get_size_dir is
 * performed. Entries . and .. are conveniently ignored.
 * 
 * @param dname name of the directory to get the size of
 * @param blocks pointer to the buffer where the size will be stored
 * @return EXIT_SUCCESS if no errors occurred, EXIT_FAILURE otherwise
 */
int get_size_dir(char *dname, size_t *blocks){
	DIR *dir;
	char* name;
	struct dirent *buff = malloc(sizeof(struct dirent));

	if ((dir = opendir(dname)) != NULL) {
		while ((buff = readdir(dir)) != NULL){ // This is in charge of reading the whole directory
			name = buff->d_name;
			if (strcmp(name,".") & strcmp(name,".."))
				get_size(name, blocks);
		}	
		closedir(dir);
		return EXIT_SUCCESS;
	} else {
		perror("");
		return EXIT_FAILURE;
	}
}