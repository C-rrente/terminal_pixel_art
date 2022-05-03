CC = g++
CCFLAGS = -Wall -Wextra -Wpedantic -pedantic-errors -Wno-unused-parameter -std=c++2a -g -O2 -fmax-errors=1 -D_GLIBCXX_DEBUG -fsanitize=undefined -fsanitize=address

all: a.out

a.out: main.cc
	$(CC) $(CCFLAGS) main.cc

clean:
	rm -f a.out core *.o
