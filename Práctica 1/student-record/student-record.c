#include <stdio.h>
#include <unistd.h> /* for getopt() */
#include <stdlib.h> /* for EXIT_SUCCESS, EXIT_FAILURE */
#include <string.h>
#include "defs.h"


student_t* findStudent(student_t* students, int nr_entries, int id, char NIF[]);
student_t* read_students_file(FILE* students, int* nr_entries);
student_t* parse_records(char* records[], options options);
static char* clone_string(char* original);
int dump_entries(student_t* entries, int nr_entries, FILE* students);
static void showRecords(student_t* records, int nRecords);
static void freeRecords(student_t* records, int nRecords);
char* loadstr(FILE* students);

int main(int argc, char* argv[])
{

	options options;
	int opt, returnValue = 0;

	/* Initialize default values for options */
	options.file = stdout;
	options.nRecords = 0;
	options.file = NULL;
	options.route = NULL;
	options.student_id = -1;

	/* Parse command-line options */
	while((opt = getopt(argc, argv, "hf:lcaq")) != -1) {
		switch (opt){
		case 'h':
		 	fprintf(stderr,"Usage: %s -f file [ -h | -l | -c | -a | -q [ -i|-n ID] ] ] [ list of records ]\n",argv[0]);
			exit(EXIT_SUCCESS);
			break;
		case 'f':
			options.route = optarg;
			break;
		case 'l':
			options.output_mode = LIST_MODE;
			options.openMode = "r";
			break;
		case 'a':
			options.output_mode = ADD_MODE;
			options.openMode = "r+";
			break;
		case 'c':
			options.output_mode = CREATION_MODE;
			options.openMode = "w+";
			break;
		case 'q':
			options.output_mode = QUERY_MODE;
			options.openMode = "r";
			while((opt = getopt(argc, argv, "i:n:")) != -1){
				switch (opt){
				case 'i': 
					if ((sscanf(optarg, "%d", &options.student_id) == EOF)){
						fprintf(stderr, "Couldn't parse UID %s field in Query", optarg);
						exit(EXIT_FAILURE);
					}
					break;
				case 'n': 
					if(strlen(optarg) > MAX_CHARS_NIF + 1){
						fprintf(stderr, "Couldn't parse NIF %s\n", optarg);
						exit(EXIT_FAILURE);
					}
					strcpy(options.NIF, optarg);
					break;
				default:
					break;
				}
			}

			break;
		default:
			break;
		}
	}

	if ((options.file = fopen(options.route, options.openMode)) == NULL){
		fprintf(stderr, "The file %s could not be opened: ", options.route);
		exit(EXIT_FAILURE);
	}

	switch (options.output_mode){
	case ADD_MODE: case CREATION_MODE:
		options.nRecords = argc - optind;
		options.records = parse_records(&argv[optind], options);

		int nEntries;
		if(options.output_mode == ADD_MODE && fread(&nEntries, sizeof(int), 1, options.file) == 1){
			nEntries += options.nRecords;
			fseek(options.file, -sizeof(int), SEEK_CUR);
			fwrite(&nEntries, sizeof(int), 1, options.file);
			fseek(options.file, 0, SEEK_END);
		}
		else{
			fwrite(&options.nRecords, sizeof(int), 1, options.file);
		}
		dump_entries(options.records, options.nRecords, options.file);
		break;

	case LIST_MODE: case QUERY_MODE:
		options.records = read_students_file(options.file, &options.nRecords);
		if(!options.records) exit(EXIT_FAILURE);

		if(options.output_mode == QUERY_MODE){
			student_t* stfound = NULL;
			if(options.student_id != -1){
				stfound = findStudent(options.records, options.nRecords, options.student_id, NULL);
			}
			else stfound = findStudent(options.records, options.nRecords, -1, options.NIF);

			if(stfound == NULL){
				printf("No entry was found");
				exit(EXIT_FAILURE);
			}

		}
		else showRecords(options.records, options.nRecords);

		break;
	default:
		break;
	}

	freeRecords(options.records, options.nRecords);
	fclose(options.file);
    exit(returnValue);
}



student_t* parse_records(char* records[], options options){
	student_t* stRecords;
	student_t* currRecord;
	token_id_t tokenId;
	char* token;

	stRecords = malloc(sizeof(student_t) * options.nRecords);

	for(int i = 0; i < options.nRecords; i++){

		tokenId = STUDENT_ID;
		currRecord=&stRecords[i];
		int id;

		while((token = strsep(&records[i], ":")) != NULL){
			switch (tokenId)
			{
			case STUDENT_ID:
				if(sscanf(token, "%d", &id) == EOF){
					fprintf(stderr, "Couldn't parse UID field in column %d. token is %s\n", i+1, token);
					return NULL;
				}
				for(int j = 0; j < i; j++){
					if(findStudent(stRecords, i, id, NULL) != NULL){
						fprintf(stderr, "Found duplicate student_id %d", currRecord->student_id);
						exit(EXIT_FAILURE);
					}
				}
				currRecord->student_id = id;
				break;
			case NIF:
				if(strlen(token) > MAX_CHARS_NIF + 1){
					fprintf(stderr, "Couldn't parse NIF field in column %d. token is %s\n", i+1, token);
					exit(EXIT_FAILURE);
				}
				strcpy(currRecord->NIF, token);
				break;
			case FIRST_NAME:
				currRecord->first_name = clone_string(token);
				break;
			case LAST_NAME:
				currRecord->last_name = clone_string(token);
				break;
			default:
				break;
			}
			tokenId++;
		}
	}

	if(options.output_mode == ADD_MODE){
		int nFileS;
		student_t* fileStudents;
		if ((fileStudents = read_students_file(options.file, &nFileS)) != NULL){
			for(int i = 0; i < options.nRecords; i++){
				if(findStudent(fileStudents, nFileS, stRecords[i].student_id, NULL) != NULL){
					fprintf(stderr, "Found duplicate student_id %d", stRecords[i].student_id);

					exit(EXIT_FAILURE);
				}
			}
		}
		freeRecords(fileStudents, nFileS);
	}

	return stRecords;
}

/**
 * @brief  Make a copy of existing string allocating memory accordingly
 *
 * @param original
 * @return new string that is a clone of original
 **/
static char* clone_string(char* original)
{
	char* copy;
	copy=malloc(strlen(original)+1);
	strcpy(copy,original);
	return copy;
}

student_t* findStudent(student_t* students, int nr_entries, int id, char NIF[]){
	int j = 0;
	student_t* stReturn = NULL;
	char findMode = (NIF == NULL);
	char found = 0;

	while(j < nr_entries && stReturn == NULL){
		switch (findMode){
		case 0:
			found = !strcmp(students[j].NIF, NIF);
			break;
		case 1:
			found = students[j].student_id == id;
		default:
			break;
		}

		if(found){
			stReturn = &students[j];
		}
		++j;
	}
	return stReturn;
}

static void showRecords(student_t* records, int nRecords){
	for (int i = 0; i < nRecords; i++){
		student_t* s = &records[i];
		printf("[Entry #%d]\n",i);
		printf("\tstudent_id=%d\n" "\tNIF=%s\n" "\tfirst_name=%s\n" "\tlast_name=%s\n",
			s->student_id, s->NIF, s->first_name, s->last_name);
	}
}

student_t* read_students_file(FILE* students, int* nr_entries){

	student_t* stRecords = NULL;
	student_t* currRecord;

	if (fread(nr_entries, sizeof(int), 1, students) == 1){
		stRecords = malloc(sizeof(student_t) * *nr_entries);
		for (int i = 0; i < *nr_entries; i++){
			currRecord=&stRecords[i];
			fread(&currRecord->student_id, sizeof(int), 1, students);
			strcpy(currRecord->NIF,loadstr(students));
			fread(currRecord->NIF, sizeof(char), MAX_CHARS_NIF + 1, students);
			currRecord->first_name = loadstr(students);
			currRecord->last_name = loadstr(students);
		}
		fseek(students, 0, SEEK_SET);
	}
	return stRecords;
}

char* loadstr(FILE* students){
	char* retS;
	int pos = 0;
	while(getc(students) != '\0'){
		pos++;
	}
	retS = malloc(sizeof(char) * pos + 1);
	fseek(students, -(pos + 1), SEEK_CUR);
	fread(retS, sizeof(char), pos + 1, students);

	return retS;
}

int dump_entries(student_t* entries, int nr_entries, FILE* students){
	int retValue;
	for(int i = 0; i < nr_entries; i++){
		retValue = !(fwrite(&entries[i].student_id, sizeof(int), 1, students) == 1);
		fwrite(&entries[i].NIF, sizeof(char), strlen(entries[i].NIF), students);
		fwrite("\0", sizeof(char), 1, students);
		fwrite(entries[i].first_name, sizeof(char), strlen(entries[i].first_name), students);
		fwrite("\0", sizeof(char), 1, students);
		fwrite(entries[i].last_name, sizeof(char), strlen(entries[i].last_name), students);
		fwrite("\0", sizeof(char), 1, students);
	}
	return retValue;
}

static void freeRecords(student_t* records, int nRecords){
	student_t* currStudent;
	for (int i = 0; i < nRecords; ++i){
		currStudent = &records[i];
		free(currStudent->last_name);
		free(currStudent->first_name);
	}
	free(records);
}