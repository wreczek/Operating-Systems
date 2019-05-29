CC = gcc -Wall -o

clean:
	rm -f main
	rm -f *.o

compile:
	make clean
	$(CC) main main.c -lpthread -lm

run:
	./main 1 interleaved apple.pgm distorted_c3.txt apple_res.pgm 
	./main 2 interleaved apple.pgm distorted_c3.txt apple_res.pgm 
	./main 4 interleaved apple.pgm distorted_c3.txt apple_res.pgm 
	./main 8 interleaved apple.pgm distorted_c3.txt apple_res.pgm 
	
	./main 1 block apple.pgm distorted_c3.txt apple_res.pgm 
	./main 2 block apple.pgm distorted_c3.txt apple_res.pgm 
	./main 4 block apple.pgm distorted_c3.txt apple_res.pgm 
	./main 8 block apple.pgm distorted_c3.txt apple_res.pgm 
