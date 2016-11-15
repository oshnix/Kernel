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

void syscalls_lstat(){
	printFileInfo(stdout, listDirectoryContent(workingDirectory));
}

void syscalls_exec(char* name) {
	int pid = 0;
	for(int i = 0; i < 256; i++) {
		if(proc[i].status == PROC_KILLED) {
			pid = i; 
			break;
		}
	}
    printf("Syscall exec pid: %i\n", pid);
	proc[pid] = initInterpretator(name, pid);

	if(proc[pid].status == PROC_INCORRECT) {
		//syscalls_kill(pid);
		proc[pid].status = PROC_INCORRECT;
		printf("sh: file %s not exists\n", name);
	} else {
		proc_count++;
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
	if(proc[pid].program) {
		free(proc[pid].name);
		free(proc[pid].variables.essenceValues);
		for(int i = 0; i < proc[pid].variables.essenceCount; i++) {
			free(proc[pid].variables.essenceNames[i]);
		}
		free(proc[pid].variables.essenceNames);
		fclose(proc[pid].program);
	}
	proc[pid].status = PROC_KILLED;
	proc_count--;
	if(proc_count == 0 || pid == 0) {
		exit(0);
	}
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
