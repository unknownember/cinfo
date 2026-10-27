#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>

#include <sys/utsname.h>

#include "cinfo.h"



int main(int argc, char *argv[])
{
    char *filtered[argc + 1];
    int filtered_argc = 1;
    int color_mode = 0; /* 0=auto, 1=always, 2=never */
    filtered[0] = argv[0];

    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--color=", 8) == 0) {
            const char *mode = argv[i] + 8;
            if (strcmp(mode, "auto") == 0)
                color_mode = 0;
            else if (strcmp(mode, "always") == 0)
                color_mode = 1;
            else if (strcmp(mode, "never") == 0)
                color_mode = 2;
            else {
                fprintf(stderr,
                        "cinfo: invalid color mode: %s\n", mode);
                return 2;
            }
        } else {
            filtered[filtered_argc++] = argv[i];
        }
    }

    filtered[filtered_argc] = NULL;
    argv = filtered;
    argc = filtered_argc;

    const char *no_color = getenv("NO_COLOR");
    if (color_mode == 1) {
        cinfo_colors_enabled = 1;
    } else if (color_mode == 2) {
        cinfo_colors_enabled = 0;
    } else {
        cinfo_colors_enabled =
            isatty(STDOUT_FILENO) &&
            !(no_color && no_color[0] != '\0');
    }
    if (argc == 1 ||
        (argc == 2 && strcmp(argv[1], "--help") == 0) ||
        (argc == 2 && strcmp(argv[1], "-h") == 0)) {
        printf("cinfo - a tiny Linux system information tool\n\n");
        printf("Usage: cinfo [OPTION] [PATTERN]\n\n");
        printf("Options:\n");
        printf("  --sysinfo    Show system information\n");
        printf("  --fetch PATTERN  List matching files and total size\n");
        printf("  --color=MODE     Color output: auto, always, never\n");
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

    if (argc == 3 && strcmp(argv[1], "--fetch") == 0)
        return fetch_files(argv[2]);

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
