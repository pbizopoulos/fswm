.POSIX:

CC = gcc
CFLAGS = -std=c89 -Wall -Wextra -Werror -Wpedantic -O3
LDLIBS = -lxcb
PREFIX = /usr/local
PRG = fswm

$(PRG): $(PRG).c Makefile
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

install: $(PRG)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(PRG) $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/$(PRG)
	mkdir -p $(DESTDIR)$(PREFIX)/share/man/man1
	cp -f $(PRG).1 $(DESTDIR)$(PREFIX)/share/man/man1/$(PRG).1
	chmod 644 $(DESTDIR)$(PREFIX)/share/man/man1/$(PRG).1

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(PRG)
	rm -f $(DESTDIR)$(PREFIX)/share/man/man1/$(PRG).1

clean:
	rm -f $(PRG)
