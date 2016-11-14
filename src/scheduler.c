#include <signal.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#include "interpretator.h"

volatile sig_atomic_t scheduler_flag = false;
interpretator_state* current_state;
interpretator_state proc[256];
size_t proc_count = 0;
size_t proc_current = 0;

void interrupt_handler(interpretator_state *state) {
	if(!scheduler_flag) {
		return;
	}
	do {
		proc_current = (proc_current + 1) % 255;
	} while(proc[proc_current].status == PROC_KILLED);
	//printf("Switch to process with position %li!\n", proc_current);
	scheduler_flag = false;
    current_state = &proc[proc_current];
}

void timer_handler(int sig) {
	scheduler_flag = true;
}


int main() {
	struct sigaction sa;
	struct itimerval timer;

	/* Install timer_handler as the signal handler for SIGVTALRM. */
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &timer_handler;
	sigaction (SIGPROF, &sa, NULL);
	/* Configure the timer to expire after 25 msec... */
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 250000;
	/* ... and every 250 msec after that. */
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 250000;
	
	for(size_t i = 0; i < 256; i++) {
		proc[i].status = PROC_KILLED;
	}
	
	syscalls_exec(NULL);
	
	current_state = &proc[0];
	
	setitimer (ITIMER_PROF, &timer, NULL);
	while(proc_count)
		launchInterpretator(current_state);
	
	return 0;
}
