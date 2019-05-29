CC = gcc -Wall -o
flags = -lpthread -lrt
	
clean:
	rm -f loader
	rm -f trucker
	rm -f test
	rm -f *.o
	
compile:
	$(CC) loader loader.c $(flags)
	$(CC) trucker trucker.c $(flags)
	
run_trucker:
	./trucker 100 16 50
	
run_loader:
	./loader 5 4 3
