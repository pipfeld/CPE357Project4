#include <limits.h>
#include <string.h>
#include <assert.h>
#include "hashTable.h"
#include "ui.h"

void rehasher(HashTable *ht);
void destroyNode(HashTable *ht, int i);
unsigned getChainCount(HashTable *ht, int i, unsigned chainCount);
unsigned countChain(HashTable *ht, int i);
void chainRehash(HashTable *ht, HashNode *hn, HashNode** newArray);
HashNode* addRehashedNode(HashNode *target, HashNode *hn);
HashNode* searchForData(HashTable *ht, HashNode *hn, int *found, void *data);
HashNode* addNewNode(HashTable *ht, HashNode *hn, void *data, int hashVal);
void arrayHelper(HTEntry *entries, HashNode *hn, unsigned *size);
/* Description: Creates a new hash table as specified.
 *
 * Notes:
 *   1. The function asserts (man 3 assert) if numSizes is not 1 or more.
 *   2. The function asserts (man 3 assert) if any of the sizes are not
 *      greater than the immediately preceding size.
 *   3. The function asserts (man 3 assert) If the load factor is not greater
 *      than 0.0 and less than or equal to 1.0.
 *   4. You must make a deep copy of the _functions_ pointers and _sizes_
 *      array since you don't know where the caller allocated them - they
 *      could be on the stack and you don't want to point there - that's a
 *      potential and likely _unlucky bug_ waiting to happen!
 *
 * Parameters:
 *    functions: A structure of function pointers to the data-specific
 *       functions required by the hash table.
 *    sizes: An array of hash table sizes. Must always be at least one size
 *       greater than zero specified, more if rehashing is desired. Prime
 *       numbers are best but not required/checked for. That being said, you
 *       should use prime numbers!
 *    numSizes: The number of values in the sizes array.
 *    rehashLoadFactor: The load factor to rehash at. Should be a value greater
 *       than 0.0 and less than or equal to 1.0. A value of 1.0 means
 *       "do not rehash".
 *
 * Return: A pointer to an anonymous (file-local) structure representing a
 *         hash table.
 */
void* htCreate(
   HTFunctions *functions,
   unsigned sizes[],
   int numSizes, 
   float rehashLoadFactor)
{
   int i;
 
   HashTable *ht = malloc(sizeof(HashTable));
   checkPtr(ht, NULL);

   assert(sizes[0] > 0);   
   assert(numSizes > 0);
   for(i = 0; i < numSizes - 1; i++)
      assert(sizes[i] < sizes[i + 1]);
   assert(rehashLoadFactor > 0.0 && rehashLoadFactor <= 1.0);
   
   ht->funcs = malloc(sizeof(HTFunctions));
   checkPtr(ht->funcs, NULL);
   *(ht->funcs) = *functions;

   ht->sizes = malloc(sizeof(unsigned) * numSizes);
   checkPtr(ht->sizes, NULL);
   
   memcpy(ht->sizes, sizes, sizeof(unsigned) * numSizes);
   ht->numSizes = numSizes;
   ht->rehashLoadFactor = rehashLoadFactor;
   ht->rehash = 0;
   ht->total = 0;
   ht->unique = 0;
   ht->theArray = calloc(sizes[0], sizeof(HashNode*));
   checkPtr(ht->theArray, NULL);

   return ht; 
}

/* Description: Frees all of the dynamically allocated memory allocated by the
 *    hash table itself AS WELL AS all of the data object added to the hash
 *    table by the user via htAdd.
 *
 *    IMPORTANT: When an FNDestroy function was provided to htCreate, i.e.,
 *               FNDestroy is not NULL, this function (htDestroy) MUST call the
 *               provided FNDestroy function and pass each data object added to
 *               the hash table by the user via htAdd PRIOR to freeing that
 *               object. 
 *
 *               The htDestroy function, and ONLY the htDestroy function, calls
 *               free on the data added to the hash table via htAdd AND ALWAYS
 *               calls it!
 *
 *               Be sure to test htDestroy with and without an FNDestroy
 *               function and be sure to include data with a sub-allocation
 *               (maybe a structure with a pointer to some dynamically
 *               allocated memory?) to be sure both situations work correctly!
 *
 * Parameters:
 *    hashTable: A pointer returned by htCreate.
 *
 * Return: None
 */

void htDestroy(void *hashTable)
{
   HashTable *ht = hashTable;
   int i; 
   unsigned cap = htCapacity(ht); 
   for(i = 0; i < cap; i++)
      destroyNode(ht, i);
   free(ht->theArray);
   free(ht->sizes);
   free(ht->funcs);
   free(ht);
}

void destroyNode(HashTable *ht, int i){
   HashNode *hn, *next;
   hn = ht->theArray[i];
   while(hn != NULL){
      next = hn->next;
      if(ht->funcs->destroy != NULL)
         ht->funcs->destroy(hn->entry->data);  
      free(hn->entry->data);
      free(hn->entry);
      free(hn);
      hn = next;
   } 
}
/* Description: Adds a shallow copy of the data to the hash table. The data
 *    being added MUST BE dynamically allocated because it will be freed
 *    when the hash table is destroyed!
 * 
 * Notes:
 *    1. The function is expected to have O(1) performance.
 *    2. The function asserts (man 3 assert) if data is NULL. 
 *    3. The function rehashes to the next size when:
 *          1. The rehash load factor is NOT 1.0, a value of 1.0 indicates
 *             rehashing is not desired.
 *          2. And there is a next size. Otherwise continue with the current
 *             size.
 *          3. And the ratio of unique entries TO the current hash table size
 *             (BEFORE adding the new data) exceeds the rehash load factor.
 *    4. When the data being added is a duplicate, the original entry is kept
 *       in the hash table AND the caller is responsible for freeing the
 *       duplicate.
 *
 * Parameters:
 *    hashTable: A pointer returned by htCreate.
 *    data: The data to add.
 *
 * Return: The frequency of the data in the hash table. A value of 1 means it
 *    is a new and unique entry, values greater than 1 mean it is a duplicate
 *    with the indicated frequency.
 */

void rehasher(HashTable *ht){
   int i; 
   unsigned oldCap = htCapacity(ht);
   HashNode **newArray;
   HashNode *hn;
   (ht->rehash)++; 
   newArray = calloc(htCapacity(ht), sizeof(HashNode*));
   checkPtr(newArray, NULL);  
   for(i = 0; i < oldCap; i++){
      hn = ht->theArray[i];
      chainRehash(ht, hn, newArray);
   }
   free(ht->theArray);
   ht->theArray = newArray;
}

void chainRehash(HashTable *ht, HashNode *hn, HashNode** newArray){
   int hashVal;
   HashNode* next;
   unsigned cap = htCapacity(ht);
   while(hn != NULL){
      next = hn->next;
      hashVal = ht->funcs->hash(hn->entry->data) % cap;
      newArray[hashVal] = addRehashedNode(newArray[hashVal], hn);
      hn = next;
   }
}

HashNode* addRehashedNode(HashNode *target, HashNode *hn){
   HashNode *curr = target, *prev = NULL;
   hn->next = NULL;
   while(curr != NULL){
      prev = curr;
      curr = curr->next;
   }
   if(prev != NULL){
      prev->next = hn;
      return target;
   }
   
   return hn;
}
      
HashNode* searchForData(HashTable *ht, HashNode *hn, int *found, void *data){
   HashNode *curr = hn;
   while(curr != NULL){
      if(ht->funcs->compare(curr->entry->data, data) == 0){
         ++(curr->entry->frequency);
         *found = 1;
         return curr;
      }
      curr = curr->next;
   }
   return hn;
}

HashNode* addNewNode(HashTable *ht, HashNode *hn, void *data, int hashVal){
   HTEntry *e = malloc(sizeof(HTEntry));
   HashNode *hash = malloc(sizeof(HashNode)); 
   checkPtr(e, NULL);
   checkPtr(hash, NULL);
   (ht->unique)++;
   e->data = data;
   e->frequency = 1;
   hash->entry = e;
   hash->next = hn; 
   ht->theArray[hashVal] = hash;
   return hash;
}

unsigned htAdd(void *hashTable, void *data)
{

   HashTable *ht = hashTable;
   HashNode *hn;   
   float loadFactor = (float)(ht->unique)/htCapacity(ht);
   int hashVal, found = 0;
      
   assert(data != NULL);
   
   if(loadFactor > ht->rehashLoadFactor && ht->rehashLoadFactor != 1.0 
      && (ht->rehash + 1) != ht->numSizes)
      rehasher(ht);
   hashVal = ht->funcs->hash(data) % htCapacity(ht);  
   (ht->total)++; 
   hn = ht->theArray[hashVal];
   
   if(hn != NULL)
      hn = searchForData(ht, hn, &found, data);
   
   if(!found)
      hn = addNewNode(ht, hn, data, hashVal);
   
   return hn->entry->frequency;
}

HTEntry htLookUp(void *hashTable, void *data)
{
   HTEntry e = {NULL, 0};
   HashTable *ht = hashTable;
   HashNode *hn;
   int hashVal;
   
   assert(data != NULL);
   
   hashVal = (ht->funcs->hash(data)) % (htCapacity(ht));
   hn = ht->theArray[hashVal];
   while(hn != NULL){
      if(ht->funcs->compare(hn->entry->data, data) == 0)
         e = *(hn->entry);
      hn = hn->next;
   }
         
   return e;
}

HTEntry* htToArray(void *hashTable, unsigned *size)
{
   HashTable *ht = hashTable;
   HTEntry *entries = malloc(sizeof(HTEntry) * ht->total);
   HashNode *hn;
   unsigned cap = htCapacity(ht);
   int i;
   checkPtr(entries, NULL);
   *size = 0;
   for(i = 0; i < cap; i++){
      hn = ht->theArray[i];
      arrayHelper(entries, hn, size);
   }
   if(*size == 0){
      free(entries);
      entries = NULL;
   }

   return entries;
}

void arrayHelper(HTEntry *entries, HashNode *hn, unsigned *size){
   while(hn != NULL){ 
      entries[*size] = *(hn->entry);
      (*size)++;
      hn = hn->next;
   }
}

unsigned htCapacity(void *hashTable)
{
   HashTable *ht = hashTable;
   return ht->sizes[ht->rehash];
}

unsigned htUniqueEntries(void *hashTable)
{
   return ((HashTable *)hashTable)->unique;
}

unsigned htTotalEntries(void *hashTable)
{
   return ((HashTable *)hashTable)->total; 
}

HTMetrics htMetrics(void *hashTable)
{
   HTMetrics metrics;
   HashTable *ht = hashTable;

   int i;
   unsigned total = 0, chainCount = 0, maxChain = 0, count = 0;
   
   for(i = 0; i < htCapacity(ht); i++){
      chainCount = getChainCount(ht, i, chainCount);
      count = countChain(ht, i);
      total = total + count;
      if(count > maxChain)
         maxChain = count;
   }

   metrics.numberOfChains = chainCount;
   
   metrics.maxChainLength = maxChain;
   if(total == 0)
      metrics.avgChainLength = 0;
   else
      metrics.avgChainLength = (float)total/chainCount;

   return metrics;
}

unsigned getChainCount(HashTable *ht, int i, unsigned chainCount){
   HashNode *hn = ht->theArray[i];
   if(hn != NULL)
      chainCount++;
   return chainCount;
}

unsigned countChain(HashTable *ht, int i){
   int count = 0;
   HashNode *hn = ht->theArray[i];
   while(hn != NULL){
      count++;
      hn = hn->next;
   }

   return count;
}
