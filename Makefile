CC = gcc
CFLAGS = -std=c89 -pedantic -Wall
OBJS = groffhl.o
TARGET = groffhl
DESTDIR =
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

VERSION = 1.0

all: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS += -g -Og
debug: clean all

install: CFLAGS += -O3
install: clean all
	@mkdir -p -- $(DESTDIR)$(PREFIX)/bin
	cp -- $(TARGET) $(DESTDIR)$(PREFIX)/bin
	@chmod 755 -- $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	@mkdir -p -- $(DESTDIR)$(MANPREFIX)/man1
	sed "s/VERSION/$(VERSION)/g" < groffhl.1 > $(DESTDIR)$(MANPREFIX)/man1/groffhl.1
	@chmod 644 -- $(DESTDIR)$(MANPREFIX)/man1/groffhl.1

uninstall:
	rm -f -- $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	rm -f -- $(DESTDIR)$(MANPREFIX)/man1/groffhl.1

clean:
	rm -f -- *.o
