#ifndef SHAREDMEMORYCONTROLLER_H
#define SHAREDMEMORYCONTROLLER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <sys/types.h>

#define SNAME "/mysem21"

typedef struct LLNode {
    int id;
    int val;
    int next;
} LLNode;

typedef struct stats_t{
  int count_ops;
  int inserts;
  int lookups_true;
  int lookups_false;
  int removes;
} stats_t;

typedef struct free_id_t{
  int id;
  int previous;
} free_id_t;

typedef struct sh_mem_t{
  int n_nodes;
  int node_mem_shmid;
  int mem_ptr;
  int free_list_shmid;
  int free_list_size;
  int next_free_id;
  int stats_shmid;
} sh_mem_t;

typedef struct sh_mem_add_t{
  sh_mem_t* ctrl_add;
  free_id_t* free_list_add;
  LLNode* node_mem_add;
  stats_t* stats_add;
} sh_mem_add_t;


extern int mem_ctrl_id;
extern sh_mem_add_t sh_mem_adds;

sh_mem_t* createShMem(int n_nodes);

LLNode* shAlloc(int* id);

void shFree(int id);

LLNode* getNode(int id);

void getMemAdds(int ct);

void printNode(LLNode* node);

void printLista();
#endif
