.POSIX:

CC = cc
CFLAGS = -O3 -std=c89 -Wall -Wconversion -Werror -Wextra -Wmissing-prototypes -Wold-style-definition -Wpedantic -Wstrict-prototypes
LDLIBS = -lxcb -lxcb-keysyms
PREFIX = /usr/local

all: fswm

check:

clean:
	rm -f fswm

install: fswm
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f fswm ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/fswm

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/fswm

fswm: fswm.c
	${CC} ${CFLAGS} ${LDLIBS} -o $@ $<
