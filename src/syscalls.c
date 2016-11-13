#include <stdio.h>
#include <stdbool.h>
#include "syscalls.h"
#include "interpretator.h"

extern size_t proc_count;
extern interpretator_state proc[255];
extern scheduler_flag;

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

void syscalls_yield() {
	scheduler_flag = true;
}

void syscalls_kill(int pid) {
	proc[pid].status = PROC_KILLED;
	proc_count--;
	printf("Process %i killed\n", pid);
	syscalls_yield();
}
