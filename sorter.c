#include "getWord.h"
#include "hashTable.h"

int freqCompare(const void *a, const void *b);
int byteCompare(const void *a, const void *b);

void qsortHTEntries(HTEntry *entries, int numberOfEntries){
   qsort((void *) entries, numberOfEntries, sizeof(HTEntry), freqCompare);
}

int freqCompare(const void *a, const void *b){
   int freq1 = ((HTEntry *)a)->frequency;
   int freq2 = ((HTEntry *)b)->frequency;
   if(freq1 > freq2)
      return -1;
   else if(freq2 > freq1)
      return 1;
   else
      return byteCompare(a, b);
   return 0;
}

int byteCompare(const void *a, const void *b){
   HTEntry *entry1 = (HTEntry *)a;
   HTEntry *entry2 = (HTEntry *)b;
   Word *word1 = (Word *)(entry1->data);
   Word *word2 = (Word *)(entry2->data);
   int i;
   int max = (word1->length > word2->length) ? word2->length : word1->length;
   for(i = 0; i < max; i++){
      if(word1->bytes[i] > word2->bytes[i])
         return 1;
      else if(word2->bytes[i] > word1->bytes[i])
         return -1;
   }
   if(word1->length > word2->length)
      return 1;
   else if(word2->length > word1->length)
      return -1;
   return 0;  
}

