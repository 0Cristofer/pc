/* Implementação do benchmark LinkedList da RSTM em C */
/* Autor: Bruno Cesar, @bcesarg6, bcesar.g6@gmail.com */
/* Abril de 2018                                      */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>

#define TRUE 1
#define FALSE 0

/* Estruturas */
typedef struct pthread_arg{
  int in;
  int out;
}pthread_arg;

pthread_arg* p;

typedef struct LLNode {
    int val;
    struct LLNode *next;
} LLNode;

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

// these three are for getting various lookup/insert/remove ratios
static float lookupPct = 0.34f;
static float insertPct = 0.67f;

// Controla o tempo de execução
struct timespec tstart, tend;
double timeDiff;

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
    {"size", 1, NULL, 's'},
    {"time", 1, NULL, 't'},
    {"warmup", 0, NULL, 'w'},
    {"x", 1, NULL, 'x'}
	};

	while ((op = getopt_long(argc, argv, "s:t:wx:h", longopts, NULL)) != -1) {
		switch (op) {
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

/* Checa se os parametros são validos, aborta caso não sejam */
void checkData(){
	if (datasetsize < 1){
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

  //duration = duration * 1000;
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
    LLNode* insert_point = prev;

    // ESCRITA : REGIÃO CRITICA
    LLNode* novo = malloc(sizeof(LLNode));
    novo->val = val;
    novo->next = NULL;

    insert_point->next = novo;
    // FIM
    }
}

// search function
void lookup(void* arg){
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
  const LLNode* prev = sentinela;
  const LLNode* curr = prev->next;

  while (curr != NULL) {
    // if we find the node, disconnect it and end the search
    if (curr->val == val) {
      LLNode* mod_point = prev;

      // ESCRITA : REGIÃO CRITICA
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

void printInfo(){
  printf("\nDuração = %d segundos", duration / 10000);
  printf("\nTamanho máximo da fila = %d nodes", datasetsize);
  printf("\nPorcentagens das operações: %.2f Lookup / %.2f Insert / %.2f Remove",
            lookupPct, insertPct - lookupPct, 1.0f - insertPct);

  if(doWarmup)
    printf("\nWarm Up: ativado");
  else
    printf("\nWarm Up: desativado");
}

void experiment(){
    //int tid = (int*) arg;
    int result;

    float action = (rand()%100) / 100.0;
    int val = rand()%1000;

    //printf("\nAction = %f | val = %d\n", action, val);

    if (action < lookupPct) {
      //printf("Lookup\n");

      p->in = val;
      lookup(p);
      result = p->out;

      if (result)
        lookups_true++;
      else
        lookups_false++;
    }
    else if (action < insertPct) {
      //printf("Insert\n");
      insert(val);
      inserts++;
    }
    else {
        //printf("Remove\n");
        removeNode(val);
        removes++;
    }

    int sane = isSane();
}

int main(int argc, char const *argv[]) {
  int i;
  printf("\nLinked List - versão sequencial\n");

	getArgs(argc, argv);
	checkData();
  printInfo();

  /* Inicializa a lista criando a sentinela */
  sentinela = malloc(sizeof(LLNode));
  sentinela->val = -1;
  sentinela->next = NULL;

  p = malloc(sizeof(pthread_arg));

  /* Warm Up */
  // warmup inserts half of the elements in the datasetsize
  if(doWarmup){
      for (i = 0; i < datasetsize; i+=2) {
        insert(i);
      }
  }

  srand (time(NULL));
  clock_gettime(CLOCK_MONOTONIC, &tstart);
  timeDiff = 0;

  printf("\n\n\t--- Rodando experimentos ---\n");
  // Num_ops mode
  if(num_ops != 0){
    for(i = 0; i < num_ops; i++){
      experiment();
      count_ops++;
    }
    clock_gettime(CLOCK_MONOTONIC, &tend);
    timeDiff = ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);

  // Time duration mode
  } else {
    while(timeDiff < duration){
      experiment();
      clock_gettime(CLOCK_MONOTONIC, &tend);
      timeDiff = ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
      count_ops++;
    }
  }

  printf("\t    FIM DA EXECUÇÃO.\n");

  //printLista();
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
