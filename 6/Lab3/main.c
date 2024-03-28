#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "process_utils.h"

int main(int argc, char* argv[]) {
    if (argc == 2 && strcmp(argv[1], "-l") == 0) {
        list_processes();
    } else if (argc == 3 && strcmp(argv[1], "-p") == 0) {
        int pid = atoi(argv[2]);
        process_info(pid);
    } else if (argc == 4 && strcmp(argv[1], "-k") == 0) {
        int pid = atoi(argv[2]);
        int signal = atoi(argv[3]);
        send_signal(pid, signal);
    } else {
        printf("Usage: ./program -l\n");
        printf("       ./program -p <pid>\n");
        printf("       ./program -k <pid> <signal>\n");
    }

    return 0;
}
