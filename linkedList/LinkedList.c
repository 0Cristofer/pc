/* Implementação do benchmark LinkedList da RSTM em C */
/* Autor: Bruno Cesar, @bcesarg6, bcesar.g6@gmail.com */
/* Abril de 2018                                      */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

/* Estruturas */
typedef struct LLNode {
    int val;
    struct LLNode *next;
} LLNode;

/* Dados globais */
static int n;

LLNode* sentinela;

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

	printf("\nLinked List - versão sequencial\n\tn : Numero de operações a serem executadas\n\th : Mostra essa mensagem\n\n");
	exit(1);
}

/* Pega argumentos com getopt */
void getArgs(int argc, char *argv[]){
	extern char *optarg;
	char op;

	if(argc < 2){

		help(1);
	}

	struct option longopts[] = {
    {"numero", 1, NULL, 'n'}
	};

	while ((op = getopt_long(argc, argv, "n:h", longopts, NULL)) != -1) {
		switch (op) {
			case 'n':
				n = atoi(optarg);
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
	if (n < 1){
		printf("Numero de operações inválido. Abortando...\n");
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

    printf(" NULL\n");
}

int main(int argc, char const *argv[]) {
  int i;
  printf("Iniciando execução...\n");

	getArgs(argc, argv);
	checkData();

  /* Inicializa a lista criando a sentinela */
  sentinela = malloc(sizeof(LLNode));
  sentinela->next = NULL;

  /* TODO implementar mecanismo que aleatoriza as operações? */
  /* for 1 -> n : pthread_create? */
  /* rand(x) -> switch(x): case  1: insert | case 2: remove? */
  /* barreiras ? */

  /* Teste */
  for(i = 1; i <= n; i++){
    insert(i);
  }

  printLista();
  removeNode(3);
  printLista();

  return 0;
}
