//
// Created by danya on 12.11.16.
//


FILE* initProc(char* filename){
    FILE* fin = fopen(filename, "r");
    return fin;
}

void executeNextCommand(FILE* fin){
    
}



int main(int argc, char *argv[]){
    FILE* program;
    if(!(argc > 1 || program = initProc(argv[0]))){
        printf("Invalid file");
        exit(1);
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