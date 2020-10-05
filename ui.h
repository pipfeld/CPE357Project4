#ifndef UI_H
#define UI_H
#include "getWord.h"
#include "hashTable.h"

typedef struct node{
   HTEntry *entry;
   struct node *next;
}HashNode;

typedef struct{
   HTFunctions *funcs;
   unsigned *sizes;
   int numSizes;
   int rehash;
   unsigned unique;
   unsigned total;
   float rehashLoadFactor;
   HashNode **theArray;
}HashTable;

void usageError();

void checkPtr(void *ptr, const char *fname);

FILE* fileOpen(const char *fname);

FILE* parseArgs(char argument[], int *n, int *filePresent);

void printSpecs(void *ht, int n);

void readFile(void *ht, FILE *fp, Byte **word, unsigned *wordLength, int *hasPrintable);

void readStdin(void *ht, int filePresent, Byte **word, unsigned *wordLength, int *hasPrintable);

#endif
