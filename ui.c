#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "hashTable.h"
#include "getWord.h"
#include "ui.h"
#include "qsortHTEntries.h"

void readInput(void *ht, FILE* fp, Byte **word, 
   unsigned *wordLength, int *hasPrintable);
int isFlag(char word[], int *n);

void checkPtr(void *ptr, const char *fname){
   const char *err = "";
   if(ptr == NULL){
      fprintf(stderr, "wf: ");
      if(fname != NULL)
         err = fname;
      perror(err);
      exit(EXIT_FAILURE);
   }
}

void usageError(){
   fprintf(stderr, "Usage: wf [-nX] [file...]\n");
   exit(EXIT_FAILURE);
}

FILE* fileOpen(const char *fname){
   FILE *fp = fopen(fname, "rb");

   checkPtr(fp, fname);
 
   return fp;
}

FILE* parseArgs(char argument[], int *n, int *filePresent){
   FILE* fp = NULL;
   if(!isFlag(argument, n)){
      fp = fileOpen(argument);
      checkPtr(fp, argument);
      *filePresent +=1;
   }
   return fp;
}

int isFlag(char word[], int *n){
   int i;
   if(word[0] == '-' && word[1] == 'n'){
      i = sscanf(word, "-n%d", n);
      if(i < 1 || *n < 1){
         *n = 10;
         usageError();
      }
      return 1;
   }
   return 0;
}

void printWord(Word *word){
   Byte *chars = word->bytes;
   int length = word->length, i;

   for(i = 0; i < length && i < 30; i++){
      if(isprint(chars[i]))
         printf("%c", chars[i]);
      else
         printf("%c", '.');
   }

   if(length > 30)
      printf("...");
   
   printf("\n");
}

void printSpecs(void *ht, int n){
   int i;
   unsigned size;
   HashTable *hashTable = ht;
   HTEntry *entries = htToArray(ht, &size);
   unsigned cap = (size > n) ? n : size;
   qsortHTEntries(entries, size);
   
   printf("%u unique words found in %u total words\n",
      hashTable->unique, hashTable->total);
   for(i = 0; i < cap && i < 30; i++){
      printf("%10u - ", entries[i].frequency);
      printWord((Word *)entries[i].data); 
   }
   free(entries);
}

void readFile(void *ht, FILE *fp, Byte **word, 
   unsigned *wordLength, int *hasPrintable){
   if(fp != NULL)
      readInput(ht, fp, word, wordLength, hasPrintable);
}

int addEntry(Word *w, unsigned wordLength, Byte *word, void *ht){
   int freq;
   checkPtr(w, NULL);
   w->bytes = word;
   w->length = wordLength;
   freq = htAdd(ht, w);
   return freq;
}

void checkWords(Byte **word, unsigned *wordLength, int *hasPrintable, void *ht){
   int freq = 0;
   Word *w;
   if(*hasPrintable){
      w = malloc(sizeof(Word));
      freq = addEntry(w, *wordLength, *word, ht);
   }
   if(freq > 1){
      free(*word);
      free(w);
   }
   else if(!(*hasPrintable) || freq == 0)
      free(*word);
}

void readInput(void *ht, FILE* fp, 
   Byte **word, unsigned *wordLength, int *hasPrintable){
   Word *w; 
   while(EOF != getWord(fp, word, wordLength, hasPrintable)){
      checkWords(word, wordLength, hasPrintable, ht);
   }
   if(*wordLength > 0 && *hasPrintable){
      w = malloc(sizeof(Word));
      addEntry(w, *wordLength, *word, ht);
   }
   fclose(fp);
}

void readStdin(void *ht, int filePresent, 
   Byte **word, unsigned *wordLength, int *hasPrintable){
   if(!filePresent)
      readInput(ht, stdin, word, wordLength, hasPrintable);
}
