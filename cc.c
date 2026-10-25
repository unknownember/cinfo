#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/statvfs.h>

static void sysinfo(void)
{
    struct utsname u;
    long cpus = sysconf(_SC_NPROCESSORS_ONLN);

    if (uname(&u) == -1) {
        perror("uname");
        return;
    }

    printf("=== System Information ===\n");
    printf("OS:           %s\n", u.sysname);
    printf("Kernel:       %s\n", u.release);
    printf("Architecture: %s\n", u.machine);
    printf("Online CPUs:  %ld\n", cpus);
    printf("Username:     %s\n", getenv("USER") ?
            getenv("USER") : "unknown");
}

static void show_memory(void)
{
    FILE *fp = fopen("/proc/meminfo", "r");
    if (fp == NULL) {
        perror("cinfo: /proc/meminfo");
        return;
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
        return;
    }

    printf("Total RAM:     %.2f GiB\n", total / 1048576.0);
    printf("Available RAM: %.2f GiB\n", available / 1048576.0);
}


static void show_load(void)
{
    FILE *fp = fopen("/proc/loadavg", "r");

    if (fp == NULL) {
        perror("cinfo: /proc/loadavg");
        return;
    }

    double one, five, fifteen;

    if (fscanf(fp, "%lf %lf %lf", &one, &five, &fifteen) != 3) {
        fprintf(stderr, "cinfo: could not read load averages\n");
        fclose(fp);
        return;
    }

    fclose(fp);

    printf("Load average: %.2f, %.2f, %.2f\n",
           one, five, fifteen);
    puts("              1 min  5 min  15 min");
}


static void show_disk(void)
{
    struct statvfs fs;

    if (statvfs("/", &fs) != 0) {
        perror("cinfo: statvfs");
        return;
    }

    unsigned long long total =
        (unsigned long long)fs.f_blocks * fs.f_frsize;
    unsigned long long available =
        (unsigned long long)fs.f_bavail * fs.f_frsize;
    unsigned long long used =
        total - (unsigned long long)fs.f_bfree * fs.f_frsize;

    double total_gib = (double)total / (1024.0 * 1024.0 * 1024.0);
    double used_gib = (double)used / (1024.0 * 1024.0 * 1024.0);
    double available_gib =
        (double)available / (1024.0 * 1024.0 * 1024.0);
    double percent = total ? 100.0 * (double)used / (double)total : 0.0;

    puts("=== Root Filesystem ===");
    printf("Total:     %.2f GiB\n", total_gib);
    printf("Used:      %.2f GiB (%.1f%%)\n", used_gib, percent);
    printf("Available: %.2f GiB\n", available_gib);
}



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
	printf("  --cpu        Show CPU architecture and online processor count\n");
        printf("  --kernel     Show kernel information\n");
        printf("  --memory     Show RAM usage\n");
        printf("  --load       Show system load averages\n");
        printf("  --disk       Show root filesystem usage\n");
        printf("  --all        Show all system information\n");
        return 0;
    }

    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("cinfo 1.0.0");
        return 0;
    }

    if (argc == 2 && strcmp(argv[1], "--sysinfo") == 0) {
        sysinfo();
        return 0;
    }

    if (argc == 2 && strcmp(argv[1], "--memory") == 0) {
    show_memory();
    return 0;
    }

    if (argc == 2 && strcmp(argv[1], "--cpu") == 0) {
    struct utsname u;

    if (uname(&u) == -1) {
        perror("cinfo: uname");
        return 1;
    }

    printf("Architecture: %s\n", u.machine);
    printf("Online CPUs:  %ld\n", sysconf(_SC_NPROCESSORS_ONLN));
    return 0;
}

if (argc == 2 && strcmp(argv[1], "--kernel") == 0) {
    struct utsname u;

    if (uname(&u) == -1) {
        perror("cinfo: uname");
        return 1;
    }

    printf("Kernel: %s %s\n", u.sysname, u.release);
    return 0;
}


    if (argc == 2 && strcmp(argv[1], "--load") == 0) {
        show_load();
        return 0;
    }

    if (argc == 2 && strcmp(argv[1], "--disk") == 0) {
        show_disk();
        return 0;
    }

    if (argc == 2 && strcmp(argv[1], "--all") == 0) {
        sysinfo();
        putchar('\n');
        show_memory();
        putchar('\n');
        show_load();
        putchar('\n');
        show_disk();
        return 0;
    }


    fprintf(stderr, "cinfo: unknown option: %s\n", argv[1]);
    fprintf(stderr, "Try 'cinfo --help' for more information.\n");
    return 2;
}
