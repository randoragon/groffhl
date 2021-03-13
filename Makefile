CC = gcc
CFLAGS = -std=c89 -pedantic -Wall
OBJS = groffhl.o
TARGET = groffhl
DESTDIR =
PREFIX = /usr/local/bin

all: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS += -g -Og
debug: clean all

install: CFLAGS += -O3
install: clean all
	cp -- $(TARGET) $(DESTDIR)$(PREFIX)/

clean:
	rm -f -- *.o
