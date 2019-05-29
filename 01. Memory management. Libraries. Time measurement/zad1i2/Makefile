CC = gcc -Wall

static:
	make clean
	$(CC) -c mylib.c
	ar crs libmylib.a mylib.o
	$(CC) main.c -o main -L. -lmylib

shared:
	make clean
	$(CC) -fPIC -c mylib.c
	$(CC) -shared -fPIC -o libmylib.so mylib.o
	$(CC) main.c -o main -L. -lmylib -Wl,-rpath=`pwd`

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f main
	rm -f *.txt

run:
	@echo -------Small-------
	./main 4 create_table 2 search_directory ../ '*' my1_1.txt  remove_block 1 remove_block 0
	./main 5 create_table 5 search_directory ../ '*' my1_2.txt remove_block 0 remove_block 1 remove_block 2 
	
	@echo -------Medium-------
	./main 4 create_table 2 search_directory ../../../ '*' my2_1.txt remove_block 1 remove_block 0
	./main 5 create_table 5 search_directory ../../../ '*' my2_2.txt remove_block 0 remove_block 1 remove_block 2 
	
	@echo -------Large-------
	./main 4 create_table 2 search_directory /lib '*' my3_1.txt remove_block 1 remove_block 0
	./main 5 create_table 5 search_directory / '*' my3_2.txt 2> error.txt remove_block 0 remove_block 1 remove_block 2 

