CFLAGS = -o2 -std=c99 -D_POSIX_C_SOURCE=199309L -Wall -pedantic -Wno-parentheses
PREFIX ?= /usr/local

all: watch debug

watch: watch.c
	$(CC) $< $(CFLAGS) -o $@

install: watch
	install watch $(PREFIX)/bin/watch

debug: watch.c
	$(CC) $< $(CFLAGS) -g -o watch_debug

uninstall:
	rm -f $(PREFIX)/bin/watch

clean:
	-rm -f watch
	-rm -f watch_debug

.PHONY: all clean install uninstall
