#ifndef mylib_h
#define mylib_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ArrayOfBlocks{
    int index;
    struct Block *arrayOfBlocks;
};

struct Block{
    char *dir;
    char *file;
    char *table;
};

struct ArrayOfBlocks createArrayOfBlocks(int size);
struct Block setDirectoryAndFile(char *dir, char *file);
struct Block find(struct Block newBlock, char *name);
int addToArrayOfBlocks(struct ArrayOfBlocks arrayOfBlocks, struct Block newBlock);
void removeBlock(struct ArrayOfBlocks arrayOfBlocks, int index);
int isNumber(char* s);

#endif //mylib_h
