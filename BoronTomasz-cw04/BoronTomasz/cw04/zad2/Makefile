compile:
	make clean
	gcc -c -Wall main.c
	gcc main.o -o main

test:
	./main nocldwait
	./main nocldstop
	./main siginfo

clean:
	rm -f *.o
	rm -f main