#include <stdio.h>
#include <stdbool.h>
#include "syscalls.h"
#include <stdlib.h>
#include "filesystem.h"
#include "interpretator.h"

extern size_t proc_count;
extern interpretator_state proc[256];
extern scheduler_flag;
extern proc_foreground;

void interrupt_handler(interpretator_state*);

void syscalls_jobs() {
	for(int i = 0; i < 256; i++) {
		if(proc[i].status == PROC_KILLED) {
			continue;
		}
		printf("[%i] ", i);
		switch(proc[i].status) {
			case PROC_RUNNING:
				printf("Running           ");
				break;
			case PROC_STOPPED:
				printf("Stopped           ");
				break;
			case PROC_BLOCKING_IO:
				printf("Waiting for I/O   ");
				break;
			default:
				continue;
		}
		printf("%s\n", proc[i].name);
	}
}

void syscalls_lstat(file* working_directory){
	list_directory_content(working_directory, stdout);
}

void syscalls_exec(char* name, file* working_directory) {
	int pid = 0;
	for(int i = 0; i < 256; i++) {
		if(proc[i].status == PROC_KILLED) {
			pid = i; 
			break;
		}
	}
	proc[pid] = initInterpretator(name, pid, working_directory);
	if(proc[pid].status == PROC_INCORRECT) {
		//syscalls_kill(pid);
        printf("proc[%i] incorrect\n", pid);
		proc[pid].status = PROC_KILLED;
		printf("sh: file %s not exists\n", name);
	} else {
		proc_count++;
        printf("proc[%i] inited\n", pid);
	}
}

void syscalls_yield(interpretator_state* state) {
	scheduler_flag = true;
	interrupt_handler(state);
}

void syscalls_kill_verbose(int pid) {
	if(syscalls_kill(pid) == 0) {
		printf("Process %i killed\n", pid);
	} else {
		printf("No process with pid %i\n", pid);
	}
}

int syscalls_kill(int pid) {
	if(proc[pid].status == PROC_KILLED) {
		return 1;
	}
	if(proc_count == 0 || pid == 0) {
		exit(0);
	}
	if(proc[pid].program) {
		free(proc[pid].name);
		free(proc[pid].variables.essenceValues);
		for(int i = 0; i < proc[pid].variables.essenceCount; i++) {
			free(proc[pid].variables.essenceNames[i]);
		}
		free(proc[pid].variables.essenceNames);
		//free(proc[pid].program);
	}
	proc[pid].status = PROC_KILLED;
	proc_count--;
	return 0;
}

void syscalls_bg(int pid) {
	if(proc[pid].status != PROC_KILLED) {
		proc[pid].status = PROC_RUNNING;
	} else {
		printf("No process with pid %i\n", pid);
	}
}
void syscalls_fg(int pid) {
	if(proc[pid].status != PROC_KILLED) {
		proc_foreground = pid;
		proc[pid].status = PROC_RUNNING;
	} else {
		printf("No process with pid %i\n", pid);
	}
}
