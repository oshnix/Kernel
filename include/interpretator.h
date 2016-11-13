#ifndef _INTERPRETATOR_H_ 
#define _INTERPRETATOR_H_

#define START_ESSENCE_SIZE 8
#define ESSENCE_NAME_SIZE 50
#define SIZE_OF_OPERATION 3

typedef struct {
    int *essenceValues;
    int essenceCount;
    int essenceLimit;
    char **essenceNames;
}essence;

typedef struct {
	FILE* program;
	char *buffer;
	essence variables;
	essence labels;
	int position;
	
	int pid;
} interpretator_state;

interpretator_state initInterpretator(char* file, int pid);

int launchInterpretator(interpretator_state* state);

#endif
