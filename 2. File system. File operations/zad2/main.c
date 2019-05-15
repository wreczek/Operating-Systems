#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ftw.h>

struct tm date_nftw;
char sign_nftw;

void error(char *msg){
    printf("%s", msg);
    exit(EXIT_FAILURE);
}

int compDates(struct tm *date1, struct tm *date2, char *sign){
    int datem1, datem2;
    datem1 = date1->tm_mday + 32*date1->tm_mon + 366*date1->tm_year;
    datem2 = date2->tm_mday + 32*date2->tm_mon + 366*date2->tm_year;

    if(strcmp(sign, "=") == 0)
        return datem1 == datem2;
    if(strcmp(sign, "<") == 0)
        return datem1 < datem2;
    if(strcmp(sign, ">") == 0)
        return datem1 > datem2;
    return 0;	// nie uwzgledniaj
}

char *type_of_file(unsigned char type) {
    switch(type) {
        case 8: return "file"; //DT_REG      8  
        case 4: return "dir"; //DT_DIR       4
        case 2: return "char dev"; //DT_CHR  2
        case 6: return "block dev"; //DT_BLK 6
        case 1: return "fifo"; //DT_FIFO     1
        case 10: return "slink"; //DT_LNK    10
        case 12: return "sock"; //DT_SOCK    12
        default: return "";
    };
}

char *type_of_file_nftw(int type){
    switch(type) {
        case FTW_F: return "file";
        case FTW_D: return "dir";
        case FTW_SL: return "link";
        default: return "";
    };
}

void _stat(char* dir, char *sign, struct tm *date){
    DIR* directory = opendir(dir);

    if(directory == NULL)
        error("Can't open directory");

    struct dirent *file;
    struct tm *modTime;
    struct tm *accTime;
    char buffer[50];
    char *absPath = (char*) calloc(100, sizeof(char));
    char *nextPath = (char*) calloc(100, sizeof(char));
    struct stat stats;

    while((file = readdir(directory))){
        strcpy(nextPath, dir);
        strcat(nextPath, "/");
        strcat(nextPath, file->d_name);
        realpath(nextPath, absPath);

        stat(nextPath, &stats);

        if(strcmp(file->d_name, ".") != 0 && strcmp(file->d_name, "..") != 0){
            modTime = localtime((const time_t *) &stats.st_mtime);
            accTime = localtime((const time_t *) &stats.st_atime);

            if(compDates(modTime, date, sign)){
                printf("\nPath:              %s\n"
                       "Type:              %s\n"
                       "Size:              %ld\n",
                       absPath, 
                       type_of_file(file->d_type), 
                       stats.st_size);
                strftime (buffer, 50 ,"Last mod.:         %d.%m.%Y\n", modTime);
                printf("%s", buffer);
                strftime (buffer, 50 ,"Last access:       %d.%m.%Y\n", accTime);
                printf("%s", buffer);
            }

            if(file->d_type == 4)
                _stat(nextPath, sign, date);
        }
    }

    free(nextPath);
    free(absPath);
    free(file);
    closedir(directory);
}

int display_info(const char *path, const struct stat *sb, int type_of_flag, struct FTW *tw_buf){
    struct tm *modTime;
    struct tm *accTime;
    char buffer[50];
    char *absPath = (char*) calloc(100, sizeof(char));

    modTime = localtime((const time_t *) &sb->st_mtime);
    accTime = localtime((const time_t *) &sb->st_atime);

    realpath(path, absPath);

    if(compDates(modTime, &date_nftw, &sign_nftw)){
        printf("\nPath:              %s\n"
               "Type:              %s\n"
               "Size:              %ld\n",
               absPath, 
               type_of_file_nftw(type_of_flag), 
               sb->st_size);
        strftime (buffer, 50 ,"Last mod.:         %d.%m.%Y\n", modTime);
        printf("%s", buffer);
        strftime (buffer, 50 ,"Last access:       %d.%m.%Y\n", accTime);
        printf("%s", buffer);
    }

    free(absPath);
    return 0;
}

void _nftw(char *dir, char *sign, struct tm *date){
    sign_nftw = *sign;
    date_nftw = *date;
    nftw(dir, display_info, 10, FTW_PHYS);
}

int main(int argc, char **argv) {
    if(argc != 4)
        error("Invalid number of arguments");

    char *path_to_dir, *sign;
    path_to_dir = argv[1];

    if(strcmp(argv[2], "<") == 0 || strcmp(argv[2], ">") == 0 || strcmp(argv[2], "=") == 0)
        sign = argv[2];
    else
    	error("Bad argv[2]");
	
    struct tm date;

    if(strptime(argv[3], "%d.%m.%Y", &date) == NULL)
        error("Bad argv[3]");

    printf("\nSearching in %s files %s %s\n", argv[1],argv[2],argv[3]);
    printf("\n############--stat--#############\n");
    _stat(path_to_dir, sign, &date);
    
    printf("\n\n############--nftw--#############\n\n");
    _nftw(path_to_dir, sign, &date);
	
    return 0;
}
