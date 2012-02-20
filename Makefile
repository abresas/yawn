CFLAGS+= -Wall -Os
LDADD+= -lxcb -lxcb-keysyms
LDFLAGS=
EXEC=yawn

PREFIX?= /usr
BINDIR?= $(PREFIX)/bin

XDG_CONFIG_DIRS?=/etc

# CC=gcc
CC=clang

all: $(EXEC)

yawn: action.o client.o configuration.o desktop.o event.o keyboard.o stack.o string.o window.o yawn.o
	$(CC) $(LDFLAGS) -o $@ $+ $(LDADD)

install: all
	install -Dm 755 yawn $(DESTDIR)$(BINDIR)/yawn
	install -b -D -m755 yawn.conf $(XDG_CONFIG_DIRS)/yawn/yawn.conf

clean:
	rm -f yawn *.o
