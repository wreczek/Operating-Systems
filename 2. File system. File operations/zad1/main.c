#include <stdio.h> // printf, scanf, NULL
#include <fcntl.h>  // O_RDONLY
#include <sys/stat.h> // open
#include <unistd.h> // no read warning
#include <stdlib.h> // calloc, exit, free
#include <string.h> // strcpy
#include <time.h>
#include <sys/times.h>

#define size 100

FILE * file1;   // records.txt
FILE * file2;   // recordsCopy.txt
FILE * resultFile;
// wyniki.txt - czasy

int error(char * msg){
    perror(msg);
    return 1;
}

double difference(clock_t t1, clock_t t2){
    return ((double)(t2 - t1) / sysconf(_SC_CLK_TCK));
}

void writeResult(struct tms* t_start,struct tms* t_end){
    fprintf(resultFile,"\tUSER_TIME: %fl\n", difference(t_start->tms_utime,t_end->tms_utime));
    fprintf(resultFile,"\tSYSTEM_TIME: %fl\n\n", difference(t_start->tms_stime,t_end->tms_stime));
}

void generate(char * name, char * records, char * length){
    int i;
    char * command = calloc(size, sizeof(char));
    for(i = 0; i < size; ++i){
        command[i] = 0;	
    }
    strcpy(command, "head -c 1000000000 /dev/urandom | tr -dc 'a-z'");
    strcat(command, " | fold -w ");  // A-Z0-9~!@#$%^&*_-' | fold -w ");
    strcat(command, length);
    strcat(command, " | head -n ");
    strcat(command, records);
    strcat(command, " > ");//records.txt");
    strcat(command, name);
    system(command);

    free(command);
}

char * getBlock_sys(int fd, int index, int length){
    char * block = calloc(length, sizeof(char));
    lseek(fd, (length+1)*index, 0);
    read(fd, block, length);
    return block;
}

void writeBlock_sys(int fd, char * block, int index, int length){
    lseek(fd, (length+1)*index, 0);
    write(fd, block, length);
}

void sort_sys(char * fileName, int records, int length){
    int i;
    int fd = open(fileName, O_RDWR);
    if (fd < 0){
        error("open");
    }

    int j;
    for (i = 0; i < records-1; ++i){
        char * block1 = getBlock_sys(fd, i, length);

        char minChar = block1[0];
        int minInd = 0;

        for (j = i+1; j < records; ++j){
            char * block2 = getBlock_sys(fd, j, length);

            if (block2[0] <= minChar){
                minChar = block2[0];
                minInd = j;
            }
        }
        if (minChar < block1[0]){
            writeBlock_sys(fd, getBlock_sys(fd, minInd, length), i, length);
            writeBlock_sys(fd, block1, minInd, length);
        }
    }

    close(fd);
}

char * getBlock_lib(FILE * file, int index, int length){
    char * block = calloc(length, sizeof(char));
    fseek(file, (length+1)*index, 0);
    fread(block, sizeof(char), length, file);
    return block;
}

void writeBlock_lib(FILE * file, char * block, int index, int length){
    fseek(file, (length+1)*index, 0);
    fwrite(block, sizeof(char), length, file);
}

void sort_lib(char * fileName, int records, int length){
    int i;
    FILE * file = fopen(fileName, "r+");
    if (!file){
        error("fopen");
    }

    int j;
    for (i = 0; i < records-1; ++i){
        char * block1 = getBlock_lib(file, i, length);
        int minInd = 0;
        int minChar = block1[0];

        for (j = i+1; j < records; ++j){
            char * block2 = getBlock_lib(file, j, length);

            if (block2[0] <= minChar){
                minChar = block2[0];
                minInd = j;
            }
        }
        if (minChar < block1[0]){
            writeBlock_lib(file, getBlock_lib(file, minInd, length), i, length);
            writeBlock_lib(file, block1, minInd, length);
        }
    }

    fclose(file);
}

void copy_sys(char * fileName1, char * fileName2, int records, int length){
    int fd1, fd2;
    if ( (fd1 = open(fileName1, O_RDONLY)) < 0){
        error("copy_sys open1");
    }
    if ( (fd2 = open(fileName2, O_WRONLY)) < 0){ // O_CREAT
        error("copy_sys open2");
    }
    char buff[length+1];

    int i, rd, wr;
    for (i = 0; i < records; ++i){
        if ( (rd = read(fd1, buff, length+1)) != length+1){
            error("copy_sys read");
        }
        if ( (wr = write(fd2, buff, length+1)) != length+1){
            error("copy_sys write");
        }

    }

    close(fd1);
    close(fd2);
}

void copy_lib(char * fileName1, char * fileName2, int records, int length){
    int i;
    char * buff = calloc(length+1, sizeof(char));
    for (i = 0; i < length+1; i++){
        buff[i] = 0;
    }

    if ( !(file1 = fopen(fileName1, "r")) ){
        error("fopen file1");
    }

    if ( !(file2 = fopen(fileName2, "w")) ){
        error("fopen file1");
    }

    i = 0;
    size_t readSize;
    size_t writeSize;
    while (i < records){
        ++i;
        if ( (readSize = fread(buff, sizeof(char), length+1, file1)) != length + 1){
            error("readSize");
        }
        //printf("\nRead: %d, ", readSize);
        if ( (writeSize = fwrite(buff, sizeof(char), length+1, file2)) != length+1){
            error("writeSize");
        }
        //printf("write: %d", writeSize);
    }

    fclose(file1);
    fclose(file2);

    free(buff);
}

void copy_file(char * fileName1, char * fileName2){
    if ( !(file1 = fopen(fileName1, "r")) ){
        error("copy_file fopen1");
    }

    if ( !(file2 = fopen(fileName2, "r")) ){
        error("copy_file fopen2");
    }

    char sign = getc(file1);
    while (sign != EOF){
        fprintf(file2, "%c", sign);
        sign = getc(file1);
    }
    fclose(file1);
    fclose(file2);
}

int main(int argc, char *argv[]){
    struct tms *tms[argc+1];
    int i;
    for (i = 0; i < argc; i++){
        tms[i] = calloc(1, sizeof(struct tms *));
    }

    resultFile = fopen("wyniki.txt", "a");
    int current = 0;

	for (i = 0; i < argc; i++)
		printf("%d: %s\n", i, argv[i]);

    i = 1;
    while (i < argc){
        if (strcmp(argv[i], "generate") ==  0){
            /// name = i+1, records = i+2, length = i+3
            if ( argc < i+3)    error("Bad arg");
            times(tms[current]);
            generate(argv[i+1], argv[i+2], argv[i+3]);
            times(tms[current+1]);
            fprintf(resultFile, "Generate %s records each %s bytes long:\n", argv[i+2], argv[i+3]);
            writeResult(tms[current], tms[current+1]);
            i += 4;
	    current += 2;
        }
        else if (strcmp(argv[i], "sort") == 0){
            /// name = i+1, records = i+2, length = i+3, opt = i+4
            if (argc < i+4)     error("Bad arg2");
            if (strcmp(argv[i+4], "sys") == 0){
		times(tms[current]);
                sort_sys(argv[i+1], atoi(argv[i+2]), atoi(argv[i+3]));
		times(tms[current+1]);
		fprintf(resultFile, "Sort_%s %s with %s records each %s bytes long:\n", argv[i+4], argv[i+1], argv[i+2], argv[i+3]);
                writeResult(tms[current], tms[current+1]);
	        current +=2;
            }
            else if (strcmp(argv[i+4], "lib") == 0){
                times(tms[current]);
                sort_lib(argv[i+1], atoi(argv[i+2]), atoi(argv[i+3]));
                times(tms[current+1]);
		fprintf(resultFile, "Sort_%s %s with %s records each %s bytes long:\n", argv[i+4], argv[i+1], argv[i+2], argv[i+3]);
                writeResult(tms[current], tms[current+1]);
	    }
            else    error("Bad arg3");
            i += 5;
	    current += 2;
        }
        else if (strcmp(argv[i], "copy") == 0){
            /// name1 = i+1, name2 = i+2, rec = i+3, length = i+4, opt=i+5
            if (argc < i+5)     error("Bad arg4");
            if (strcmp(argv[i+5], "sys") == 0){
                times(tms[current]);
                copy_sys(argv[i+1], argv[i+2], atoi(argv[i+3]), atoi(argv[i+4]));
                times(tms[current+1]);
		fprintf(resultFile, "Copy_%s %s to %s with %s records each %s bytes long:\n", argv[i+5], argv[i+1], argv[i+2], argv[i+3], argv[i+4]);
	 	writeResult(tms[current], tms[current+1]);
            }
            else if (strcmp(argv[i+5], "lib") == 0){
                times(tms[current]);
                copy_lib(argv[i+1], argv[i+2], atoi(argv[i+3]), atoi(argv[i+4]));
                times(tms[current+1]);
		fprintf(resultFile, "Copy_%s %s to %s with %s records each %s bytes long:\n", argv[i+5], argv[i+1], argv[i+2], argv[i+3], argv[i+4]);
		writeResult(tms[current], tms[current+1]);
            }
            else       error("Bad arg5");
            i += 6;
	    current += 2;
        }
        else if (strcmp(argv[i], "copy_file") == 0){
            /// name1 = i+1, name2 = i+2
            copy_file(argv[i+1], argv[i+2]);
            i += 3;
        }
        else {
            error("Bad argument");
	    //printf("%d\n", i);        
	}
    }
    fclose(resultFile);
    return 0;
}
