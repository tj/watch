
CFLAGS = -std=c99 -D_POSIX_C_SOURCE=200809L
CFLAGS += -Wall -pedantic -Wno-parentheses -Ideps
PREFIX ?= /usr/local
SRC = src/watch.c
DEPS = $(wildcard deps/*/*.c)
OBJS = $(SRC:.c=.o) $(DEPS:.c=.o)

all: watch

watch: $(OBJS)
	$(CC) $^ $(CFLAGS) -o $@

install: watch
	install watch $(PREFIX)/bin/watch

uninstall:
	rm -f $(PREFIX)/bin/watch

clean:
	rm -f watch $(OBJS)

.PHONY: all clean install uninstall
