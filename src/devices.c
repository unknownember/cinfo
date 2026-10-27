#define _POSIX_C_SOURCE 200809L

#include <stdio.h>

#include <string.h>

#include <dirent.h>

#include <ctype.h>

#include <unistd.h>

#include "cinfo.h"



int show_gpu(void)
{
    DIR *drm = opendir("/sys/class/drm");
    if (drm == NULL) {
        perror("cinfo: opendir /sys/class/drm");
        return 1;
    }

    struct dirent *entry;
    int found = 0;
    int status = 0;

    while ((entry = readdir(drm)) != NULL) {
        if (strncmp(entry->d_name, "card", 4) != 0 ||
            !isdigit((unsigned char)entry->d_name[4]))
            continue;

        char path[512];
        char vendor[64];
        char device[64];
        char driver[128];

        int n = snprintf(path, sizeof(path),
                         "/sys/class/drm/%s/device/vendor", entry->d_name);
        if (n < 0 || (size_t)n >= sizeof(path)) {
            fprintf(stderr, "cinfo: DRM path too long\n");
            status = 1;
            continue;
        }

        FILE *fp = fopen(path, "r");
        if (fp == NULL) {
            /* Not every DRM card entry necessarily exposes a PCI device. */
            continue;
        }
        if (fgets(vendor, sizeof(vendor), fp) == NULL) {
            if (ferror(fp)) {
                perror("cinfo: read GPU vendor");
                status = 1;
            }
            fclose(fp);
            continue;
        }
        if (fclose(fp) != 0) {
            perror("cinfo: close GPU vendor");
            status = 1;
            continue;
        }
        vendor[strcspn(vendor, "\r\n")] = '\0';

        n = snprintf(path, sizeof(path),
                     "/sys/class/drm/%s/device/device", entry->d_name);
        if (n < 0 || (size_t)n >= sizeof(path)) {
            fprintf(stderr, "cinfo: DRM path too long\n");
            status = 1;
            continue;
        }
        fp = fopen(path, "r");
        if (fp == NULL || fgets(device, sizeof(device), fp) == NULL) {
            if (fp != NULL) {
                if (ferror(fp)) {
                    perror("cinfo: read GPU device");
                    status = 1;
                }
                fclose(fp);
            }
            continue;
        }
        if (fclose(fp) != 0) {
            perror("cinfo: close GPU device");
            status = 1;
            continue;
        }
        device[strcspn(device, "\r\n")] = '\0';

        n = snprintf(path, sizeof(path),
                     "/sys/class/drm/%s/device/driver", entry->d_name);
        if (n < 0 || (size_t)n >= sizeof(path)) {
            fprintf(stderr, "cinfo: DRM path too long\n");
            status = 1;
            continue;
        }

        ssize_t len = readlink(path, driver, sizeof(driver) - 1);
        if (len < 0) {
            snprintf(driver, sizeof(driver), "unknown");
        } else {
            driver[len] = '\0';
            char *last = strrchr(driver, '/');
            if (last != NULL)
                memmove(driver, last + 1, strlen(last + 1) + 1);
        }

        if (!found)
            cinfo_heading("GPU Information");
        printf("GPU (%s): vendor %s, device %s, driver %s\n",
               entry->d_name, vendor, device, driver);
        found = 1;
    }

    if (closedir(drm) != 0) {
        perror("cinfo: closedir /sys/class/drm");
        status = 1;
    }

    if (!found && status == 0) {
        puts("GPU:          No DRM GPU devices found");
    }

    return status;
}
