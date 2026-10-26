#define _POSIX_C_SOURCE 200809L

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <dirent.h>

#include <ctype.h>

#include <unistd.h>

#include <sys/utsname.h>

#include "cinfo.h"



int sysinfo_func(void)
{
    struct utsname u;
    long cpus = sysconf(_SC_NPROCESSORS_ONLN);

    if (cpus == -1) {
        perror("cinfo: sysconf");
        return 1;
    }

    if (uname(&u) == -1) {
        perror("uname");
        return 1;
    }

    printf("=== System Information ===\n");
    printf("OS:           %s\n", u.sysname);
    printf("Kernel:       %s\n", u.release);
    printf("Architecture: %s\n", u.machine);
    printf("Online CPUs:  %ld\n", cpus);
    printf("Username:     %s\n", getenv("USER") ?
            getenv("USER") : "unknown");
    return 0;
}

int show_host(void)
{
    char hostname[256];

    if (gethostname(hostname, sizeof(hostname)) != 0) {
        perror("cinfo: gethostname");
        return 1;
    }
    hostname[sizeof(hostname) - 1] = '\0';

    FILE *fp = fopen("/etc/os-release", "r");
    if (fp == NULL) {
        perror("cinfo: /etc/os-release");
        return 1;
    }

    char *line = NULL;
    size_t capacity = 0;
    char distro[256] = "";
    int read_error = 0;

    while (getline(&line, &capacity, fp) != -1) {
        if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
            char *value = line + 12;
            value[strcspn(value, "\r\n")] = '\0';

            size_t len = strlen(value);
            if (len >= 2 &&
                ((value[0] == '"' && value[len - 1] == '"') ||
                 (value[0] == '\'' && value[len - 1] == '\''))) {
                value[len - 1] = '\0';
                value++;
            }

            snprintf(distro, sizeof(distro), "%s", value);
            break;
        }
    }

    if (ferror(fp))
        read_error = 1;

    free(line);
    fclose(fp);

    if (read_error) {
        fprintf(stderr, "cinfo: failed reading /etc/os-release\n");
        return 1;
    }

    if (distro[0] == '\0') {
        fprintf(stderr, "cinfo: PRETTY_NAME not found in /etc/os-release\n");
        return 1;
    }

    printf("Hostname:     %s\n", hostname);
    printf("Distribution: %s\n", distro);
    return 0;
}

int show_uptime(void)
{
    FILE *fp = fopen("/proc/uptime", "r");

    if (fp == NULL) {
        perror("cinfo: /proc/uptime");
        return 1;
    }

    double uptime_seconds;

    if (fscanf(fp, "%lf", &uptime_seconds) != 1) {
        fprintf(stderr, "cinfo: could not read uptime information\n");
        fclose(fp);
        return 1;
    }

    fclose(fp);

    unsigned long long seconds = (unsigned long long)uptime_seconds;
    unsigned long long days = seconds / 86400;
    unsigned long long hours = (seconds % 86400) / 3600;
    unsigned long long minutes = (seconds % 3600) / 60;
    unsigned long long secs = seconds % 60;

    printf("Uptime: ");
    if (days > 0)
        printf("%llu days, ", days);
    printf("%llu hours, %llu minutes, %llu seconds\n",
           hours, minutes, secs);

    return 0;
}

int show_processes(void)
{
    DIR *proc = opendir("/proc");
    if (proc == NULL) {
        perror("cinfo: opendir /proc");
        return 1;
    }

    size_t count = 0;
    struct dirent *entry;

    while ((entry = readdir(proc)) != NULL) {
        const unsigned char *name = (const unsigned char *)entry->d_name;
        if (*name == '\0')
            continue;

        int numeric = 1;
        for (const unsigned char *ch = name; *ch != '\0'; ch++) {
            if (!isdigit(*ch)) {
                numeric = 0;
                break;
            }
        }

        if (numeric)
            count++;
    }

    if (closedir(proc) != 0) {
        perror("cinfo: closedir /proc");
        return 1;
    }

    printf("Processes:    %zu\n", count);
    return 0;
}
