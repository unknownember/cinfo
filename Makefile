CC ?= cc
CFLAGS ?= -Wall -Wextra -O3

all: cinfo

cinfo: cc.c
	$(CC) $(CFLAGS) $(LDFLAGS) cc.c -o cinfo

install: cinfo
	install -Dm755 cinfo $(DESTDIR)/usr/bin/cinfo

clean:
	rm -f cinfo
