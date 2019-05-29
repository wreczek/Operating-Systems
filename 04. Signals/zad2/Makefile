CC = gcc -Wall

clean:
	rm -f *.o
	rm -f main
	rm -f tester
	rm -f archiwum/*
	truncate -s 0 obiekt*

compileMain:
	$(CC) -o main main.c
	
compileTester:
	$(CC) -o tester tester.c
	
run:
	make clean
	make compileMain
	./main lista.txt
	
runTester:
	make compileTester
	./tester obiekt.txt 1 5 10 30
	#./tester obiekt1.txt 2 3 5 30
	#./tester obiekt2.txt 5 7 1 30
	#./tester obiekt3.txt 0 10 17 30
