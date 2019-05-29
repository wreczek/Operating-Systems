CC = gcc -Wall
	
clean:
	rm -f server
	rm -f client
	rm -f *.o
	rm -f *.so
	rm -f *.a
	
compile:
	make clean
	$(CC) server.c lib.c -o server -lrt -lpthread
	$(CC) client.c lib.c -o client -lrt -lpthread
	
runserver:
	./server
	
runclient:
	./client