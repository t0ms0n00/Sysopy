clean:
	rm -f *.o
	rm -f main

compile:
	gcc -c -Wall main.c
	gcc main.o -o main -lpthread

test: compile
	./main