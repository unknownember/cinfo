#define _POSIX_C_SOURCE 200809L

#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
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

#define VERSION "1.1.5"

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

static int show_memory_from(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        perror(path);
        return 1;
    }

    char line[256];
    unsigned long long total = 0, available = 0;
    int total_found = 0, available_found = 0;

    while (fgets(line, sizeof line, fp)) {
        char number[32], unit[8], extra;
        unsigned long long value;
        char *end;

        int fields = sscanf(line, "MemTotal: %31s %7s %c",
                            number, unit, &extra);
        if (fields == 2 && number[0] != '-' &&
            number[0] != '+' && strcmp(unit, "kB") == 0) {
            errno = 0;
            value = strtoull(number, &end, 10);
            if (errno == 0 && *number != '\0' && *end == '\0') {
                total = value;
                total_found = 1;
                continue;
            }
        }

        fields = sscanf(line, "MemAvailable: %31s %7s %c",
                        number, unit, &extra);
        if (fields == 2 && number[0] != '-' &&
            number[0] != '+' && strcmp(unit, "kB") == 0) {
            errno = 0;
            value = strtoull(number, &end, 10);
            if (errno == 0 && *number != '\0' && *end == '\0') {
                available = value;
                available_found = 1;
            }
        }
    }

    int read_error = ferror(fp);
    fclose(fp);

    if (read_error || !total_found || total == 0 || !available_found) {
        fprintf(stderr, "cinfo: could not read memory information\n");
        return 1;
    }

    printf("Total RAM:     %.2f GiB\n", total / 1048576.0);
    printf("Available RAM: %.2f GiB\n", available / 1048576.0);
    return 0;
}

static int show_memory(void)
{
    return show_memory_from("/proc/meminfo");
}


static int show_load_from(const char *path)
{
    FILE *fp = fopen(path, "r");

    if (fp == NULL) {
        if (strcmp(path, "/proc/loadavg") == 0)
            perror("cinfo: /proc/loadavg");
        else
            perror(path);
        return 1;
    }

    double one, five, fifteen;
    int parsed = fscanf(fp, "%lf %lf %lf", &one, &five, &fifteen);
    fclose(fp);

    if (parsed != 3 || !isfinite(one) || !isfinite(five) ||
        !isfinite(fifteen) || one < 0.0 || five < 0.0 || fifteen < 0.0) {
        fprintf(stderr, "cinfo: could not read valid load averages\n");
        return 1;
    }

    printf("Load average: %.2f, %.2f, %.2f\n",
           one, five, fifteen);
    puts("              1 min  5 min  15 min");
    return 0;
}

static int show_load(void)
{
    return show_load_from("/proc/loadavg");
}


static int has_non_whitespace(const char *value)
{
    for (const unsigned char *ch = (const unsigned char *)value;
         *ch != '\0'; ch++) {
        if (!isspace(*ch))
            return 1;
    }
    return 0;
}

static int read_distro(const char *path, char distro[256])
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
        return -1;

    char *line = NULL;
    size_t capacity = 0;
    char pretty_name[256] = "";
    char name[256] = "";
    char version[256] = "";
    int read_error = 0;
    int read_errno = 0;

    for (;;) {
        errno = 0;
        ssize_t line_length = getline(&line, &capacity, fp);
        if (line_length == -1) {
            if (ferror(fp) || errno != 0) {
                read_error = 1;
                read_errno = errno != 0 ? errno : EIO;
            }
            break;
        }

        char *value;
        char *target;

        if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
            value = line + 12;
            target = pretty_name;
        } else if (strncmp(line, "NAME=", 5) == 0) {
            value = line + 5;
            target = name;
        } else if (strncmp(line, "VERSION=", 8) == 0) {
            value = line + 8;
            target = version;
        } else {
            continue;
        }

        value[strcspn(value, "\r\n")] = '\0';
        size_t len = strlen(value);
        if (len > 0 && (value[0] == 34 || value[0] == 39) &&
            (len < 2 || value[len - 1] != value[0]))
            continue;
        if (len >= 2 &&
            ((value[0] == '"' && value[len - 1] == '"') ||
             (value[0] == '\'' && value[len - 1] == '\''))) {
            char quote = value[0];
            value[len - 1] = '\0';
            value++;
            if (quote == '"') {
                char parsed[256];
                size_t i = 0, j = 0;
                while (value[i] != '\0' && j < sizeof(parsed) - 1) {
                    if (value[i] == '\\' && value[i + 1] != '\0')
                        i++;
                    parsed[j++] = value[i++];
                }
                parsed[j] = '\0';
                snprintf(target, 256, "%s", parsed);
            } else {
                snprintf(target, 256, "%s", value);
            }
        } else {
            snprintf(target, 256, "%s", value);
        }
    }

    free(line);
    errno = 0;
    if (fclose(fp) != 0) {
        if (!read_error)
            read_errno = errno != 0 ? errno : EIO;
        read_error = 1;
    }
    if (read_error) {
        errno = read_errno;
        return -1;
    }

    if (has_non_whitespace(pretty_name))
        snprintf(distro, 256, "%s", pretty_name);
    else if (has_non_whitespace(name) && has_non_whitespace(version)) {
        size_t used = strlen(name);
        if (used > 256 - 1)
            used = 256 - 1;
        memcpy(distro, name, used);

        if (used < 256 - 1)
            distro[used++] = ' ';

        size_t remaining = 256 - 1 - used;
        size_t version_len = strlen(version);
        if (version_len > remaining)
            version_len = remaining;
        memcpy(distro + used, version, version_len);
        distro[used + version_len] = '\0';
    }
    else if (has_non_whitespace(name))
        snprintf(distro, 256, "%s", name);
    else
        distro[0] = '\0';

    return distro[0] != '\0' ? 0 : 1;
}

static int validate_hostname(const char *hostname, size_t size)
{
    if (size == 0 || memchr(hostname, '\0', size) == NULL) {
        errno = ENAMETOOLONG;
        return -1;
    }

    if (hostname[0] == '\0') {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

static int read_hostname(char *hostname, size_t size)
{
    if (size == 0) {
        errno = EINVAL;
        return -1;
    }

    memset(hostname, 0xFF, size);
    if (gethostname(hostname, size) != 0)
        return -1;

    return validate_hostname(hostname, size);
}

static int show_host(void)
{
    char hostname[256];
    char distro[256];

    if (read_hostname(hostname, sizeof(hostname)) != 0) {
        perror("cinfo: gethostname");
        return 1;
    }

    int result = read_distro("/etc/os-release", distro);
    if (result == -1) {
        perror("cinfo: reading os-release");
        return 1;
    }
    if (result == 1) {
        fprintf(stderr, "cinfo: no usable PRETTY_NAME or NAME in os-release\n");
        return 1;
    }

    printf("Hostname:     %s\n", hostname);
    printf("Distribution: %s\n", distro);
    return 0;
}

static int show_uptime_from(const char *path)
{
    FILE *fp = fopen(path, "r");

    if (fp == NULL) {
        if (strcmp(path, "/proc/uptime") == 0)
            perror("cinfo: /proc/uptime");
        else
            perror(path);
        return 1;
    }

    double uptime_seconds;

    if (fscanf(fp, "%lf", &uptime_seconds) != 1) {
        fprintf(stderr, "cinfo: could not read uptime information\n");
        fclose(fp);
        return 1;
    }

    fclose(fp);

    if (!isfinite(uptime_seconds) || uptime_seconds < 0.0 ||
        uptime_seconds >= (double)ULLONG_MAX) {
        fprintf(stderr, "cinfo: invalid uptime value\n");
        return 1;
    }

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

static int show_uptime(void)
{
    return show_uptime_from("/proc/uptime");
}

static int show_processes_from(const char *path)
{
    DIR *proc = opendir(path);
    if (proc == NULL) {
        perror("cinfo: opendir process directory");
        return 1;
    }

    size_t count = 0;
    struct dirent *entry;
    int status = 0;

    for (;;) {
        errno = 0;
        entry = readdir(proc);
        if (entry == NULL) {
            if (errno != 0) {
                perror("cinfo: readdir process directory");
                status = 1;
            }
            break;
        }

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
        perror("cinfo: closedir process directory");
        status = 1;
    }

    if (status != 0)
        return 1;

    printf("Processes:    %zu\n", count);
    return 0;
}

static int show_processes(void)
{
    return show_processes_from("/proc");
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

static int show_gpu_from(const char *drm_path)
{
    DIR *drm = opendir(drm_path);
    if (drm == NULL) {
        perror("cinfo: opendir DRM directory");
        return 1;
    }

    struct dirent *entry;
    int found = 0;
    int status = 0;

    while ((entry = readdir(drm)) != NULL) {
        if (strncmp(entry->d_name, "card", 4) != 0 ||
            !isdigit((unsigned char)entry->d_name[4]))
            continue;

        const char *suffix = entry->d_name + 4;
        while (isdigit((unsigned char)*suffix))
            suffix++;
        if (*suffix != '\0')
            continue;

        char path[512];
        char vendor[64];
        char device[64];
        char driver[128];

        int n = snprintf(path, sizeof(path), "%s/%s/device/vendor",
                         drm_path, entry->d_name);
        if (n < 0 || (size_t)n >= sizeof(path)) {
            fprintf(stderr, "cinfo: DRM path too long\n");
            status = 1;
            continue;
        }

        FILE *fp = fopen(path, "r");
        if (fp == NULL)
            continue;

        if (fgets(vendor, sizeof(vendor), fp) == NULL) {
            if (ferror(fp)) {
                perror("cinfo: read GPU vendor");
                status = 1;
            }
            if (fclose(fp) != 0) {
                perror("cinfo: close GPU vendor");
                status = 1;
            }
            continue;
        }
        if (fclose(fp) != 0) {
            perror("cinfo: close GPU vendor");
            status = 1;
            continue;
        }
        vendor[strcspn(vendor, "\r\n")] = '\0';

        n = snprintf(path, sizeof(path), "%s/%s/device/device",
                     drm_path, entry->d_name);
        if (n < 0 || (size_t)n >= sizeof(path)) {
            fprintf(stderr, "cinfo: DRM path too long\n");
            status = 1;
            continue;
        }

        fp = fopen(path, "r");
        if (fp == NULL)
            continue;

        if (fgets(device, sizeof(device), fp) == NULL) {
            if (ferror(fp)) {
                perror("cinfo: read GPU device");
                status = 1;
            }
            if (fclose(fp) != 0) {
                perror("cinfo: close GPU device");
                status = 1;
            }
            continue;
        }
        if (fclose(fp) != 0) {
            perror("cinfo: close GPU device");
            status = 1;
            continue;
        }
        device[strcspn(device, "\r\n")] = '\0';

        n = snprintf(path, sizeof(path), "%s/%s/device/driver",
                     drm_path, entry->d_name);
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
        perror("cinfo: closedir DRM directory");
        status = 1;
    }

    if (!found && status == 0)
        puts("GPU:          No DRM GPU devices found");

    return status;
}

static int show_gpu(void)
{
    return show_gpu_from("/sys/class/drm");
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
        printf("  --sysinfo    Show basic system information\n");
        printf("  --all        Show all available system information\n\n");
        printf("  --cpu        Show CPU model, architecture and online processor count\n");
        printf("  --kernel     Show kernel name and release\n");
        printf("  --memory     Show RAM usage\n");
        printf("  --load       Show system load averages\n");
        printf("  --uptime     Show system uptime\n");
        printf("  --host       Show hostname and Linux distribution\n");
        printf("  --processes  Show process count\n");
        printf("  --gpu        Show graphics devices\n");
        printf("  --users      Show logged-in terminal sessions\n");
        printf("  --network    Show network interfaces and addresses\n");
        printf("  --disk       Show root filesystem usage\n\n");
        printf("  -h, --help   Show this help message\n");
        printf("  --version    Show version information\n");
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
