CC = gcc -Wall -o

make clean:
	rm -f *.o
	rm -f *.a
	rm -f main
	rm -f catcher
	rm -f sender

make compile:
	$(CC) sender sender.c && $(CC) catcher catcher.c

make kill:
	./catcher KILL

make queue:
	./catcher SIGQUEUE

make rt:
	./catcher SIGRT
