#define _POSIX_C_SOURCE 200809L

#include <stdio.h>

#include <sys/statvfs.h>

#include "cinfo.h"



int show_disk(void)
{
    struct statvfs fs;

    if (statvfs("/", &fs) != 0) {
        perror("cinfo: statvfs");
        return 1;
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

    cinfo_heading("=== Root Filesystem ===");
    printf("Total:     %.2f GiB\n", total_gib);
    printf("Used:      %.2f GiB (%.1f%%)\n", used_gib, percent);
    printf("Available: %.2f GiB\n", available_gib);
    return 0;
}
