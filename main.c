#include <stdio.h>
#include <string.h>
#include "hashTable.h"
#include "getWord.h"
#include "ui.h"
static unsigned hashString(const void *data){
   unsigned hash;
   const char *str = (const char *)(((Word *)data)->bytes);
   int length = ((Word *)data)->length, i = 0;
   for (hash = 0; i < length; str++){
      hash = *str + 31 * hash;
      i++;
   }

   return hash;
}

int compareString(const void *a, const void *b){
   const char *str1 = (const char *)(((Word *)a)->bytes);
   const char *str2 = (const char *)(((Word *)b)->bytes);
   int length1 = ((Word *)a)->length, i, comp = 0;
   int length2 = ((Word *)b)->length;
   if(length1 != length2)
      return 1;
   for(i = 0; i < length1; i++){
      if(str1[i] != str2[i])
         comp = 1;
   }
   return comp;
}

static void deleteWord(const void *data){
   free(((Word *)data)->bytes);
}

void* initializeHT(){
   unsigned sizes[] = {
      11, 23, 47, 89, 191, 409, 857, 
      1699, 3407, 6871, 13229, 27409, 58913, 120763, 248071, 
      500009, 1023199, 2014217, 4016167
   };
   int numSizes = 19;
   HTFunctions funcs = {hashString, compareString, deleteWord};
   float load = .7;
   return htCreate(&funcs, sizes, numSizes, load);
}

int main(int argc, char *argv[]){
   int n = 10; 
   Byte *word = NULL;
   int hasPrintable = 0, i, filePresent = 0;
   unsigned wordLength = 0; 
   void *ht;
   FILE *fp;
   
   ht = initializeHT();  
   for(i = 1; i < argc; i++){
      fp = parseArgs(argv[i], &n, &filePresent); 
      readFile(ht, fp, &word, &wordLength, &hasPrintable);
   } 

   readStdin(ht, filePresent, &word, &wordLength, &hasPrintable);
   printSpecs(ht, n); 
   
   htDestroy(ht); 
   return 0;
}
