CC = gcc -Wall -o
	
clean:
	rm -f main
	rm -f *.o
	rm -f *.so
	rm -f *.a
	
compile:
	make clean
	$(CC) server server.c -lpthread
	$(CC) client client.c -lpthread
	
runserver:
	./server
	
runclient:
	./client