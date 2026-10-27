#define _POSIX_C_SOURCE 200809L

#include <stdio.h>

#include <stdlib.h>

#include <ifaddrs.h>

#include <arpa/inet.h>

#include <netinet/in.h>

#include "cinfo.h"



int show_network(void)
{
    struct ifaddrs *interfaces = NULL;
    if (getifaddrs(&interfaces) != 0) {
        perror("cinfo: getifaddrs");
        return 1;
    }

    cinfo_heading("Network interfaces:");
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
