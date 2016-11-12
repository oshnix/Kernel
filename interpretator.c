#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#define START_ESSENCE_SIZE 8
#define ESSENCE_NAME_SIZE 50
#define SIZE_OF_OPERATION 3

typedef struct {
    int *essenceValues;
    int essenceCount;
    int essenceLimit;
    char **essenceNames;
}essence;



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

int  arithmeticOperation(int first, int second, char *operand){
    if(operand[0] == '+'){
        return first + second;

    } else if(operand[0] == '-'){
        return first - second;

    } else if(operand[0] == '*'){
        return first*second;

    } else if(operand[0] == '/'){
        return first/second;

    } else if(operand[0] == '%'){
        return first%second;

    } else if(strcmp(operand, "and")){
        return first & second;

    } else if(strcmp(operand, "or")){
        return first | second;

    } else{
        exit(1);
    }


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

void interpretateNextWord(char *word, char *buffer, essence *variables, essence *labels){
    int variableIndex;

    if(!strcmp(word, "if")){

    } else if(strcmp(word, "goto")== 0){


    } else if(strcmp(word, "print")== 0){
        word = strtok(NULL, " \n");
        variableIndex = isVariable(word, variables);
        if(variableIndex != -1){
            printf("%d\n", variables->essenceValues[variableIndex]);
        }
        else{
            printf("%s\n", word);
        }


    } else if(strcmp(word, "end") == 0){
        printf("EXIT\n");
        exit(0);

    }else if(word[strlen(word) - 1] == ':'){
        //int label = addNewLabel();

    } else{
        int variableIndex = isVariable(word, variables);
        if(variableIndex == -1) variableIndex = addNewElement(word, variables);
        word = strtok(NULL, " ");
        if(word[0] != '='){
            exit(1);
        }
        word = strtok(NULL, " ");
        int firstOperand = readVarNum(word, variables);
        char *operand = strtok(NULL, " \0\n");
        if(!operand == NULL){
            word = word = strtok(NULL, " \n");
            int secondOperand = readVarNum(word, variables);
            variables->essenceValues[variableIndex] = arithmeticOperation(firstOperand, secondOperand, operand);
        }
        else{
            variables->essenceValues[variableIndex] = firstOperand;
        }
    }

}

void executeNextCommand(FILE* fin, char *buffer, essence *variables, essence *labels){
    size_t len = 0;
    int a = getline(&buffer, &len, fin);
    char *word = malloc(ESSENCE_NAME_SIZE + 2);
    word = strtok(buffer, " \n\0");
    interpretateNextWord(word, buffer, variables, labels);
}

int main(int argc, char *argv[]){
    FILE* program;
    if(!(argc > 1) || !(program = initProc(argv[1]))){
        printf("Invalid file");
        exit(1);
    }
    char *buffer = malloc(256);
    essence variables = essenceInit(), labels = essenceInit();
    while(1){
        executeNextCommand(program, buffer, &variables, &labels);
    }
     /**/
}