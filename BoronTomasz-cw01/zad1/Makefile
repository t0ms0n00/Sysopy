static:
	make clean
	gcc -Wall -c main.c
	gcc -Wall -c array_blocks.c
	ar rcs libarray_blocks.a array_blocks.o
	gcc -Wall main.c -o main -L. -l	array_blocks

shared:
	make clean
	gcc -Wall -c main.c
	gcc -Wall -c array_blocks.c
	gcc -Wall -shared -fPIC -o libarray_blocks.so array_blocks.o	
	gcc -Wall main.c -o main -L. -l array_blocks -Wl,-rpath=`pwd`

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f main
	rm -f *.txt