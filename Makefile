
CC = gcc
CFLAGS = -O3 -std=c99 -pedantic -pedantic-errors -fno-exceptions \
 -Wl,-z,relro -Wl,-z,now -fvisibility=hidden -W -Wall \
 -Wno-unused-parameter -Wno-unused-function -Wno-unused-label \
 -Wpointer-arith -Wformat -Wreturn-type -Wsign-compare -Wmultichar \
 -Wformat-nonliteral -Winit-self -Wuninitialized -Wno-deprecated \
 -Wformat-security -Werror -Wformat=2 -Wno-format-nonliteral -Wshadow \
 -Wpointer-arith -Wcast-qual -Wmissing-prototypes -Wno-missing-braces
LINK = gcc
LINKFLAGS = -Wl,-O1 -Wl,--discard-all -Wl,--no-undefined

all: bin/pltest.exe

bin/pltest.exe: bin/pl.o bin/dpll.o bin/pltest.o
	$(LINK) $(LINKFLAGS) -o $@ $^

bin/%.o: src/%.c
	mkdir -p bin
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -rf *.exe
	rm -rf *.o
