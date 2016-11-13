#include <stdio.h>
#include <stdbool.h>
#include "syscalls.h"
#include "interpretator.h"

extern size_t proc_count;
extern interpretator_state proc[255];
extern scheduler_flag;
void interrupt_handler(interpretator_state*);

void syscalls_jobs() {
	for(int i = 0; i < 255; i++) {
		if(proc[i].status == PROC_KILLED) {
			continue;
		}
		printf("[%i] ", i);
		switch(proc[i].status) {
			case PROC_RUNNING:
				printf("Running           ");
				break;
			case PROC_BLOCKING_IO:
				printf("Waiting for I/O   ");
				break;
			default:
				break;
		}
		printf("%s\n", proc[i].name);
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
	proc[pid].status = PROC_KILLED;
	proc_count--;
}
