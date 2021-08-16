compile:
	gcc -c -Wall client.c
	gcc -c -Wall server.c
	gcc client.o -o client -lrt
	gcc server.o -o server -lrt

clean:
	rm -f *.o
	rm -f client
	rm -f server