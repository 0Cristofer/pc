/* Implementação do benchmark LinkedList da RSTM em C */
/* Autor: Bruno Cesar, @bcesarg6, bcesar.g6@gmail.com */
/* Abril de 2018                                      */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#define TRUE 1
#define FALSE 0

/* Estruturas */
typedef struct LLNode {
    int val;
    struct LLNode *next;
} LLNode;

LLNode* sentinela;

/* Dados globais com valores padrão */
static int datasetsize = 256;                  // number of items
static int duration = 1;                       // in seconds
static int doWarmup = FALSE;

// these three are for getting various lookup/insert/remove ratios
static float lookupPct = 0.34f;
static float insertPct = 0.67f;

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
  printf("\n\tt : Tempo de duração da execução (segundos) [1]");
  printf("\n\tw : Ativa o Warm Up antes da execução [FALSE]");
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
    {"warmup", 0, NULL, 'w'}
	};

	while ((op = getopt_long(argc, argv, "s:t:wh", longopts, NULL)) != -1) {
		switch (op) {
			case 's':
				datasetsize = atoi(optarg);
				break;

      case 't':
        duration = atoi(optarg);
        break;

      case 'w':
        doWarmup = TRUE;
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
  printf("\nDuração = %d segundos", duration);
  printf("\nTamanho máximo da fila = %d nodes", datasetsize);
  printf("\nPorcentagens das operações: %.2f Lookup / %.2f Insert / %.2f Remove",
            lookupPct, insertPct - lookupPct, 1.0f - insertPct);

  if(doWarmup)
    printf("\nWarm Up: ativado");
  else
    printf("\nWarm Up: desativado");

  printf("\nIniciando o experimento...\n\n");
}

/*
void* experiment(void* arg){
    int tid = (int*) arg;

    float action = rand

    if (action < BMCONFIG.lookupPct) {
        TX_FUNC(result, SET->template lookup, val);
        if (result)
            ++args->count[TXN_LOOKUP_TRUE];
        else
            ++args->count[TXN_LOOKUP_FALSE];
    }
    else if (action < BMCONFIG.insertPct) {
        TX_CALL(SET->template insert, val);
        ++args->count[TXN_INSERT];
    }
    else {
        TX_CALL(SET->template remove, val);
        ++args->count[TXN_REMOVE];
    }

    bool sanity_check() const { return SET->isSane(); }
}
*/

int main(int argc, char const *argv[]) {
  int i;
  printf("\nLinked List - versão sequencial\n");

	getArgs(argc, argv);
	checkData();
  printInfo();

  /* Inicializa a lista criando a sentinela */
  sentinela = malloc(sizeof(LLNode));
  sentinela->next = NULL;

  /* Warm Up */
  // warmup inserts half of the elements in the datasetsize
  if(doWarmup){
      for (i = 0; i < datasetsize; i+=2) {
        insert(i);
      }
  }

  /* TODO implementar mecanismo que aleatoriza as operações? */
  /* for 1 -> datasetsize : pthread_create? */
  /* rand(x) -> switch(x): case  1: insert | case 2: remove? */
  /* barreiras ? */

  printLista();
  removeNode(3);
  printLista();

  return 0;
}
