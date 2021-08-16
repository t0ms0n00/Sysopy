clean:
	rm -f *.o
	rm -f client
	rm -f server

compile:
	gcc -c -Wall client.c
	gcc -c -Wall server.c
	gcc server.o -o server -lpthread
	gcc client.o -o client -lpthread

test: compile
	./server 7774 /tmp/socket