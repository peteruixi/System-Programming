#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define MAXLEN 80
#define WORD 64

typedef struct
{
  int status;
  int tag;
  int count;
}line;

int s, E, b;
int verbose;
line** cache;

void help()
{
  printf("Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n");
  printf("Option:\n");
  printf("-h: Optional help flag that prints usage info\n"); 
  printf("-v: Optional verbose flag that displays trace info\n"); 
  printf("-s <s>: Number of set index bits (S = 2s is the number of sets)\n"); 
  printf("-E <E>: Associativity (number of lines per set)\n"); 
  printf("-b <b>: Number of block bits (B = 2b is the block size)\n"); 
  printf("-t <tracefile>: Name of the valgrind trace to replay\n"); 
}

void init()
{
  int S =1<<s;
  cache = malloc (S * sizeof(line*));
  for (int r=0; r< S; r++){
    cache[r] = malloc (S*sizeof(line));
    for (int c=0; c<E; c++){
      cache[r][c].status = 0;
    }
  }
}

void UpdateCount (int set, int j){ 
  for (int i = 0 ; i<E; i++){
    if((cache[set][i].status == 1)&& 
       (cache[set][i].count>cache[set][j].count)){
      cache[set][i].count--;
    }
  }
  cache[set][j].count=E;
}

void eval(int argc, char* argv[], char* tracefile)
{
  char opt;
  while ((opt = getopt(argc,argv,"hvs:E:b:t:")) != EOF) { 
    switch (opt){
    case 'h':
      help();
      break;
    case 'v':
      verbose=1;
      break;
    case 's':
      s = atoi(optarg);
      break;
    case 'E':
      E = atoi(optarg);
      break;
    case 'b':
      b = atoi(optarg);
      break;
    case 't':
      strcpy(tracefile,optarg);
      break;
    default:
      break;
     
    }
  }
}

int hit, miss, evic;


void run(char* data){
   char instruct;
   int i,set;
   unsigned long address,tag;

   sscanf(data, " %c %lx",&instruct, &address);
   set = (address << (WORD-s-b)) >> (WORD -s);
   tag = address >> (s + b);
  
  for (i = 0; i < E; i++) {
    if ((cache[set][i].status) &&
	(cache[set][i].tag == tag)) {
      
      int temp  = (instruct == 'M') ? 2 : 1;
      hit+=temp;
      UpdateCount(set, i);
      return;
    }
  } 

  miss++;
  for (i = 0; i < E; i++) {
    if (!cache[set][i].status) {
      cache[set][i].status = 1;
      cache[set][i].tag = tag;
            
      if (instruct == 'M'){
	hit++;
      }
      UpdateCount(set, i);
      return;
    }
  }
  evic++;

  for(i = 0;i<E; i++){
    if(cache[set][i].count == 1){
      cache[set][i].tag = tag;
      if (instruct == 'M'){
	hit++;
      }
      UpdateCount(set,i);
      return;
    }
  }
}


int main(int argc, char* argv[])
{
   char tracefile[MAXLEN];
   char fileline[MAXLEN];
   FILE *file;
   
   eval(argc,argv,tracefile);
   if (!( s && E && b)){
     exit(1);
   }

   file = fopen(tracefile,"r");
   if(!file){
     printf("%s:File cannot be found. \n", tracefile);
     exit(1);
   }
   
   init();
     while (fgets(fileline, MAXLEN, file)) {
       if (fileline[0] ==' '){
	 fileline[strlen(fileline)-1]= '\0';
	 run(fileline);
       }
     }
   fclose(file);
   printSummary(hit,miss,evic);
   return 0;
}
