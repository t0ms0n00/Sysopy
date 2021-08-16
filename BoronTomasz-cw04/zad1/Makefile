fork:
	make clean
	gcc -c -Wall main.c
	gcc main.o -o main

exec:
	make clean
	gcc -c -Wall main_exec.c
	gcc main_exec.o -o main_exec

test_fork:
	./main ignore
	./main handler
	./main mask
	./main pending

test_exec:
	./main_exec ignore false
	./main_exec mask false
	./main_exec pending false

clean:
	rm -f *.o
	rm -f main
	rm -f main_exec