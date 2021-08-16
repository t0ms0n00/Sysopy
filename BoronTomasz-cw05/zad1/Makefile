compile:
	gcc -c -Wall main.c
	gcc main.o -o main

clean:
	rm -f *.o
	rm -f main

test:
	make compile
	./main data.txt