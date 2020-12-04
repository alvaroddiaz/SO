CCFLAGS = -Wall -g



shell: list.o shell.o

	gcc $(CCFLAGS) -o shell list.o shell.o



list.o:  list.h list.c

	gcc $(CCFLAGS) -c list.c



shell.o: list.h shell.c

	gcc $(CCFLAGS) -c shell.c



shell-static: list.o shell.o

	gcc $(CCFLAGS) -o shell-static -static list.o shell.o





all: clean list.o shell





clean:

	rm shell *.o



cleanstatic:

	rm shell-static