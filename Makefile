CC       = gcc
VERSION := ${shell cat VERSION}
CFLAGS  := $(CFLAGS) -g -Wall -Wextra -std=gnu99 -D'VERSION="$(VERSION)"'

OBJS = navis.o helper.o

.PHONY: all clean distclean
all: navis error_pages

navis: $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -o navis

navis.o: navis.c navis.h
helper.o: helper.c helper.h

install: navis
	install -D -m755 navis $(PREFIX)/bin/navis

uninstall:
	rm -rf $(PREFIX)/bin/navis

error_pages: build_error_pages.pl
	perl build_error_pages.pl $(VERSION)

clean:
	rm -rf *.o

distclean: clean
	rm -rf navis
