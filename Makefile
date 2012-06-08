
CFLAGS = -std=c99 -D_POSIX_C_SOURCE=199309L -Wall -pedantic -Wno-parentheses
PREFIX ?= /usr/local

all: watch

watch: src/watch.c
	$(CC) $< $(CFLAGS) -o $@

install: watch
	install watch $(PREFIX)/bin/watch

uninstall:
	rm -f $(PREFIX)/bin/watch

clean:
	rm -f watch

.PHONY: all clean install uninstall
