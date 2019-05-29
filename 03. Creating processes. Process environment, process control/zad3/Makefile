CC = gcc -Wall

clean:
	rm -f *.o
	rm -f monitor
	rm -f tester
	rm -f archiwum/*
	truncate -s 0 obiekt*

compileMonitor:
	$(CC) -o main main.c
		
runArch:
	make clean
	make compileMonitor
	./main lista.txt 30 arch 20 1
	#./main lista.txt 30 arch 200 200

runExec:
	make clean
	make compileMonitor
	./main lista.txt 30 exec 20 1
	#./main lista.txt 30 exec 200 200
