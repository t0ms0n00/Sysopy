clean:
	rm -f *.o
	rm -f main

compile:
	gcc -c -Wall main.c
	gcc main.o -o main -lpthread

test_1:
	./main 4 numbers barbara.ascii.pgm output.pgm

test_2:
	./main 4 block barbara.ascii.pgm output.pgm