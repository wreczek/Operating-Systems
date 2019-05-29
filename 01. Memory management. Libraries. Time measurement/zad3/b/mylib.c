#include <ctype.h>
#include "mylib.h"

int size = 140;

struct ArrayOfBlocks createArrayOfBlocks(int size){
    struct ArrayOfBlocks array;

    array.arrayOfBlocks = (struct Block*) calloc(size, sizeof(struct Block));
    array.index = -1;

    return array;
};

struct Block setDirectoryAndFile(char *dir, char *file){
    struct Block newBlock;
    newBlock.dir = (char*) calloc(strlen(dir), sizeof(char));
    strcpy(newBlock.dir, dir);
    newBlock.file = (char*) calloc(strlen(file), sizeof(char));
    strcpy(newBlock.file, file);

    return  newBlock;
};

struct Block find(struct Block newBlock, char* name){
    char * command = calloc(size, sizeof(char));
    int i;
    for(i = 0; i < size; ++i){
        command[i] = 0;
    }
    strcpy(command, "find ");
    strcat(command, newBlock.dir);
    strcat(command, " -type f -name '");
    strcat(command, newBlock.file);
    strcat(command, "' > ");
    strcat(command, name);

    system(command);

    FILE *f = fopen(name, "r");
    if (f == NULL)
    {
        perror("Cannot open file");
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char ch;
    char * result = (char*) calloc((size_t) fileSize, sizeof(char));

    i=0;
    while(!feof(f)) {
        if((ch = (char) fgetc(f)) != -1){
            result[i++] = ch;
        }
    }
    result[i] = '\0';

    newBlock.table = (char*) calloc(strlen(result), sizeof(char));
    strcpy(newBlock.table, result);
    free(result);
    fclose(f);

    return newBlock;
};

int addToArrayOfBlocks(struct ArrayOfBlocks arrayOfBlocks, struct Block newBlock){
    arrayOfBlocks.arrayOfBlocks[arrayOfBlocks.index+1] = newBlock;
    return arrayOfBlocks.index+1;
};

void removeBlock(struct ArrayOfBlocks arrayOfBlocks, int index){
    free(arrayOfBlocks.arrayOfBlocks[index].table);
    free(arrayOfBlocks.arrayOfBlocks[index].file);
    free(arrayOfBlocks.arrayOfBlocks[index].dir);
};

int isNumber(char* s)
{
    int i;
    for (i = 0; i < strlen(s); i++)
        if (!isdigit(s[i]))
            return 0;
    return 1;
}
