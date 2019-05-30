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
	./server 2137 "HOME"
	
runclient:
	./client client1 unix "HOME"
	#./client client2 inet 127.0.0.1 2137
