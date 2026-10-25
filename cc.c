#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/statvfs.h>
#include <utmp.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define VERSION "1.1.1"

static int sysinfo_func(void)
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

static int show_memory(void)
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


static int show_load(void)
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


static int show_host(void)
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


static int show_uptime(void)
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

static int show_processes(void)
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

static int show_cpu(void)
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

static int show_gpu(void)
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

static int show_users(void)
{
    setutent();

    size_t count = 0;
    struct utmp *entry;

    puts("Logged-in terminal sessions:");
    while ((entry = getutent()) != NULL) {
        if (entry->ut_type != USER_PROCESS)
            continue;

        printf("  %.*s on %.*s\n",
               (int)sizeof(entry->ut_user), entry->ut_user,
               (int)sizeof(entry->ut_line), entry->ut_line);
        count++;
    }

    endutent();

    if (count == 0)
        puts("  None found");
    printf("User sessions: %zu\n", count);
    return 0;
}

static int show_network(void)
{
    struct ifaddrs *interfaces = NULL;
    if (getifaddrs(&interfaces) != 0) {
        perror("cinfo: getifaddrs");
        return 1;
    }

    puts("Network interfaces:");
    int found = 0;

    for (struct ifaddrs *item = interfaces; item != NULL; item = item->ifa_next) {
        if (item->ifa_addr == NULL)
            continue;

        int family = item->ifa_addr->sa_family;
        if (family != AF_INET && family != AF_INET6)
            continue;

        char address[INET6_ADDRSTRLEN];
        const void *source;

        if (family == AF_INET) {
            source = &((struct sockaddr_in *)item->ifa_addr)->sin_addr;
        } else {
            source = &((struct sockaddr_in6 *)item->ifa_addr)->sin6_addr;
        }

        if (inet_ntop(family, source, address, sizeof(address)) == NULL) {
            perror("cinfo: inet_ntop");
            freeifaddrs(interfaces);
            return 1;
        }

        printf("  %s: %s\n", item->ifa_name, address);
        found = 1;
    }

    freeifaddrs(interfaces);

    if (!found)
        puts("  No IPv4 or IPv6 addresses found");

    return 0;
}

static int show_disk(void)
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

    puts("=== Root Filesystem ===");
    printf("Total:     %.2f GiB\n", total_gib);
    printf("Used:      %.2f GiB (%.1f%%)\n", used_gib, percent);
    printf("Available: %.2f GiB\n", available_gib);
    return 0;
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
