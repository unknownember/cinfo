#ifndef CINFO_H
#define CINFO_H

#define VERSION "1.2.0-rc2"

int sysinfo_func(void);
int show_memory(void);
int show_load(void);
int show_host(void);
int show_uptime(void);
int show_processes(void);
int show_cpu(void);
int show_gpu(void);
int show_users(void);
int show_network(void);
int show_disk(void);
int fetch_files(const char *pattern);
extern int cinfo_colors_enabled;
const char *cinfo_color(const char *sequence);
void cinfo_heading(const char *text);

#endif
