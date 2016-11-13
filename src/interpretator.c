#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include "errors.h"
#include "operators.h"
#include "interpretator.h"

char executeNextCommand(interpretator_state *state);
char goToLabel(essence *labels, FILE *fin);



int isVariable(char *word, essence *list){
    for(int i = 0; i < list->essenceCount; i++){
        if(!strcmp(word, list->essenceNames[i])){
            return i;
        }
    }
    return -1;
}


FILE* initProc(char* filename){
    FILE *fin;
    fin = fopen(filename, "r");
    return fin;
}

int comparations(int first, int second, char *operand, char *result){
    switch (operand[0]){
        case '>':
            *result = operand[1] == '=' ? first >= second : first > second;
            break;
        case '<':
            *result = operand[1] == '=' ? first <= second : first < second;
            break;
        default:{
            if(strcmp(operand, "==")){
                *result = first == second;
            } else if(strcmp(operand, "!=")){
                *result = first != second;
            } else {
                return SHIT_HAPPENED;
            }
        }
    }
    return ALL_OK;
}

essence essenceInit(){
    essence newEssence;
    newEssence.essenceCount = 0;
    newEssence.essenceLimit = START_ESSENCE_SIZE;
    newEssence.essenceNames = malloc(START_ESSENCE_SIZE * sizeof(void*));
    newEssence.essenceValues = malloc(START_ESSENCE_SIZE * sizeof(int));
    return newEssence;
}

int addNewElement(char *word, essence *list){
    list->essenceNames[list->essenceCount] = malloc(ESSENCE_NAME_SIZE + 1);
    strcpy(list->essenceNames[list->essenceCount], word);
    ++list->essenceCount;
    if(list->essenceCount == list->essenceLimit){
        list->essenceLimit *= 2;
        list->essenceNames = realloc(list->essenceNames, list->essenceLimit* sizeof(void*));
        list->essenceValues = realloc(list->essenceValues, list->essenceLimit * sizeof(int));
    }
    return list->essenceCount -1 ;
}

int readVarNum(char *word, essence *variables){
    int result = isVariable(word, variables);
    if(result == -1){
        return strtol(word, '\0', 10);
    }
    result = variables->essenceValues[result];
    return result;
}

void addLabel(essence *labels, char *word, int position, FILE *fin){
    position += strlen(word)+1;
    word[strlen(word) - 1] = '\0';
    int label = addNewElement(word, labels);
    labels->essenceValues[label] = position;
    fseek(fin, position, SEEK_SET);
}

char interpretateNextWord(char *word, interpretator_state *state){
    int variableIndex;
    if(!strcmp(word, "if")){
		
        word = strtok(NULL, " \n");
        int firstOperand = readVarNum(word, &state->variables);
        char *operand = strtok(NULL, " \0\n");
        word = strtok(NULL, " \n");
        int secondOperand = readVarNum(word, &state->variables);
        char result;
        if(comparations(firstOperand, secondOperand, operand, &result)){
            if(result){
                word = strtok(NULL, " \n");
                if(strcmp(word, "goto") == 0){
                    return goToLabel(&state->labels, state->program);
                }
                return SHIT_HAPPENED;
            }
            else{
                return ALL_OK;
            }
        }
        else{
            return SHIT_HAPPENED;
        }


    } else if(strcmp(word, "goto") == 0){
        return goToLabel(&state->labels, state->program);

    } else if(strcmp(word, "print")== 0){
        word = strtok(NULL, " \n");
        variableIndex = isVariable(word, &state->variables);
        if(variableIndex != -1){
            printf("%d\n", state->variables.essenceValues[variableIndex]);
        }
        else{
            printf("%s\n", word);
        }
        return ALL_OK;

    } else if(strcmp(word, "end") == 0) {
        printf("EXIT\n");
        return 0;

    } else if(strcmp(word, "read") == 0) {

    } else if(strcmp(word, "write") == 0) {

    } else if(strcmp(word, "cd") == 0) {

    } else if(strcmp(word, "kill") == 0) {

    } else if(word[strlen(word) - 1] == ':'){
        state->position += strlen(word)+1;
        fseek(state->program, state->position, SEEK_SET);
        return executeNextCommand(state);
    } else{
        int variableIndex = isVariable(word, &state->variables);
        if(variableIndex == -1) variableIndex = addNewElement(word, &state->variables);
        word = strtok(NULL, " ");
        if(word[0] != '='){
            return SHIT_HAPPENED;
        }
        word = strtok(NULL, " ");
        int firstOperand = readVarNum(word, &state->variables);
        char *operand = strtok(NULL, " \0\n");
        if(!operand == NULL){
            word = strtok(NULL, " \n");
            int secondOperand = readVarNum(word, &state->variables);
            return Operation(firstOperand, secondOperand, operand, &state->variables.essenceValues[variableIndex]);;
        }
        else{
            state->variables.essenceValues[variableIndex] = firstOperand;
            return ALL_OK;
        }
    }
    return SHIT_HAPPENED;
}

char goToLabel(essence *labels, FILE *fin){
    char *word = strtok(NULL, " \n");
    int variableIndex = isVariable(word, labels);
    if(variableIndex == -1){
        return 0;
    }
    int position = labels->essenceValues[variableIndex];
    fseek(fin, position, SEEK_SET);
    return 1;
}

void fillLabels(FILE *fin, char *buffer, essence *labels){
    char *word = "\0";
    size_t len = 0;
    while(1){
        int position = ftell(fin);
        getline(&buffer, &len, fin);
        word = strtok(buffer, " \n\0");
        if(word[strlen(word) - 1] == ':'){
            addLabel(labels, word, position, fin);
        }
        if(strcmp(word, "end") == 0){
            return;
        }
    }
}

char executeNextCommand(interpretator_state *state) {
    char *buffer = malloc(256);
    size_t len = 0;
    getline(&buffer, &len, state->program);
    state->position = ftell(state->program);
    char *word = strtok(buffer, " \n\0");
    printf("Word: %s\n", word);
    char ret = interpretateNextWord(word, state);
    free(buffer);
    printf("Error code: %i\n", (int)ret);
    return ret;
}

interpretator_state initInterpretator(char* file, int pid) {
	interpretator_state state;
    if(!(state.program = initProc(file))){
        return state;
    }
    state.buffer = malloc(256);
    state.variables = essenceInit();
    state.labels = essenceInit();
    state.position = ftell(state.program);
    state.pid = pid;
    fillLabels(state.program, state.buffer, &state.labels);
    rewind(state.program);
    return state;
}

int launchInterpretator(interpretator_state *state) {
	char errorCode;
    while(errorCode = executeNextCommand(state)) {
		interrupt_handler(state);
	}
	printf("launchInterpretator: %i\n", (int)errorCode);
    return errorCode;
}
