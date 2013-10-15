CC       = gcc
VERSION := ${shell cat VERSION}
CFLAGS  := $(CFLAGS) -g -Wall -Wextra -std=gnu99 -D'VERSION="$(VERSION)"'

OBJS = navis.o conf.o helper.o

.PHONY: all install uninstall clean distclean
all: navis

navis: $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -o navis

navis.o: navis.c navis.h
helper.o: helper.c helper.h
conf.o: conf.c conf.h

install: navis
	install -D -m755 navis $(PREFIX)/bin/navis

uninstall:
	rm -rf $(PREFIX)/bin/navis

clean:
	rm -rf *.o

distclean: clean
	rm -rf navis
