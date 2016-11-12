#include <stdio.h>
#include <malloc.h>
#include <stdlib.h1>

char *buffer;
char *variables[];
int *variableValues;
int variablesCount;


void readWord(char* word){
    while(*buffer && *buffer != ' ' && *buffer != '\n') *(word++) = *(buffer++);
    *(word) = '\0';
}

int strlen(char *str){
    int res = 0;
    while(*(str++)) res++;
    return res;
}

void strcopy(char *src, char *res){
    while(*(res++) = *(src++));
}

int addNewVariable(char *word){
    variables[variablesCount] = malloc(strlen(word)+1);
    strcopy(word, variables[variablesCount]);
    return variablesCount++;
}

char equal(char *left, char *right){
    while(*left != '\0' && *right != '\0' && *(left++) == *(right++));
    if(*left == '\0' && *right == '\0') return 1;
    else return 0;
}

void interpretateNextWord(char *word){
    if(equal(word, "if")){

    } else if(equal(word, "goto")){

    } else if(equal(word, "print")){

    } else if(equal(word, "end")){

    } else{
        int a = isVariable(word);
        if(a == -1) a = addNewVariable(word);
    }

}

int isVariable(char *word){
    for(int i = 0; i < variablesCount; i++){
        if(equal(word, variables[i])){
            return i;
        }
    }
    return -1;
}


FILE* initProc(char* filename){
    FILE* fin = fopen(filename, "r");
    return fin;
}

void executeNextCommand(FILE* fin){
    size_t len = 0;
    getline(&buffer, &len, fin);
    char *word = malloc(51);
    readWord(word);
    interpretateNextWord(word);



}



int main(int argc, char *argv[]){
    FILE* program;
    if(!(argc > 1 || program = initProc(argv[0]))){
        printf("Invalid file");
        exit(1);
    }
    else{
        buffer = malloc(256);

    }

    while(1){
        executeNextCommand(program);
    }
    //init
    /*while(true){
        execute next command();
     */
    return 0;
}