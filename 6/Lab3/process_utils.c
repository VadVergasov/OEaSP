#include "process_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <libproc.h>

void list_processes() {
    pid_t pids[10240];
    int count = proc_listallpids(pids, sizeof(pids));

    if (count <= 0) {
        printf("Failed to get process list\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        char path[PROC_PIDPATHINFO_MAXSIZE];
        int ret = proc_pidpath(pids[i], path, sizeof(path));
        if (ret > 0) {
            printf("PID: %d, Executable: %s\n", pids[i], path);
        }
    }
}

void process_info(int pid) {
    char path[PROC_PIDPATHINFO_MAXSIZE];
    int ret = proc_pidpath(pid, path, sizeof(path));
    if (ret > 0) {
        printf("PID: %d, Executable: %s\n", pid, path);
    } else {
        printf("Process with PID %d not found\n", pid);
    }
}

int send_signal(int pid, int signal) {
    int result = kill(pid, signal);
    if (result == 0) {
        printf("Signal %d sent to process with PID %d\n", signal, pid);
    } else {
        printf("Failed to send signal %d to process with PID %d\n", signal,
               pid);
    }
    return result;
}
