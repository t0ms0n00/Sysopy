compile:
	gcc -c -Wall main.c
	gcc main.o -o main -lrt -lpthread
	gcc -c -Wall chef.c
	gcc chef.o -o chef -lrt -lpthread
	gcc -c -Wall supplier.c
	gcc supplier.o -o supplier -lrt -lpthread

test_small: compile
	./main 3 3

test_big: compile
	./main 8 8

clean:
	rm -f main
	rm -f chef
	rm -f supplier
	rm -f *.o