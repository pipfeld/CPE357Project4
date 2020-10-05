#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "getWord.h"
#include "ui.h"

void exponentialGrowth(Byte **mem, int limit, int c, unsigned *size){
   if(limit == 1){
      *mem = malloc(sizeof(Byte) * (*size));
      checkPtr(*mem, NULL);  
   }
    
   else if(*size <= limit){
      *size *= 2;  
      *mem = realloc(*mem, sizeof(Byte) * (*size));
      checkPtr(*mem, NULL);
   }

   (*mem)[limit - 1] = tolower(c);        
}

int getWord(FILE *file, Byte **word, unsigned *wordLength, int *hasPrintable){
   int c;
   unsigned size = 10;
   *hasPrintable = 0;
   *wordLength = 0;
   *word = NULL;
   
   while(isspace(c = fgetc(file))){
   }

   while(EOF != c){
      if(isspace(c))
         break;

      if(isprint(c))
         *hasPrintable = 1;

      (*wordLength)++;
      exponentialGrowth(word, *wordLength, c, &size); 
      c = fgetc(file);
   }

   if(*wordLength > 0 && *wordLength < size){ 
      *word = realloc(*word, sizeof(Byte) * (*wordLength));
      checkPtr(*word, NULL);
   }
   
   if(c == EOF)  
      return EOF;
      
   return 0;

}

