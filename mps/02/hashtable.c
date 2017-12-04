

#include <stdlib.h>
#include <string.h>
#include "hashtable.h"

/* Daniel J. Bernstein's "times 33" string hash function, from comp.lang.C;

   See https://groups.google.com/forum/#!topic/comp.lang.c/lSKWXiuNOAk */
unsigned long hash(char *str) {
  unsigned long hash = 5381;
  int c;

  while ((c = *str++))
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}

hashtable_t *make_hashtable(unsigned long size) {
  hashtable_t *ht = malloc(sizeof(hashtable_t));
  ht->size = size;
  ht->buckets = calloc(sizeof(bucket_t *), size);
  return ht;
}

void ht_put(hashtable_t *ht, char *key, void *val)
 {
  /* FIXME: the current implementation doesn't update existing entries */
  unsigned int idx = hash(key) % ht->size;
  bucket_t *tem = ht->buckets[idx];
  while (tem)
 {
    if (strcmp(tem->key,key)==0)
      {
	free(tem->val);
	tem->val=val;
	free(key);
	return;
      }
       tem=tem->next;
     
  }
  
   bucket_t *b = malloc(sizeof(bucket_t));
   b->key = key;
   b->val = val;
   b->next = ht->buckets[idx];
   ht->buckets[idx] = b;
   return;
}

void *ht_get(hashtable_t *ht, char *key) {
  unsigned int idx = hash(key) % ht->size;
  bucket_t *b = ht->buckets[idx];
  while (b)
 {
    if (strcmp(b->key, key) == 0) {
      return b->val;
    }
    b = b->next;
  }
  return NULL;
}

void ht_iter(hashtable_t *ht, int (*f)(char *, void *)) {
  bucket_t *b;
  unsigned long i;
  for (i=0; i<ht->size; i++) {
    b = ht->buckets[i];
    while (b) {
      if (!f(b->key, b->val)) {
        return ; // abort iteration
      }
      b = b->next;
    }
  }
}

void free_hashtable(hashtable_t *ht)
{
  int i = 0;
  // unsigned int idx = i;
  // bucket_t *b = ht->buckets[idx];//new
  // bucket_t *c = ht->buckets[idx];//new
  while (i<ht->size)
  {
     unsigned int idx = i;
     bucket_t *b = ht->buckets[idx];
     bucket_t *c = ht->buckets[idx];
    while (b)
      {
        free(b->val);
	free(b->key);
	c=b->next;
	free(b);
	b=c;
      }
    i++;
    // free(ht->buckets);
    // free(ht);
  }
   free(ht->buckets);
   free(ht); // FIXME: must free all substructures!
   // return ;
}

/* TODO */
void  ht_del(hashtable_t *ht, char *key)
{
  unsigned int idx = hash(key) % ht->size;
  bucket_t *tem = ht->buckets[idx];
  bucket_t *deletoid = ht->buckets[idx];
  while (tem)
    {
     if(strcmp(tem->key,key)==0)
        {
	  ht->buckets[idx]=tem->next;
	  free(tem->val);
	  free(tem->key);
	  free(tem);
	  return;
	}
      deletoid = tem->next;
      if (strcmp(deletoid->key,key)==0)
	{
	  tem->next = deletoid->next;
	  free(deletoid->val);
	  free(deletoid->key);
	  free(deletoid);
	  return;
	}

      tem=deletoid;

    }

}



void  ht_rehash(hashtable_t *ht, unsigned long newsize)
{
  hashtable_t *new_ht = malloc(sizeof(hashtable_t));
  new_ht->size = newsize;
  new_ht->buckets = calloc(sizeof(bucket_t *), newsize);
  int i = 0;
  while (i<=ht->size-1)
    {
      unsigned int idx = i;
      bucket_t *b = ht->buckets[idx];
      bucket_t *c = ht->buckets[idx];
      while (b)
	{
	  c=b->next;
	  ht_put(new_ht,b->key,b->val);
	  free(b);
	  b=c;
	  //free(b->key);
	  // free(b->val);

	}
      free(c);
      i++;
    }
  free(ht->buckets); 
  *ht=*new_ht;
  free(new_ht);
  // free(ht);
  // return;
  
}
