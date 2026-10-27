#define _POSIX_C_SOURCE 200809L

#include <stdio.h>

#include <utmp.h>

#include "cinfo.h"



int show_users(void)
{
    setutent();

    size_t count = 0;
    struct utmp *entry;

    cinfo_heading("Logged-in terminal sessions:");
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
