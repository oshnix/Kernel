#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

void syscalls_jobs();
int syscalls_kill(int pid);
void syscalls_kill_verbose(int pid);
void syscalls_yield();
void syscalls_exec(char* name);

#endif
