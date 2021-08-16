compile:
	gcc -c -Wall main.c
	gcc main.o -o main

clean:
	rm -f *.o
	rm -f main

test_write:
	@echo "Put user: "; \
	read USER; \
	./main $$USER TEMAT TRESC

test_read:
	./main sender
	./main date