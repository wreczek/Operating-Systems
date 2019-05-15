CC = gcc -Wall

clean:
	rm -f *.o
	rm -f monitor
	rm -f tester
	rm -f archiwum/*
	truncate -s 0 obiekt*

compileMonitor:
	$(CC) -o monitor monitor.c
	
compileTester:
	$(CC) -o tester tester.c
	
runArch:
	make clean
	make compileMonitor
	./monitor lista.txt 30 arch

runExec:
	make clean
	make compileMonitor
	./monitor lista.txt 30 exec
	
runTester:
	make compileTester
	./tester obiekt.txt 1 5 10 30
	#./tester obiekt1.txt 2 3 5 30
	#./tester obiekt2.txt 5 7 1 30
	#./tester obiekt3.txt 0 10 17 30
