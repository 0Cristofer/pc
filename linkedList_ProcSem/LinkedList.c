/* Implementação do benchmark LinkedList da RSTM em C */
/* Autor: Bruno Cesar, @bcesarg6, bcesar.g6@gmail.com */
/* Abril de 2018                                      */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include "SharedMemoryController.h"

#define TRUE 1
#define FALSE 0

sem_t sem;
int mem_ctrl_id;
sh_mem_add_t sh_mem_adds;

/* Estruturas */
typedef struct pthread_arg{
  int in;
  int out;
}pthread_arg;

LLNode* sentinela;
static int lookups_true = 0;
static int lookups_false = 0;
static int inserts = 0;
static int removes = 0;

/* Dados globais com valores padrão */
static int datasetsize = 256;                  // number of items
static double duration = 5.0f;                 // in seconds
static int doWarmup = FALSE;
static int num_ops = 0;                        // number of operations mode value.
static int count_ops = 0;
static int n_threads = 2;

// these three are for getting various lookup/insert/remove ratios
static float lookupPct = 0.30f;
static float insertPct = 0.81f;

// Controla o tempo de execução
struct timespec tstart, tend;
double timeDiff;

int gtid = 0;

/* Exibe ajuda e finaliza o programa */
void help(int msg){
  switch (msg) {
    case 1:
      printf("\nNúmero insuficiente de parametros!\n");
      break;

    case 2:
      printf("\nParametros de entrada inválidos!\n");
      break;

    default:
      break;
  }

  printf("\n\tn : Número de threads [2]");
	printf("\n\ts : Tamanho do datasetsize [256]");
  printf("\n\tt : Tempo de duração da execução (segundos) [5.0]");
  printf("\n\tw : Ativa o Warm Up antes da execução [FALSE]");
  printf("\n\tx : Muda para o modo de execução por número de operações.");
  printf("\n\th : Mostra essa mensagem\n\n");
	exit(1);
}

/* Pega argumentos com getopt */
void getArgs(int argc, char *argv[]){
	extern char *optarg;
	char op;

	struct option longopts[] = {
    {"n_threads", 1, NULL, 'n'},
    {"size", 1, NULL, 's'},
    {"time", 1, NULL, 't'},
    {"warmup", 0, NULL, 'w'},
    {"x", 1, NULL, 'x'}
	};

	while ((op = getopt_long(argc, argv, "n:s:t:wx:h", longopts, NULL)) != -1) {
		switch (op) {
      case 'n':
        n_threads = atoi(optarg);
        break;

			case 's':
				datasetsize = atoi(optarg);
				break;

      case 't':
        duration = atof(optarg);
        break;

      case 'w':
        doWarmup = TRUE;
        break;

      case 'x':
        num_ops = atoi(optarg);
        break;

      case 'h':
        help(0);
        break;

			default:
        help(2);
        break;
      }
    }
}

/* Sanity Check */
int isSane(){
    int sane = TRUE;
    sem_wait(&sem);
    LLNode* prev = sentinela;
    LLNode* curr = prev->next;

    while (curr != NULL) {
        if ((prev->val) >= (curr->val)) {
            printf("FAILED SANITY CHECK IN: %d < %d\n", prev->val, curr->val);
            sane = FALSE;
            break;
        }
        prev = curr;
        curr = (curr->next);
    }
    sem_post(&sem);
    return sane;
}


// insert method; find the right place in the list, add val so that it is in
// sorted order; if val is already in the list, exit without inserting
void insert(int val){
  int id;
  sem_wait(&sem);
  // traverse the list to find the insertion point
  LLNode* prev = sentinela;
  LLNode* curr = sentinela->next;

  while (curr != NULL){
    if (curr->val >= val)
      break;

    prev = curr;
    curr = prev->next;
  }

  // now insert new_node between prev and curr
  if (!curr || (curr->val > val)){
    // ESCRITA : REGIÃO CRITICA

    LLNode* insert_point = prev;
    LLNode* novo = shAlloc(&id);
    novo->val = val;
    novo->next = curr;
    novo->id = id;

    insert_point->next = novo;
    // FIM
    }
    sem_post(&sem);
}

// search function
void lookup(void* arg){
  sem_wait(&sem);
  pthread_arg* p = (pthread_arg*) arg;
  int val = p->in;

  int found = FALSE;

  LLNode* curr = sentinela;
  curr = curr->next;

  while (curr != NULL) {
    if (curr->val >= val)
      break;

    curr = curr->next;
  }

  found = ((curr != NULL) && (curr->val == val));

  p->out = found;
  sem_post(&sem);
}

// remove a node if its value == val
void removeNode(int val){
  sem_wait(&sem);
  // find the node whose val matches the request
  LLNode* prev = sentinela;
  LLNode* curr = prev->next;

  while (curr != NULL) {
    // if we find the node, disconnect it and end the search
    if (curr->val == val) {
      // ESCRITA : REGIÃO CRITICA

      LLNode* mod_point = prev;
      mod_point->next = curr->next;

      // delete curr...
      shFree(curr->id);
      // FIM
      break;
    }
    else if (curr->val > val) {
      // this means the search failed
      break;
    }
    prev = curr;
    curr = prev->next;
  }
  sem_post(&sem);
}

// print the list
void printLista(){
    LLNode* curr = sentinela;
    curr = (curr->next);

    printf("lista :");
    while (curr != NULL){
        printf(" %d ->", curr->val);
        curr = (curr->next);
    }

    printf(" NULL\n\n");
}

void* experiment(void* arg){
  /* Garante thread id unico para a threads */
  sem_wait(&sem);
  int tid = gtid++;
  sem_post(&sem);

  //printf("tid = %d\n", tid);
  int result, val, i;
  float action;
  int l_ops, l_lookups_true, l_lookups_false, l_inserts, l_removes;
  l_ops = l_lookups_true = l_lookups_false = l_inserts = l_removes = 0;

  pthread_arg* p;
  p = malloc(sizeof(pthread_arg));

  srand(time(NULL) + tid);

  //printf("\nAction = %f | val = %d\n", action, val);

  if(num_ops != 0){
    for(i = 0; i < num_ops / n_threads; i++){
      action = (rand()%100) / 100.0;
      val = rand() % 1000;

      if (action < lookupPct) {
        printf("%d -> lookup\n", tid);
        p->in = val;
        lookup(p);
        result = p->out;

        if (result)
          l_lookups_true++;
        else
          l_lookups_false++;
      }
      else if (action < insertPct) {
        printf("%d -> insert %d\n", tid, val);
        insert(val);
        if(val > 50)
            removeNode(val);
        l_inserts++;
      }
      else {
        printf("%d -> remove %d\n", tid, val);
        removeNode(val);
        l_removes++;
      }

      //int sane = isSane();
      l_ops++;
    }
    // Time duration mode
  } else {
    //printf("%d Entrou time duration mode\n",tid);
    while(timeDiff < duration){
      action = (rand()%100) / 100.0;
      val = rand() % 1000;
      if (action < lookupPct) {
        p->in = val;
        lookup(p);
        result = p->out;

        if (result)
          l_lookups_true++;
        else
          l_lookups_false++;
      }
      else if (action < insertPct) {
        insert(val);
        l_inserts++;
      }
      else {
        removeNode(val);
        l_removes++;
      }

      //int sane = isSane();
      l_ops++;

      if(tid == 0){
        clock_gettime(CLOCK_MONOTONIC, &tend);
        timeDiff = ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
      }
    }
  }

  sem_wait(&sem);
  count_ops += l_ops;
  inserts += l_inserts;
  lookups_true += l_lookups_true;
  lookups_false += l_lookups_false;
  removes += l_removes;
  sem_post(&sem);
}

/* Checa se os parametros são validos, aborta caso não sejam */
void checkData(){
  if(n_threads < 1){
    printf("Número inválido de threads. Abortando...\n");
    exit(1);
  }

	if(datasetsize < 1){
		printf("Tamanho da lista inválida. Abortando...\n");
		exit(1);
	}

  if(duration <=0){
    printf("Tempo de execução inválido. Abortando...\n");
    exit(1);
  }

  if(num_ops < 0){
    printf("Modo Número de operações: Valor inválido. Abortando...\n");
    exit(1);
  }
}

void printInfo(){
  printf("\nNúmero de threads = %d", n_threads);
  if(num_ops > 0){
    printf("\nNúmero de operações = %d", num_ops);
  } else {
    printf("\nDuração = %2.lf segundos", duration);
  }
  printf("\nTamanho máximo da fila = %d nodes", datasetsize);
  printf("\nPorcentagens das operações: %.2f Lookup / %.2f Insert / %.2f Remove",
            lookupPct, insertPct - lookupPct, 1.0f - insertPct);

  if(doWarmup)
    printf("\nWarm Up: ativado");
  else
    printf("\nWarm Up: desativado");
}

int main(int argc, char *argv[]) {
  int i;
  printf("\nLinked List - versão sem\n");

	getArgs(argc, argv);
	checkData();
  printInfo();

  /* Inicializa a lista criando a sentinela */
  sentinela = malloc(sizeof(LLNode));
  sentinela->val = -1;
  sentinela->next = NULL;

  sem_init(&sem, 0, 1);

  pthread_t threads[n_threads];
  void* pth_status;

  /* Warm Up */
  // warmup inserts half of the elements in the datasetsize
  if(doWarmup){
      for (i = 0; i < datasetsize; i+=2) {
        insert(i);
      }
  }

  clock_gettime(CLOCK_MONOTONIC, &tstart);
  timeDiff = 0;

  /* SHM */
  printf("\n\n\t--- Rodando experimentos ---\n");

  createShMem(1000); //1000 é o número de nós máximo

  //FORK

  //A partir daqui todos o processos executam isso
  getMemAdds();
  sentinela = sh_mem_adds.node_mem_add;

  sh_mem_adds.ctrl_add = NULL;
  sh_mem_adds.free_list_add = NULL;
  sh_mem_adds.node_mem_add = NULL;

  experiment(NULL);

  //FORK "join"


  //TODO estatísticas ainda não estão na memória compartilhada, apenas os nós e
  //dados referentes a SH. Após a implementação do fork/join o funcionamento básico
  //*irá funcionar*, faltando os dados estatísticos
  /* SHM_END */

  /* TRHEAD (Remover quando inserir o fork)*/
  printf("\n\n\t--- Rodando experimentos ---\n");
  for(i = 0; i < n_threads; i++){
		pthread_create(&threads[i], NULL, experiment, NULL);
	}

  //experiment(NULL);

  for(i = 0; i < n_threads; i++){
		 pthread_join(threads[i], &pth_status);
  }
  /* THREAD_END */

  clock_gettime(CLOCK_MONOTONIC, &tend);
  timeDiff = ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);

  printf("\t    FIM DA EXECUÇÃO.\n");

  printLista();
  printf("\nSanity Check: ");
  if(isSane())
    printf("Passed\n");
  else
    printf("Failed! Isn't sane!\n");

  printf("Tempo de execução dos experimentos = %lf segundos\n", timeDiff);
  printf("Total de operações realizadas = %d\n",count_ops);
  printf("Total de lookups acertados: %d\n", lookups_true);
  printf("Total de lookups falhados: %d\n", lookups_false);
  printf("Total de Inserts: %d\n", inserts);
  printf("Total de removes: %d\n", removes);

  return 0;
}
