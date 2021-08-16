compile:
	make clean
	gcc -c -Wall catcher.c
	gcc -c -Wall sender.c
	gcc catcher.o -o catcher
	gcc sender.o -o sender

clean:
	rm -f *.o
	rm -f sender
	rm -f catcher