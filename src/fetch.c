#define _POSIX_C_SOURCE 200809L

#include <glob.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>

#include "cinfo.h"

static void format_size(uintmax_t bytes, char *out, size_t out_size)
{
    static const char *units[] = {
        "B", "KiB", "MiB", "GiB", "TiB", "PiB"
    };
    double size = (double)bytes;
    size_t unit = 0;

    while (size >= 1024.0 && unit < sizeof units / sizeof units[0] - 1) {
        size /= 1024.0;
        unit++;
    }

    if (unit == 0)
        snprintf(out, out_size, "%" PRIuMAX " B", bytes);
    else
        snprintf(out, out_size, "%.1f %s", size, units[unit]);
}

int fetch_files(const char *pattern)
{
    glob_t matches = {0};
    int result = glob(pattern, 0, NULL, &matches);

    if (result != 0) {
        if (result == GLOB_NOMATCH)
            fprintf(stderr, "cinfo: no files match: %s\n", pattern);
        else
            fprintf(stderr, "cinfo: could not expand pattern: %s\n", pattern);

        globfree(&matches);
        return 1;
    }

    uintmax_t total = 0;
    int status = 0;
    size_t listed = 0;

    cinfo_heading("File Fetcher");
    printf("------------\n");

    for (size_t i = 0; i < matches.gl_pathc; i++) {
        struct stat info;

        if (stat(matches.gl_pathv[i], &info) != 0) {
            perror(matches.gl_pathv[i]);
            status = 1;
            continue;
        }

        if (!S_ISREG(info.st_mode))
            continue;

        if (info.st_size < 0) {
            fprintf(stderr, "cinfo: invalid file size: %s\n",
                    matches.gl_pathv[i]);
            status = 1;
            continue;
        }

        uintmax_t size = (uintmax_t)info.st_size;

        if (UINTMAX_MAX - total < size) {
            fprintf(stderr, "cinfo: total file size overflow\n");
            status = 1;
            break;
        }

        char formatted[64];
        format_size(size, formatted, sizeof formatted);
        printf("%s\t%s\n", matches.gl_pathv[i], formatted);

        total += size;
        listed++;
    }

    if (listed == 0) {
        fprintf(stderr, "cinfo: no regular files found for: %s\n", pattern);
        status = 1;
    } else {
        char formatted[64];
        format_size(total, formatted, sizeof formatted);
        printf("\nTotal size: %s\n", formatted);
    }

    globfree(&matches);
    return status;
}
