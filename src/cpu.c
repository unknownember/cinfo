#define _POSIX_C_SOURCE 200809L

#include <stdio.h>

#include <string.h>

#include <unistd.h>

#include <sys/utsname.h>

#include "cinfo.h"



int show_cpu(void)
{
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (fp == NULL) {
        perror("cinfo: fopen /proc/cpuinfo");
        return 1;
    }

    char line[512];
    char model[512] = "";

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, "model name", 10) == 0) {
            char *value = strchr(line, ':');
            if (value != NULL) {
                value++;
                while (*value == ' ' || *value == '\t')
                    value++;
                value[strcspn(value, "\r\n")] = '\0';
                if (*value != '\0') {
                    snprintf(model, sizeof(model), "%s", value);
                    break;
                }
            }
        }
    }

    if (ferror(fp)) {
        perror("cinfo: read /proc/cpuinfo");
        fclose(fp);
        return 1;
    }
    if (fclose(fp) != 0) {
        perror("cinfo: fclose /proc/cpuinfo");
        return 1;
    }

    struct utsname u;
    if (uname(&u) == -1) {
        perror("cinfo: uname");
        return 1;
    }

    long cpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (cpus == -1) {
        perror("cinfo: sysconf");
        return 1;
    }

    printf("CPU:          %s\n", model[0] != '\0' ? model : "Unknown");
    printf("Architecture: %s\n", u.machine);
    printf("Online CPUs:  %ld\n", cpus);
    return 0;
}
