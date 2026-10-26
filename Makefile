CC ?= cc
CFLAGS ?= -Wall -Wextra -Wpedantic -O3
CPPFLAGS ?=
LDFLAGS ?=
CPPFLAGS += -D_POSIX_C_SOURCE=200809L -Iinclude

SOURCES = src/main.c src/system.c src/cpu.c src/memory.c src/load.c src/disk.c src/devices.c src/users.c src/network.c

.PHONY: all install clean test

all: cinfo

cinfo: $(SOURCES) include/cinfo.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SOURCES) $(LDFLAGS) -o cinfo

install: cinfo
	install -Dm755 cinfo $(DESTDIR)/usr/bin/cinfo

clean:
	rm -f cinfo

test: cinfo
	sh tests/cli.sh
