#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cinfo.h"

int cinfo_colors_enabled = 0;

const char *cinfo_color(const char *sequence)
{
    return cinfo_colors_enabled ? sequence : "";
}

void cinfo_heading(const char *text)
{
    printf("%s%s%s\n", cinfo_color("\033[1;36m"), text,
           cinfo_color("\033[0m"));
}
