compile:
	gcc -c -Wall main.c
	gcc main.o -o main

clean:
	rm -f *.o
	rm -f main

test:
	./main dir "pattern exists" 5
	./main dir process 5
	./main dir "pattern exists" 2
	./main dir process 2
	./main dir/dir4/dir7 "pattern exists" 2
	./main dir/dir4/dir7 process 2