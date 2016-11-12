#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include "errors.h"
#include "operators.h"

#define START_ESSENCE_SIZE 8
#define ESSENCE_NAME_SIZE 50
#define SIZE_OF_OPERATION 3

typedef struct {
    int *essenceValues;
    int essenceCount;
    int essenceLimit;
    char **essenceNames;
}essence;

char executeNextCommand(FILE* fin, char *buffer, essence *variables, essence *labels);
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

char interpretateNextWord(char *word, char *buffer, essence *variables, essence *labels, int position, FILE* fin){
    int variableIndex;
    if(!strcmp(word, "if")){
        word = strtok(NULL, " \n");
        int firstOperand = readVarNum(word, variables);
        char *operand = strtok(NULL, " \0\n");
        word = word = strtok(NULL, " \n");
        int secondOperand = readVarNum(word, variables);
        char result;
        if(comparations(firstOperand, secondOperand, operand, &result)){
            if(result){
                word = strtok(NULL, " \n");
                if(strcmp(word, "goto") == 0){
                    return goToLabel(labels, fin);
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
        return goToLabel(labels, fin);

    } else if(strcmp(word, "print")== 0){
        word = strtok(NULL, " \n");
        variableIndex = isVariable(word, variables);
        if(variableIndex != -1){
            printf("%d\n", variables->essenceValues[variableIndex]);
        }
        else{
            printf("%s\n", word);
        }


    } else if(strcmp(word, "end") == 0) {
        printf("EXIT\n");
        return 0;

    } else if(strcmp(word, "read") == 0) {

    } else if(strcmp(word, "write") == 0) {

    } else if(strcmp(word, "cd") == 0) {

    } else if(strcmp(word, "kill") == 0) {

    } else if(word[strlen(word) - 1] == ':'){
        position += strlen(word)+1;
        fseek(fin, position, SEEK_SET);
        return executeNextCommand(fin, buffer, variables,labels);
    } else{
        int variableIndex = isVariable(word, variables);
        if(variableIndex == -1) variableIndex = addNewElement(word, variables);
        word = strtok(NULL, " ");
        if(word[0] != '='){
            return SHIT_HAPPENED;
        }
        word = strtok(NULL, " ");
        int firstOperand = readVarNum(word, variables);
        char *operand = strtok(NULL, " \0\n");
        if(!operand == NULL){
            word = strtok(NULL, " \n");
            int secondOperand = readVarNum(word, variables);
            return Operation(firstOperand, secondOperand, operand, &variables->essenceValues[variableIndex]);
        }
        else{
            variables->essenceValues[variableIndex] = firstOperand;
        }
    }

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
        int a = getline(&buffer, &len, fin);
        word = strtok(buffer, " \n\0");
        if(word[strlen(word) - 1] == ':'){
            addLabel(labels, word, position, fin);
        }
        if(strcmp(word, "end") == 0){
            return;
        }
    }
}

char executeNextCommand(FILE* fin, char *buffer, essence *variables, essence *labels){
    size_t len = 0;
    int position = ftell(fin);
    getline(&buffer, &len, fin);
    char *word = malloc(ESSENCE_NAME_SIZE + 2);
    word = strtok(buffer, " \n\0");
    return interpretateNextWord(word, buffer, variables, labels, position, fin);
}

int main(int argc, char *argv[]){
    FILE* program;
    if(!(argc > 1) || !(program = initProc(argv[1]))){
        printf("Invalid file");
        exit(1);
    }
    char *buffer = malloc(256);
    essence variables = essenceInit(), labels = essenceInit();
    char errorCode;
    fillLabels(program, buffer, &labels);
    rewind(program);
    while(errorCode = executeNextCommand(program, buffer, &variables, &labels));
    return errorCode;
}