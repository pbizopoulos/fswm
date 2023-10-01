.POSIX:

CC = gcc
CFLAGS = -g -Ofast -std=c89 -Wall -Wconversion -Werror -Wextra -Wmissing-prototypes -Wold-style-definition -Wpedantic -Wstrict-prototypes
LDLIBS = -lxcb -lxcb-keysyms
PREFIX = /usr/local
PRG = fswm

all: $(PRG)

check:

clean:
	rm -f $(PRG)

install: $(PRG)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(PRG) $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/$(PRG)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(PRG)

.gitignore:
	printf '$(PRG)\n' > $@

$(PRG): $(PRG).c
	$(CC) $(CFLAGS) $(LDLIBS) -o $@ $(PRG).c
