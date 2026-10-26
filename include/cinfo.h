#ifndef CINFO_H
#define CINFO_H

#define VERSION "1.2.0-rc1"

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

#endif
