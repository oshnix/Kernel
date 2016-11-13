#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include "errors.h"
#include "operators.h"
#include "interpretator.h"

char executeNextCommand(interpretator_state *state);
char goToLabel(interpretator_state *state);

extern scheduler_flag;


int isVariable(char *word, essence *list){
    for(int i = 0; i < list->essenceCount; i++){
        if(!strcmp(word, list->essenceNames[i])){
            return i;
        }
    }
    return -1;
}

char* strparse(char *dest, char *res){
    static const char delims[] = {' ', '\t', '\n', '\0'};
    int i, count = sizeof(delims);
    char state = 1;
    while(*(res) == ' ' || *(res) == '\t') ++res;
    while(state){
        *(dest++) = *(res++);
        for (i = 0; i < count; i++) {
            if(*res == delims[i]){
                state = 0;
                ++res;
                *dest = '\0';
                break;
            }
        }
    }
    return res;
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

char interpretateNextWord(interpretator_state *state){
    int variableIndex;
    if(!strcmp(state->word, "if")){
        state->buffer = strparse(state->word, state->buffer);
        int firstOperand = readVarNum(state->word, &state->variables);
        state->buffer = strparse(state->operand, state->buffer);
        state->buffer = strparse(state->word, state->buffer);
        int secondOperand = readVarNum(state->word, &state->variables);
        char result;
        if(comparations(firstOperand, secondOperand, state->operand, &result)){
            if(result){
                state->buffer = strparse(state->word, state->buffer);
                if(strcmp(state->word, "goto") == 0){
                    return goToLabel(state);
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


    } else if(strcmp(state->word, "goto") == 0){
        return goToLabel(state);

    } else if(strcmp(state->word, "print")== 0){
        state->buffer = strparse(state->word, state->buffer);
        variableIndex = isVariable(state->word, &state->variables);
        if(variableIndex != -1){
            printf("%d\n", state->variables.essenceValues[variableIndex]);
        }
        else{
            printf("%s\n", state->word);
        }
        return ALL_OK;

    } else if(strcmp(state->word, "end") == 0) {
        printf("EXIT\n");
        return 0;

    } else if(strcmp(state->word, "read") == 0) {

    } else if(strcmp(state->word, "write") == 0) {

    } else if(strcmp(state->word, "cd") == 0) {

    } else if(strcmp(state->word, "kill") == 0) {

    } else if(state->word[strlen(state->word) - 1] == ':'){
        state->position += strlen(state->word)+1;
        fseek(state->program, state->position, SEEK_SET);
        return executeNextCommand(state);
    } else{
        int variableIndex = isVariable(state->word, &state->variables);
        if(variableIndex == -1) variableIndex = addNewElement(state->word, &state->variables);
        state->buffer = strparse(state->word, state->buffer);
        if(state->word[0] != '='){
            return SHIT_HAPPENED;
        }
        state->buffer = strparse(state->word, state->buffer);
        int firstOperand = readVarNum(state->word, &state->variables);
        state->buffer = strparse(state->word, state->buffer);
        state->buffer = strparse(state->operand, state->buffer);
        if(!state->operand == NULL){
            state->buffer = strparse(state->word, state->buffer);
            int secondOperand = readVarNum(state->word, &state->variables);
            return Operation(firstOperand, secondOperand, state->operand, &state->variables.essenceValues[variableIndex]);
        }
        else{
            state->variables.essenceValues[variableIndex] = firstOperand;
            return ALL_OK;
        }
    }
    return SHIT_HAPPENED;
}

char goToLabel(interpretator_state *state){
    state->buffer = strparse(state->word, state->buffer);
    int variableIndex = isVariable(state->word, &state->labels);
    if(variableIndex == -1){
        return SHIT_HAPPENED;
    }
    int position = state->labels.essenceValues[variableIndex];
    fseek(state->program, position, SEEK_SET);
    return ALL_OK;
}

void fillLabels(FILE *fin, char *buffer, char *word, essence *labels){
    size_t len = 0;
    while(1){
        int position = ftell(fin);
        getline(&buffer, &len, fin);
        buffer = strparse(word, buffer);
        printf("%s %s\n", buffer, word);
        if(word[strlen(word) - 1] == ':'){
            addLabel(labels, word, position, fin);
        }
        if(strcmp(word, "end") == 0){
            return;
        }
    }
}

char executeNextCommand(interpretator_state *state) {
    char *buffer;
    size_t len = 0;
    getline(&buffer, &len, state->program);
    state->buffer = buffer;
    state->position = ftell(state->program);
    state->buffer = strparse(state->word, state->buffer);
    //printf("Word: %s\n", word);
    char ret = interpretateNextWord(state);
    //free(buffer);
    //printf("Error code: %i\n", (int)ret);
    free(buffer);
    return ret;
}

interpretator_state initInterpretator(char* file, int pid) {
	interpretator_state state;
    if(!(state.program = initProc(file))){
        return state;
    }
    state.buffer = malloc(256);
    state.operand = malloc(OPERAND_MAX_SIZE);
    state.word = malloc(ESSENCE_NAME_SIZE + 2);
    state.variables = essenceInit();
    state.labels = essenceInit();
    state.position = ftell(state.program);
    state.pid = pid;
    fillLabels(state.program, state.buffer, state.word, &state.labels);
    rewind(state.program);
    return state;
}

int launchInterpretator(interpretator_state *state) {
	char errorCode;
    while(errorCode = executeNextCommand(state)) {
		if(scheduler_flag) {
			interrupt_handler(state);
			return errorCode;
		}
	}
	//printf("launchInterpretator: %i\n", (int)errorCode);
    return errorCode;
}
