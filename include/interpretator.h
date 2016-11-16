#ifndef _INTERPRETATOR_H_ 
#define _INTERPRETATOR_H_

#include <unistd.h>
#include "filesystem.h"
#include <sys/poll.h>

#define START_ESSENCE_SIZE 8
#define ESSENCE_NAME_SIZE 500
#define SIZE_OF_OPERATION 3
#define OPERAND_MAX_SIZE 6

typedef struct {
    int *essenceValues;
    int essenceCount;
    int essenceLimit;
    char **essenceNames;
}essence;

typedef enum {
	PROC_RUNNING,
	PROC_STOPPED,
	PROC_BLOCKING_IO,
	PROC_KILLED,
    PROC_INCORRECT
} proc_status;

typedef struct {
	file* program;
	char *buffer;
	char *word;
	char *operand;
	essence variables;
	essence labels;
	int position;
	file* working_directory;
	// proc
	int pid;
	proc_status status;
	char* name;
	struct pollfd fds[2];
} interpretator_state;

interpretator_state initInterpretator(char* filename, int pid, file *workDirectory);

void fillLabels(interpretator_state *state);

int launchInterpretator(interpretator_state* state);

#endif
