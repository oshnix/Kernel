#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

#include "interpretator.h"

void syscalls_jobs();
int syscalls_kill(int pid);
void syscalls_kill_verbose(int pid);
void syscalls_yield();
void syscalls_exec(char* name, file* working_directory);
void syscalls_lstat(file* working_directory);
void syscalls_bg(int pid);
void syscalls_fg(int pid);

#endif
