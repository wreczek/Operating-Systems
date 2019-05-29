CC = gcc -Wall -o
	
trucker_args:
	#Pass arguments:  X, K, M
	
loader_args:
	#Pass arguments:  N, loaders_num, C
	
clean:
	rm -f *.o
	rm -f trucker
	rm -f loader
	rm -f test
	
compile:
	make clean
	$(CC) trucker trucker.c
	$(CC) loader loader.c
	
run_trucker:
	make compile
	./trucker 100 16 50
	
run_loader:
	make compile
	./loader 5 4 3
