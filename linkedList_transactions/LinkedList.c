/* Implementação do benchmark LinkedList da RSTM em C */
/* Autor: Bruno Cesar, @bcesarg6, bcesar.g6@gmail.com */
/* Abril de 2018                                      */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>

#define TRUE 1
#define FALSE 0

/* Estruturas */
typedef struct pthread_arg{
  int in;
  int out;
}pthread_arg;

typedef struct LLNode {
    int val;
    struct LLNode *next;
} LLNode;

LLNode* sentinela;
LLNode* novo;

pthread_barrier_t barrier;

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
static int lookupPct = 34;
static int insertPct = 67;

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

// print the list
void printLista(){
    const LLNode* curr = sentinela;
    curr = (curr->next);

    printf("lista :");
    while (curr != NULL){
        printf(" %d ->", curr->val);
        curr = (curr->next);
    }

    printf(" NULL\n\n");
}

/* Sanity Check */
int isSane(){
    int sane = TRUE;
    const LLNode* prev = sentinela;
    const LLNode* curr = prev->next;

    while (curr != NULL) {
        if ((prev->val) >= (curr->val)) {
            printf("FAILED SANITY CHECK IN: %d < %d\n", prev->val, curr->val );
            sane = FALSE;
            break;
        }
        prev = curr;
        curr = (curr->next);
    }
    return sane;
}


// insert method; find the right place in the list, add val so that it is in
// sorted order; if val is already in the list, exit without inserting
void insert(int val){
    // traverse the list to find the insertion point
    __transaction_atomic{
    const LLNode* prev = sentinela;
    const LLNode* curr = sentinela->next;

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
      novo = malloc(sizeof(LLNode));
      novo->val = val;
      novo->next = NULL;

      insert_point->next = novo;
      // FIM
    }
  }
}

// search function
void lookup(void* arg){
  printLista();
  //__transaction_atomic{
    pthread_arg* p = (pthread_arg*) arg;
    int val = p->in;

    int found = FALSE;

    const LLNode* curr = sentinela;
    curr = curr->next;

    while (curr != NULL) {
      if (curr->val >= val)
      break;

      curr = curr->next;
    }

    found = ((curr != NULL) && (curr->val == val));

    p->out = found;
  //}
}

// findmax function
int findmax(){
    int max = -1;
    const LLNode* curr = sentinela;

    while (curr != NULL) {
        max = curr->val;
        curr = curr->next;
    }

    return max;
}

// findmin function
int findmin(){
    int min = -1;

    const LLNode* curr = sentinela;
    curr = curr->next;

    if (curr != NULL)
        min = curr->val;

    return min;
}

// remove a node if its value == val
void removeNode(int val){
    // find the node whose val matches the request
    __transaction_atomic{
    const LLNode* prev = sentinela;
    const LLNode* curr = prev->next;

    while (curr != NULL) {
      // if we find the node, disconnect it and end the search
      if (curr->val == val) {
        // ESCRITA : REGIÃO CRITICA

        LLNode* mod_point = prev;
        mod_point->next = curr->next;


        // delete curr...
        free(curr);
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
  }
}

void* experiment(void* arg){
  /* Garante thread id unico para a threads */
  int tid;
  __transaction_atomic{
    tid = gtid++;
  }

  int result, val, i;
  int action;
  int l_ops, l_lookups_true, l_lookups_false, l_inserts, l_removes;
  l_ops = l_lookups_true = l_lookups_false = l_inserts = l_removes = 0;

  pthread_arg* p;
  p = malloc(sizeof(pthread_arg));

  srand(time(NULL) + tid);

  //printf("\nAction = %f | val = %d\n", action, val);

  if(num_ops != 0){
    for(i = 0; i < num_ops / n_threads; i++){
      action = (rand()%100);
      val = rand()%100;

      if (action < lookupPct) {
        printf("%d -> lookup\n", tid);
        p->in = val;
        lookup(p);
        result = p->out;

        if (result)
          l_lookups_true++;
        else
          l_lookups_false++;

      } else if (action < insertPct) {
        printf("%d -> insert %d\n", tid, val);
        insert(val);
        l_inserts++;
      } else {
        printf("%d -> remove %d\n", tid, val);
        removeNode(&val);
        l_removes++;
      }

      int sane = isSane();
      l_ops++;
      printf("%d esperando barreira\n",tid);
      pthread_barrier_wait(&barrier);
      printf("\n ---- TERMINOU UM LAÇO FOR ---\n");
    }
    // Time duration mode
  } else {
    printf("%d Entrou time duration mode\n",tid);
    while(timeDiff < duration){
      action = (rand()%100);
      val = rand()%100;
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
        removeNode(&val);
        l_removes++;
      }

      int sane = isSane();
      l_ops++;

      if(tid == 0){
        clock_gettime(CLOCK_MONOTONIC, &tend);
        timeDiff = ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
      }
    }
  }

  /* Soma os dados locais aos dados globais */
  __transaction_atomic{
    count_ops += l_ops;
    inserts += l_inserts;
    lookups_true += l_lookups_true;
    lookups_false += l_lookups_false;
    removes += l_removes;
  }
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
  printf("\nPorcentagens das operações: %d%% Lookup / %d%% Insert / %d%% Remove",
            lookupPct, insertPct - lookupPct, 100 - insertPct);

  if(doWarmup)
    printf("\nWarm Up: ativado");
  else
    printf("\nWarm Up: desativado");
}

int main(int argc, char const *argv[]) {
  int i;
  printf("\nLinked List - versão transactions\n");

	getArgs(argc, argv);
	checkData();
  printInfo();

  /* Inicializa a lista criando a sentinela */
  sentinela = malloc(sizeof(LLNode));
  sentinela->val = -1;
  sentinela->next = NULL;

  pthread_t threads[n_threads -1];
  void* pth_status;

  pthread_barrier_init(&barrier, NULL ,n_threads);

  /* Warm Up */
  // warmup inserts half of the elements in the datasetsize
  if(doWarmup){
      for (i = 0; i < datasetsize; i+=2) {
        insert(i);
      }

      printLista();
  }


  clock_gettime(CLOCK_MONOTONIC, &tstart);
  timeDiff = 0;

  printf("\n\n\t--- Rodando experimentos ---\n");
  for(i = 1; i < n_threads; i++){
		pthread_create(&threads[i], NULL, experiment, NULL);
	}

  experiment(NULL);

  for(i = 1; i < n_threads; i++){
		 pthread_join(threads[i], &pth_status);
	}

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
