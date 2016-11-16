#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/poll.h>
#include <stdbool.h>
#include "scheduler.h"
#include "errors.h"
#include "operators.h"
#include "interpretator.h"
#include "syscalls.h"

char executeNextCommand(interpretator_state *state);
char goToLabel(interpretator_state *state);

extern int scheduler_flag;
extern size_t proc_foreground;


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
                *dest = '\0';
                break;
            }
        }
    }
    return res;
}


file* initProc(char* filename, file* workDirectory){
    record* rec;
    int a = find_record(&filename, workDirectory, &rec);
    if(a == FILE_ALLREADY_EXISTS) {
        return rec->current;
    }
    return  NULL;
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

void addLabel(interpretator_state *state){
    state->word[strlen(state->word) - 1] = 0;
    int label = addNewElement(state->word, &state->labels);
    state->labels.essenceValues[label] = state->position;
}

char nonSyscalls(interpretator_state *state){
    if(!strcmp(state->word, "if")) {
        state->buffer = strparse(state->word, state->buffer);
        int firstOperand = readVarNum(state->word, &state->variables);
        state->buffer = strparse(state->operand, state->buffer);
        state->buffer = strparse(state->word, state->buffer);
        int secondOperand = readVarNum(state->word, &state->variables);
        char result;
        if (comparations(firstOperand, secondOperand, state->operand, &result)) {
            if (result) {
                state->buffer = strparse(state->word, state->buffer);
                if (strcmp(state->word, "goto") == 0) {
                    return goToLabel(state);
                }
                return SHIT_HAPPENED;
            } else {
                return ALL_OK;
            }
        } else {
            return SHIT_HAPPENED;
        }
    } else if(strcmp(state->word, "goto") == 0){
        return goToLabel(state);

    }
    else if(state->word[strlen(state->word) - 1] == ':'){
        state->position += strlen(state->word)+1;
        return executeNextCommand(state);
    }
    else{
        return -1;
    }
}



char interpretateNextWord(interpretator_state *state) {
    int variableIndex;
    file *buffer_file;
    char error_code;
    //printf("Working with: %s\n", state->buffer);
    if (state->pid == 0 || -1 == (error_code = nonSyscalls(state))) {
        if (strcmp(state->word, "print") == 0) {
            state->buffer = strparse(state->word, state->buffer);
            variableIndex = isVariable(state->word, &state->variables);
            if (variableIndex != -1) {
                printf("%d\n", state->variables.essenceValues[variableIndex]);
            } else {
                printf("%s\n", state->word);
            }
            return ALL_OK;

        } else if (strcmp(state->word, "end") == 0) {

			printf("Process %s exits\n", state->name);
			syscalls_kill(state->pid);
            return 2;

        } else if (strcmp(state->word, "read") == 0) {
			return ALL_OK;
        } else if (strcmp(state->word, "fg") == 0) {
			long pid = -1;
			char* endptr;
			state->buffer = strparse(state->word, state->buffer);
			pid = strtol(state->word, &endptr, 10);
			syscalls_fg(pid);
			return ALL_OK;
        } else if (strcmp(state->word, "bg") == 0) {
			long pid = -1;
			char* endptr;
			state->buffer = strparse(state->word, state->buffer);
			pid = strtol(state->word, &endptr, 10);
			syscalls_bg(pid);
			return ALL_OK;
        } else if (strcmp(state->word, "write") == 0) {
			return ALL_OK;
        } else if (strcmp(state->word, "jobs") == 0) {
			syscalls_jobs();
			return ALL_OK;
        } else if (strcmp(state->word, "ls") == 0) {
            syscalls_lstat(state->working_directory);
            return ALL_OK;
        }else if (strcmp(state->word, "kill") == 0) {
			long pid = -1;
			char* endptr;
			state->buffer = strparse(state->word, state->buffer);
			pid = strtol(state->word, &endptr, 10);
			syscalls_kill_verbose(pid);
			return ALL_OK;
        } else if (strcmp(state->word, "cd") == 0) {
			state->buffer = strparse(state->word, state->buffer);
			file* new_dir;
            error_code = navigate(state->word, state->working_directory, &new_dir);
            switch (error_code){
                case NO_PROBLEM_FOUND:
                    state->working_directory = new_dir;
                    break;
                case IS_NOT_A_DIRECTORY:
                    printf("%s is not a directory\n", state->word);
                    break;
                case FILENAME_NOT_FOUND:
                    printf("No such directory\n");
                    break;
                default:
                    printf("Unknown error happened\n");
                    break;
            }
			return ALL_OK;
        } else if (strcmp(state->word, "mkdir") == 0) {
			state->buffer = strparse(state->word, state->buffer);
            error_code = new_file(state->working_directory, state->word, 'd', &buffer_file);
			return ALL_OK;
        } else if (strcmp(state->word, "touch") == 0) {
			state->buffer = strparse(state->word, state->buffer);
            error_code = new_file(state->working_directory, state->word, '-', &buffer_file);
			return ALL_OK;
        } else if (strcmp(state->word, "pwd") == 0) {
            char *working_directory_path = print_working_directory(state->working_directory);
            printf("Output: %s\n", working_directory_path);
            return ALL_OK;
        }else if (strcmp(state->word, "exec") == 0) {
			state->buffer = strparse(state->word, state->buffer);
			syscalls_exec(state->word, state->working_directory);
			return ALL_OK;
        } else {
            int variableIndex = isVariable(state->word, &state->variables);
            if (variableIndex == -1) variableIndex = addNewElement(state->word, &state->variables);
            state->buffer = strparse(state->word, state->buffer);
            if (state->word[0] != '=') {
                --state->variables.essenceCount;
                return SHIT_HAPPENED;
            }
            state->buffer = strparse(state->word, state->buffer);
            int firstOperand = readVarNum(state->word, &state->variables);
            state->buffer = strparse(state->operand, state->buffer);
            if (!(state->operand[0] == '\0' || state->operand[0] == '\n')) {
                state->buffer = strparse(state->word, state->buffer);
                int secondOperand = readVarNum(state->word, &state->variables);
                return Operation(firstOperand, secondOperand, state->operand,
                                 &state->variables.essenceValues[variableIndex]);
            } else {
                state->variables.essenceValues[variableIndex] = firstOperand;
                return ALL_OK;
            }
        }
        return SHIT_HAPPENED;
    }
    return error_code;
}

char goToLabel(interpretator_state *state){
    state->buffer = strparse(state->word, state->buffer);
    int variableIndex = isVariable(state->word, &state->labels);
    if(variableIndex == -1){
        return SHIT_HAPPENED;
    }
    state->position = state->labels.essenceValues[variableIndex];
    return ALL_OK;
}

char* getline_file(file* program, int *position) {
    int i = *position;
    int size;
    char *content = (char*)program->content;
    while(content[i] != '\n' && content[i] != 0) {
        i++;
    }
    size = i - *position;
    char* result = (char*)malloc(size*sizeof(char));
    strncpy(result, content + *position, size);
    *position += size + 1;

    return result;
}


void fillLabels(interpretator_state *state){
    while(1) {
        state->buffer = getline_file(state->program, &state->position);
        state->buffer = strparse(state->word, state->buffer);
        printf("%s\n", state->word);
        if (state->word[strlen(state->word) - 1] == ':') {
            addLabel(state);
        }
        if (strcmp(state->word, "end") == 0) {
            return;
        }
    }
}

char executeNextCommand(interpretator_state *state) {
    char *buffer;
    size_t len = 0;
    int fd_ret;
	if(state->pid == 0) {
        fd_ret = poll(state->fds, 2, 5 * 1000);
        if(fd_ret == 0) {
            exit(0);
        }
        if (!(state->fds[0].revents & POLLIN) || proc_foreground != state->pid) {
            state->status = PROC_BLOCKING_IO;
            return 2;
        }
        state->status = PROC_RUNNING;
        getline(&buffer, &len, stdin);
        state->position = ftell(stdin);
    } else{
        buffer = getline_file(state->program, &state->position);
    }
    state->buffer = buffer;
    state->buffer = strparse(state->word, state->buffer);
    char ret = interpretateNextWord(state);
    if(ret == SHIT_HAPPENED){
        printf("Wrong input\n");
    }
    if(state->pid == 0 && proc_foreground == state->pid) {
		printf("sh > ");
		fflush(stdout);
		fflush(stdin);
	}
    free(buffer);
    return ret;
}

interpretator_state initInterpretator(char* filename, int pid, file *working_directory) {
	interpretator_state state;
    if(pid == 0) {
        state.program = NULL;
    } else {
        state.program = initProc(filename, working_directory);
        if (state.program == NULL) {
            state.status = PROC_INCORRECT;
            return state;
        }
    }
    state.working_directory = working_directory;
    state.buffer = malloc(256);
    state.operand = malloc(OPERAND_MAX_SIZE);
    state.word = malloc(ESSENCE_NAME_SIZE + 2);
    state.variables = essenceInit();
    state.labels = essenceInit();
    state.position = 0;
    state.pid = pid;
    state.status = PROC_RUNNING;
    state.name = (char*)malloc(255);
    if(state.program == NULL) {
		strcpy(state.name, "stdin");
		state.fds[0].fd = STDIN_FILENO;
		state.fds[0].events = POLLIN;
		state.fds[1].fd = STDOUT_FILENO;
		state.fds[1].events = POLLOUT;
	} else {
		strcpy(state.name, filename);
		fillLabels(&state);
		state.position = 0;
	}
    return state;
}

int launchInterpretator(interpretator_state *state) {
	char error_code;
	if(state->status == PROC_KILLED) {
		syscalls_yield(state);
		printf("switch to dead process\n");
		return 0;
	}
    do {
        error_code = executeNextCommand(state);
		if(error_code == 2)
			scheduler_flag = true;
		if(scheduler_flag) {
			interrupt_handler(state);
			return error_code;
		}
	} while (error_code);
    return error_code;
}
