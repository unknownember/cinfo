#define _POSIX_C_SOURCE 200809L

#include <stdio.h>

#include "cinfo.h"



int show_memory(void)
{
    FILE *fp = fopen("/proc/meminfo", "r");
    if (fp == NULL) {
        perror("cinfo: /proc/meminfo");
        return 1;
    }

    char line[256];
    unsigned long long total = 0, available = 0;

    while (fgets(line, sizeof line, fp)) {
        if (sscanf(line, "MemTotal: %llu kB", &total) == 1)
            continue;
        if (sscanf(line, "MemAvailable: %llu kB", &available) == 1)
            break;
    }

    fclose(fp);

    if (total == 0) {
        fprintf(stderr, "cinfo: could not read memory information\n");
        return 1;
    }

    printf("Total RAM:     %.2f GiB\n", total / 1048576.0);
    printf("Available RAM: %.2f GiB\n", available / 1048576.0);
    return 0;
}
