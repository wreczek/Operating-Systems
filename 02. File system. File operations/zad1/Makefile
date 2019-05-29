CC = gcc -Wall

compile:
	$(CC) -o main main.c

clean:
	rm main
	truncate -s 0 records.txt records2.txt records3.txt wyniki.txt
	#rm *.txt	
	
run:
	make clean
	make compile
	#./main generate records.txt 5 7 copy records.txt records2.txt 5 7 sys sort records2.txt 5 7 sys
	#./main generate records.txt 5 7 copy records.txt records2.txt 5 7 sys sort records2.txt 5 7 lib
	#./main generate records.txt 5 7 copy records.txt records2.txt 5 7 lib sort records2.txt 5 7 sys
	#./main generate records.txt 5 7 copy records.txt records2.txt 5 7 lib sort records2.txt 5 7 lib
	
	
	#tutaj na zajecia
	# 1B - dac wiecej bajtow zeby bylo pare sekund
	./main generate records.txt 1 200000 copy records.txt records2.txt 1 200000 lib copy records.txt records3.txt 1 200000 sys sort records2.txt 1 50000 lib sort records2.txt 1 50000 sys
	./main generate records.txt 1 400000 copy records.txt records2.txt 1 400000 lib copy records.txt records3.txt 1 400000 sys sort records2.txt 1 100000 lib sort records2.txt 1 100000 sys
	
	# 4B - wiecej bajtow 
	./main generate records.txt 4 4000 copy records.txt records2.txt 4 4000 lib copy records.txt records3.txt 4 4000 sys sort records2.txt 4 100 lib sort records2.txt 4 100 sys
	./main generate records.txt 4 8000 copy records.txt records2.txt 4 8000 lib copy records.txt records3.txt 4 8000 sys sort records2.txt 4 200 lib sort records2.txt 4 200 sys
	
	# 512B - wiecej bajtow 
	./main generate records.txt 512 4000 copy records.txt records2.txt 512 4000 lib copy records.txt records3.txt 512 4000 sys sort records2.txt 512 80 lib sort records2.txt 512 80 sys
	./main generate records.txt 512 8000 copy records.txt records2.txt 512 8000 lib copy records.txt records3.txt 512 8000 sys sort records2.txt 512 160 lib sort records2.txt 512 160 sys
	
	# 1024B - wiecej bajtow 
	./main generate records.txt 1024 4000 copy records.txt records2.txt 1024 4000 lib copy records.txt records3.txt 1024 4000 sys sort records2.txt 1024 40 lib sort records2.txt 1024 40 sys
	./main generate records.txt 1024 8000 copy records.txt records2.txt 1024 8000 lib copy records.txt records3.txt 1024 8000 sys sort records2.txt 1024 80 lib sort records2.txt 1024 80 sys
	
	# 4096B - wiecej bajtow 
	./main generate records.txt 4096 2000 copy records.txt records2.txt 4096 2000 lib copy records.txt records3.txt 4096 2000 sys sort records2.txt 4096 20 lib sort records2.txt 4096 20 sys
	./main generate records.txt 4096 4000 copy records.txt records2.txt 4096 4000 lib copy records.txt records3.txt 4096 4000 sys sort records2.txt 4096 40 lib sort records2.txt 4096 40 sys
	
	# 8192B - wiecej bajtow 
	./main generate records.txt 8192 1000 copy records.txt records2.txt 8192 1000 lib copy records.txt records3.txt 8192 1000 sys sort records2.txt 8192 10 lib sort records2.txt 8192 10 sys
	./main generate records.txt 8192 2000 copy records.txt records2.txt 8192 2000 lib copy records.txt records3.txt 8192 2000 sys sort records2.txt 8192 20 lib sort records2.txt 8192 20 sys
	rm main
