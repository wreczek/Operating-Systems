#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char ** argv){ 
	char nazwa[100];
	strcpy(nazwa, "cat ");
	strcat(nazwa, argv[1]);
	strcat(nazwa, " | sort ");  
	
	if (argc == 2){
		FILE * result = popen(nazwa, "w");
		fseek(result, 0L, SEEK_END);
		int size = ftell(result);
		fseek(result, 0L, SEEK_SET);
		
		char * buffer = (char*)calloc(size, sizeof(char));
		fread(buffer, sizeof(char), size, result);
		printf("%s", buffer);
		
		pclose(result);
	}
	else if (argc == 3){
		strcat(nazwa, "> ");  
		strcat(nazwa, argv[2]);
		
		FILE * result = popen(nazwa, "w");
		
		pclose(result);
	}
	else 
		printf("BLAD");
}

/// WERSJA 1
/// cat plik | sort A wykonac i wyswietlic wynik
