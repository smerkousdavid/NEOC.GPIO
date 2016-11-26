SHELL = /bin/sh
CC = gcc
CFLAGS = -fPIC -Wall -Wextra -O2 -g -Iinclude -I/usr/include -lpthread
LDFLAGS = -shared
RM = rm -f
TARGET_LIB = libneo.so

SRCS = $(shell echo src/*.c)
OBJS = $(SRCS:.c=.o)
LIBS = lib/

PREFIX = $(DESTDIR)/usr/local
BINDIR = $(PREFIX)/bin

.PHONY: all
all: ${TARGET_LIB}

$(LIBS)$(TARGET_LIB): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

$(SRCS:.c=.d):%.d:%.c
	$(CC) $(CFLAGS) -MM $< >$@

include $(SRCS:.c=.d)

.PHONY: clean
clean:
	-${RM}  ${OBJS} $(SRCS:.c=.d)


