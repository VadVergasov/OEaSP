#ifndef PROCESS_UTILS_H
#define PROCESS_UTILS_H

void list_processes();
void process_info(int pid);
int send_signal(int pid, int signal);

#endif
