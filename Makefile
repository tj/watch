
CFLAGS = -std=c99 -D_POSIX_C_SOURCE=199309L -Wall -pedantic
PREFIX = /usr/local

watch: src/watch.c
	$(CC) $< $(CFLAGS) -o $@

install: watch
	install watch $(PREFIX)/bin/watch

uninstall:
	rm -f $(PREFIX)/bin/watch

clean:
	rm -f watch

.PHONY: clean install uninstall
