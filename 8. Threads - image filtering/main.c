#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
//#include <zconf.h>
#include <pthread.h>
//#include <sys/times.h>

int thread_number;
char * way_of_division;     // block / interleaved
char * image_file_name;
char * filter_file_name;
char * result_file_name;    // Times.txt

void error(char * msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
/** Testy przeprowadziæ dla:
    - 1, 2, 4, 8 w¹tków
    - 3 <= c <= 65, c to rozmiar filtrów
    - block oraz interleaved
*/
    if (argc != 6) /* --> */ error("bad args count");

    thread_number    = atoi(argv[1]);
    way_of_division  = argv[2];
    image_file_name  = argv[3];
    filter_file_name = argv[4];
    result_file_name = argv[5];

    if (strcmp(way_of_division, "block") == 0){
        printf("block\n");
    }
    else if (strcmp(way_of_division, "interleaved") == 0){
        printf("interleaved\n");
    }

    return 0;
}
