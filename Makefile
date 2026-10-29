CC ?= cc
CFLAGS ?= -Wall -Wextra -Wpedantic -O3
LDFLAGS ?=

.PHONY: all install clean

all: cinfo

cinfo: cc.c
	$(CC) $(CFLAGS) $(LDFLAGS) cc.c -o cinfo

install: cinfo
	install -Dm755 cinfo $(DESTDIR)/usr/bin/cinfo

clean:
	rm -f cinfo

.PHONY: test

test: cinfo
	sh tests/cli.sh
	sh tests/distro.sh
	sh tests/gpu.sh
	sh tests/memory.sh
	sh tests/load.sh
	sh tests/uptime.sh
