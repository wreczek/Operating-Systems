CC = gcc -Wall -o
	
clean:
	rm -f *.o
	rm -f main
	
compile:
	make clean
	$(CC) main main.c
	
grant:
	chmod 755 date.sh 

run:
	make compile
	./main first
	
run2:
	make compile
	./main second
