CC = gcc -Wall -o
	
clean:
	rm -f *.o
	rm -f main
	
compile:
	make clean
	$(CC) main main.c -lpthread
	
run:
	make compile
	./main
	#./main 100 5 10 20
