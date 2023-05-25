#ifndef DEFS_H
#define DEFS_H

#define MAX_CHARS_NIF  9


/**
 * Estructura para almacenar la informaciónd de cada estudiante
 */
typedef struct {
    int student_id; 
    char NIF[MAX_CHARS_NIF+1];  
    char* first_name;
    char* last_name;
} student_t;

/**
 * Índice de los campos en student_t
 * Información de ayuda pra simplificar la implementación del parser
 */
typedef enum {
	STUDENT_ID=0,
	NIF,
	FIRST_NAME,
	LAST_NAME
} token_id_t;

/**
 * Output modes supported by the program
 */
typedef enum {
	CREATION_MODE=0,
	ADD_MODE,
	LIST_MODE,
	QUERY_MODE,
 } output_mode_t;

/**
 * Structure to hold the "variables" associated with
 * command-line options
 */
typedef struct {
	FILE* file;
	output_mode_t output_mode;
	student_t* records;
	int nRecords;
	char* route;
	char* openMode;

	char NIF[MAX_CHARS_NIF+1];
	int student_id;
}options;

#endif