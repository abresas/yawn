CFLAGS+= -Wall
LDADD+= -lxcb -lxcb-keysyms
LDFLAGS=
EXEC=yawn

PREFIX?= /usr
BINDIR?= $(PREFIX)/bin

XDG_CONFIG_DIRS?=/etc

# CC=gcc
CC=clang

all: $(EXEC)

yawn: yawn.o
	$(CC) $(LDFLAGS) -Os -o $@ $+ $(LDADD)

install: all
	install -Dm 755 yawn $(DESTDIR)$(BINDIR)/yawn
	install -b -D -m755 yawn.conf $(XDG_CONFIG_DIRS)/yawn/yawn.conf

clean:
	rm -f yawn *.o
