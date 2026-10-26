#define _POSIX_C_SOURCE 200809L

#include <stdio.h>

#include <string.h>

#include <sys/utsname.h>

#include "cinfo.h"



int main(int argc, char *argv[])
{
    if (argc == 1 ||
        (argc == 2 && strcmp(argv[1], "--help") == 0) ||
        (argc == 2 && strcmp(argv[1], "-h") == 0)) {
        printf("cinfo - a tiny Linux system information tool\n\n");
        printf("Usage: cinfo [OPTION]\n\n");
        printf("Options:\n");
        printf("  --sysinfo    Show system information\n");
        printf("  --help       Show this help message\n");
        printf("  --version    Show version information\n");
        printf("  --cpu        Show CPU model, architecture and online processor count\n");
        printf("  --kernel     Show kernel information\n");
        printf("  --memory     Show RAM usage\n");
        printf("  --load       Show system load averages\n");
        printf("  --disk       Show root filesystem usage\n");
        printf("  --uptime     Show system uptime\n");
        printf("  --host       Show hostname and distribution\n");
        printf("  --processes  Show process count\n");
        printf("  --gpu        Show graphics devices\n");
        printf("  --users      Show logged-in terminal sessions\n");
        printf("  --network    Show network interfaces and addresses\n");
        printf("  --all        Show all system information\n");
        return 0;
    }

    if (argc != 2) {
        fprintf(stderr, "Usage: cinfo [OPTION]\n");
        return 2;
    }

    if (strcmp(argv[1], "--version") == 0) {
        printf("cinfo %s\n", VERSION);
        return 0;
    }
    if (strcmp(argv[1], "--sysinfo") == 0)
        return sysinfo_func();
    if (strcmp(argv[1], "--memory") == 0)
        return show_memory();
    if (strcmp(argv[1], "--load") == 0)
        return show_load();
    if (strcmp(argv[1], "--disk") == 0)
        return show_disk();
    if (strcmp(argv[1], "--uptime") == 0)
        return show_uptime();
    if (strcmp(argv[1], "--host") == 0)
        return show_host();
    if (strcmp(argv[1], "--processes") == 0)
        return show_processes();
    if (strcmp(argv[1], "--gpu") == 0)
        return show_gpu();
    if (strcmp(argv[1], "--users") == 0)
        return show_users();
    if (strcmp(argv[1], "--network") == 0)
        return show_network();

    if (strcmp(argv[1], "--cpu") == 0)
        return show_cpu();

    if (strcmp(argv[1], "--kernel") == 0) {
        struct utsname u;
        if (uname(&u) == -1) {
            perror("cinfo: uname");
            return 1;
        }
        printf("Kernel: %s %s\n", u.sysname, u.release);
        return 0;
    }

    if (strcmp(argv[1], "--all") == 0) {
        int status = 0;
        if (sysinfo_func() != 0) status = 1;
        putchar('\n');
        if (show_memory() != 0) status = 1;
        putchar('\n');
        if (show_load() != 0) status = 1;
        putchar('\n');
        if (show_uptime() != 0) status = 1;
        putchar('\n');
        if (show_host() != 0) status = 1;
        if (show_processes() != 0) status = 1;
        putchar('\n');
        if (show_cpu() != 0) status = 1;
        putchar('\n');
        if (show_gpu() != 0) status = 1;
        putchar('\n');
        if (show_users() != 0) status = 1;
        putchar('\n');
        if (show_network() != 0) status = 1;
        putchar('\n');
        if (show_disk() != 0) status = 1;
        return status;
    }

    fprintf(stderr, "cinfo: unknown option: %s\n", argv[1]);
    fprintf(stderr, "Try 'cinfo --help' for more information.\n");
    return 2;
}
