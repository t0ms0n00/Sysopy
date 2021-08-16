compile:
	gcc -c -Wall producer.c
	gcc -c -Wall consumer.c
	gcc -c -Wall main.c
	gcc producer.o -o producer
	gcc consumer.o -o consumer
	gcc main.o -o main

clean:
	rm -f *.o
	rm -f producer
	rm -f consumer
	rm -f main
	rm -f fifo
	rm -f consumer1.txt
	rm -f consumer2.txt
	rm -f consumer3.txt
	rm -f producer0.txt
	rm -f producer1.txt
	rm -f producer2.txt
	rm -f producer3.txt
	rm -f producer4.txt
	rm -f producer5.txt

test_a_short:
	./main 5 1 consumer1.txt

test_a_mid:
	./main 50 1 consumer2.txt

test_a_long:
	./main 5000 1 consumer3.txt

test_b_short:
	./main 5 2 consumer1.txt

test_b_mid:
	./main 50 2 consumer2.txt

test_b_long:
	./main 5000 2 consumer3.txt

test_c_short:
	./main 5 3 consumer1.txt

test_c_mid:
	./main 50 3 consumer2.txt

test_c_long:
	./main 5000 3 consumer3.txt