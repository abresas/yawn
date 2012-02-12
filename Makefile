CFLAGS+= -Wall
LDADD+= -lxcb
LDFLAGS=
EXEC=yawn

PREFIX?= /usr
BINDIR?= $(PREFIX)/bin

CC=gcc

all: $(EXEC)

yawn: yawn.o
	$(CC) $(LDFLAGS) -Os -o $@ $+ $(LDADD)

install: all
	install -Dm 755 yawn $(DESTDIR)$(BINDIR)/yawn

clean:
	rm -f yawn *.o
