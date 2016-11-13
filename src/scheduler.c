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
interpretator_state proc[255];
size_t proc_count = 0;
size_t proc_current = 0;

void interrupt_handler(interpretator_state state) {
	if(!scheduler_flag) {
		return;
	}
	proc_current = (proc_current + 1) % proc_count;
	//printf("Switch to process with position %lu!\n", proc->position);
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
	/* Start a virtual timer. It counts down whenever this process is
	executing. */
	setitimer (ITIMER_PROF, &timer, NULL);

	FILE *KernelIn = stdin;

	proc[0] = initInterpretator("res/input", 0);
	proc_count++;
	proc[1] = initInterpretator("res/input1", 1);
	proc_count++;



	current_state = &proc[0];
	for(;;)
		launchInterpretator(current_state);

	return 0;
}
