CC = gcc -Wall -o 
	
clean:
	rm -f master
	rm -f slave
	rm -f *.o
	rm -f *.a
	rm -f nazwa	
	
compile:
	make clean
	$(CC) slave slave.c
	$(CC) master master.c
	
runmaster:
	make compile
	./master ./nazwa
	
runslave:
	./slave ./nazwa 10
