#define _POSIX_C_SOURCE 200809L

#include <stdio.h>

#include "cinfo.h"



int show_load(void)
{
    FILE *fp = fopen("/proc/loadavg", "r");

    if (fp == NULL) {
        perror("cinfo: /proc/loadavg");
        return 1;
    }

    double one, five, fifteen;

    if (fscanf(fp, "%lf %lf %lf", &one, &five, &fifteen) != 3) {
        fprintf(stderr, "cinfo: could not read load averages\n");
        fclose(fp);
        return 1;
    }

    fclose(fp);

    printf("Load average: %.2f, %.2f, %.2f\n",
           one, five, fifteen);
    puts("              1 min  5 min  15 min");
    return 0;
}
