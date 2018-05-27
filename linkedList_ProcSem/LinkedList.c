/* Implementação do benchmark LinkedList da RSTM em C */
/* Autor: Bruno Cesar, @bcesarg6, bcesar.g6@gmail.com */
/* Abril de 2018                                      */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "SharedMemoryController.h"

#define TRUE 1
#define FALSE 0

/* Estruturas */
typedef struct pthread_arg{
  int in;
  int out;
}pthread_arg;

int mem_ctrl_id;
LLNode* sentinela;
stats_t* stats;
sem_t* sem;
sh_mem_add_t sh_mem_adds;

/* Dados globais com valores padrão */
static int datasetsize = 256;                  // number of items
static double duration = 5.0f;                 // in seconds
static int doWarmup = FALSE;
static int num_ops = 0;                        // number of operations mode value.
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
    sem_wait(sem);
    LLNode* prev = sentinela;
    LLNode* curr = getNode(prev->next);

    while (curr != NULL) {
        if ((prev->val) >= (curr->val)) {
            printf("FAILED SANITY CHECK IN: %d < %d\n", prev->val, curr->val);
            sane = FALSE;
            break;
        }
        prev = curr;
        curr = getNode(curr->next);
    }
    sem_post(sem);
    return sane;
}


// insert method; find the right place in the list, add val so that it is in
// sorted order; if val is already in the list, exit without inserting
void insert(int val, int* done){
  int id;
  *done = 1;
  sem_wait(sem);
  //printf("Inserindo %d\n", val);
  // traverse the list to find the insertion point
  LLNode* prev = sentinela;
  LLNode* curr = getNode(prev->next);

  printf("prev: \n");
  printNode(prev);
  if(curr){
    printf("curr: \n");
    printNode(curr);
  }

  while (curr != NULL){
    //printf("Procuando fim\n");
    if (curr->val >= val)
      break;

    prev = curr;
    curr = getNode(curr->next);
  }

  // now insert new_node between prev and curr
  if (!curr || (curr->val > val)){
    // ESCRITA : REGIÃO CRITICA

    LLNode* insert_point = prev;
    LLNode* novo = shAlloc(&id);
    if(id == -1){
      sem_post(sem);
      *done = 0;
      printf("Número máximo de nós alcançado\n");
      return;
    }

    novo->val = val;
    novo->next = prev->next;
    novo->id = id;

    insert_point->next = novo->id;
    //printNode(novo);
    //printf("Fim inserir %d\n", val);
    }
    //printLista();
    sem_post(sem);
}

// search function
void lookup(void* arg){
  sem_wait(sem);
  pthread_arg* p = (pthread_arg*) arg;
  //printf("Procurando %d\n", p->in);
  int val = p->in;

  int found = FALSE;

  LLNode* curr = sentinela;
  curr = getNode(curr->next);

  while (curr != NULL) {
    if (curr->val >= val)
      break;

    curr = getNode(curr->next);
  }

  found = ((curr != NULL) && (curr->val == val));

  p->out = found;
  //printf("Fim procurar %d\n", p->in);
  sem_post(sem);
}

// remove a node if its value == val
void removeNode(int val){
  sem_wait(sem);
  //printf("Removendo %d\n", val);
  // find the node whose val matches the request
  LLNode* prev = sentinela;
  LLNode* curr = getNode(prev->next);

  while (curr != NULL) {
    // if we find the node, disconnect it and end the search
    if (curr->val == val) {
      printf("Achou %d, removendo\n", val);
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
    curr = getNode(prev->next);
  }
  //printf("Fim remover %d\n", val);
  //printLista();
  sem_post(sem);
}

// print the list
void printLista(){
    LLNode* curr = sentinela;
    //printNode(sentinela);
    curr = getNode(curr->next);

    printf("lista :");
    while (curr != NULL){
        printf(" %d ->", curr->val);
        curr = getNode(curr->next);
    }

    printf(" NULL\n\n");
}

void* experiment(void* arg, int tid){
  /* Garante thread id unico para a threads */
  printf("tid = %d\n", tid);

  int result, val, i, done;
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
        insert(val, &done);
        if(val > 500){
          printf("%d -> remove %d\n", tid, val);
          removeNode(val);
        }
        if(done)
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
        printf("%d -> insert %d\n", tid, val);
        insert(val, &done);
        if(done)
          l_inserts++;
      }
      else {
        removeNode(val);
        l_removes++;
      }

      //int sane = isSane();
      l_ops++;

      clock_gettime(CLOCK_MONOTONIC, &tend);
      timeDiff = ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
    }
  }

  sem_wait(sem);
  stats->count_ops += l_ops;
  stats->inserts += l_inserts;
  stats->lookups_true += l_lookups_true;
  stats->lookups_false += l_lookups_false;
  stats->removes += l_removes;
  sem_post(sem);

  exit(0);
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

void printNode(LLNode* node){
  printf("Dados do nó\n");
  printf("ID: %d\n", node->id);
  printf("VAL: %d\n", node->val);
  printf("NEXT: %d\n", node->next);

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
  int i, done, pid = 0, id = -1;
  printf("\nLinked List - versão sem\n");

	getArgs(argc, argv);
	checkData();
  printInfo();

  /* Inicializa a lista criando a sentinela */
  sentinela = malloc(sizeof(LLNode));


  pthread_t threads[n_threads];
  void* pth_status;

  /* Warm Up */
  // warmup inserts half of the elements in the datasetsize
  if(doWarmup){
      for (i = 0; i < datasetsize; i+=2) {
        insert(i, &done);
        if(!done)
        printf("Parando warmup, número máximo de nós atingido\n");
          break;
      }
  }

  clock_gettime(CLOCK_MONOTONIC, &tstart);
  timeDiff = 0;

  /* SHM */
  printf("\n\n\t--- Rodando experimentos ---\n");

  sh_mem_adds.ctrl_add = createShMem(1000); //1000 é o número de nós máximo

  getMemAdds(0);
  stats = sh_mem_adds.stats_add;

  sem = sem_open(SNAME, O_CREAT, 0644, 1);

  sentinela = shAlloc(&id);
  sentinela->val = -1;
  sentinela->next = -1;
  sentinela->id = id;

  //FORK
  for(i = 0; i < n_threads; i++){
    pid = fork();
    if(pid == -1){
      printf("Erro ao criar processo, abortando\n");
      exit(1);
    }
    else if(!pid){
      break;
    }
    else{
      printf("Filho %d criado\n", pid);
    }
  }

  if(!pid){
    printf("Iniciando processo filho\n");
    //A partir daqui todos o processos executam isso
    sh_mem_adds.ctrl_add = NULL;
    sh_mem_adds.free_list_add = NULL;
    sh_mem_adds.node_mem_add = NULL;
    sh_mem_adds.stats_add = NULL;

    getMemAdds(1);

    stats = sh_mem_adds.stats_add;
    sem = sem_open(SNAME, 0);

    //printf("%d %d\n", getpid(), sh_mem_adds.node_mem_add);
    //printNode(sentinela);

    experiment(NULL, getpid());
  }
  else{
    printf("Pai esperando\n");
    for(int i = 0; i < n_threads; i++){
      waitpid(-1, NULL, 0);
    }
    printf("Pai terminou\n");
  }

  //FORK "join"

  /* SHM_END */

  /* TRHEAD (Remover quando inserir o fork)
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
  printf("Total de operações realizadas = %d\n",stats->count_ops);
  printf("Total de lookups acertados: %d\n", stats->lookups_true);
  printf("Total de lookups falhados: %d\n", stats->lookups_false);
  printf("Total de Inserts: %d\n", stats->inserts);
  printf("Total de removes: %d\n", stats->removes);

  return 0;
}
