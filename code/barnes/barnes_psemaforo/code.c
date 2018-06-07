/*************************************************************************/
/*                                                                       */
/*  Copyright (c) 1994 Stanford University                               */
/*                                                                       */
/*  All rights reserved.                                                 */
/*                                                                       */
/*  Permission is given to use, copy, and modify this software for any   */
/*  non-commercial purpose as long as this copyright notice is not       */
/*  removed.  All other uses, including redistribution in whole or in    */
/*  part, are forbidden without prior written permission.                */
/*                                                                       */
/*  This software is provided with absolutely no warranty and no         */
/*  support.                                                             */
/*                                                                       */
/*************************************************************************/
/*
Usage: BARNES <options> < inputfile

Command line options:

    -h : Print out input file description

    Input parameters should be placed in a file and redirected through
    standard input.  There are a total of twelve parameters, and all of
    them have default values.

    1) infile (char*) : The name of an input file that contains particle
       data.

       The format of the file is:
         a) An int representing the number of particles in the distribution
         b) An int representing the dimensionality of the problem (3-D)
         c) A double representing the current time of the simulation
         d) Doubles representing the masses of all the particles
         e) A vector (length equal to the dimensionality) of doubles
            representing the positions of all the particles
         f) A vector (length equal to the dimensionality) of doubles
            representing the velocities of all the particles

       Each of these numbers can be separated by any amount of whitespace.
    2) nbody (int) : If no input file is specified (the first line is
       blank), this number specifies the number of particles to generate
       under a plummer model.  Default is 16384.
    3) seed (int) : The seed used by the random number generator.
       Default is 123.
    4) outfile (char*) : The name of the file that snapshots will be
       printed to. This feature has been disabled in the SPLASH release.
       Default is NULL.
    5) dtime (double) : The integration time-step.
       Default is 0.025.
    6) eps (double) : The usual potential softening
       Default is 0.05.
    7) tol (double) : The cell subdivision tolerance.
       Default is 1.0.
    8) fcells (double) : Number of cells created = fcells * number of
       leaves.
       Default is 2.0.
    9) fleaves (double) : Number of leaves created = fleaves * nbody.
       Default is 0.5.
    10) tstop (double) : The time to stop integration.
       Default is 0.075.
    11) dtout (double) : The data-output interval.
       Default is 0.25.
    12) NPROC (int) : The number of processors.
       Default is 1.
*/

/*
  Para a versão de processos, alocar memória compartilhada no começo, isto é,
  variáveis globais e array de variáveis locais, além dos semáforos.
  Assim, não será necessário mudar o código principal, apenas o alocamento.

  global bodytab precisa ser shared, mover para struct global
  */

#include <pthread.h>
#include <semaphore.h> /* Semaforos */
#include <sys/time.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#define MAX_THREADS 1024
pthread_t PThreadTable[MAX_THREADS];

/* VERSAO SEMAFOROS */

#define global  /* nada */

#include "code.h"
#include "defs.h"
#include <math.h>
#include <time.h>

string defv[] = {                 /* DEFAULT PARAMETER VALUES              */
    /* file names for input/output                                         */
    "in=",                        /* snapshot of initial conditions        */
    "out=",                       /* stream of output snapshots            */

    /* params, used if no input specified, to make a Plummer Model         */
    "nbody=16384",                /* number of particles to generate       */
    "seed=123",                   /* random number generator seed          */

    /* params to control N-body integration                                */
    "dtime=0.025",                /* integration time-step                 */
    "eps=0.05",                   /* usual potential softening             */
    "tol=1.0",                    /* cell subdivision tolerence            */
    "fcells=2.0",                 /* cell allocation parameter             */
    "fleaves=0.5",                 /* leaf allocation parameter             */

    "tstop=0.075",                 /* time to stop integration              */
    "dtout=0.25",                 /* data-output interval                  */

    "NPROC=1",                    /* number of processors                  */
};

void SlaveStart ();
void stepsystem (unsigned int ProcessId);
void ComputeForces ();
void Help();
FILE *fopen();
void getMemAdds(int ct, int btab, int lcl);

main(int argc, string argv[]) {
    int c;

    while ((c = getopt(argc, argv, "h")) != -1) {
        switch(c) {
            case 'h':
                Help();
                exit(-1);
                break;

            default:
                fprintf(stderr, "Only valid option is \"-h\".\n");
                exit(-1);
                break;
        }
    }

    ANLinit();
    initparam(argv, defv);
    startrun();
    initoutput();
    tab_init();

    Global->tracktime = 0;
    Global->partitiontime = 0;
    Global->treebuildtime = 0;
    Global->forcecalctime = 0;

    Global->current_id = 0;

    /* Make the master do slave work so we don't waste the processor */
    {
        struct timeval	FullTime;
        gettimeofday(&FullTime, NULL);
        (Global->computestart) = (unsigned long)(FullTime.tv_usec + FullTime.tv_sec * 1000000);
    };

    printf("COMPUTESTART  = %12lu\n",Global->computestart);

    {
      long	i, Error;
      int pid;
      for(i = 0; i < globalDefs->NPROC; i++){
        ProcessId = i;
        pid = fork();
        if(pid == -1){
          printf("Erro ao criar processo, abortando\n");
          exit(1);
        }
        else if(!pid){
          break;
        }
      }

      if(!pid){
        SlaveStart();
      }
      else{
        for(i = 0; i < globalDefs->NPROC; i++){
          waitpid(-1, NULL, 0);
        }



      {
          struct timeval	FullTime;
          gettimeofday(&FullTime, NULL);
          (Global->computeend) = (unsigned long)(FullTime.tv_usec + FullTime.tv_sec * 1000000);
      };


      printf("COMPUTEEND    = %12lu\n",Global->computeend);
      printf("COMPUTETIME   = %12lu\n",Global->computeend - Global->computestart);
      printf("TRACKTIME     = %12lu\n",Global->tracktime);
      printf("PARTITIONTIME = %12lu\t%5.2f\n",Global->partitiontime,
             ((float)Global->partitiontime)/Global->tracktime);
      printf("TREEBUILDTIME = %12lu\t%5.2f\n",Global->treebuildtime,
             ((float)Global->treebuildtime)/Global->tracktime);
      printf("FORCECALCTIME = %12lu\t%5.2f\n",Global->forcecalctime,
             ((float)Global->forcecalctime)/Global->tracktime);
      printf("RESTTIME      = %12lu\t%5.2f\n",
             Global->tracktime - Global->partitiontime -
             Global->treebuildtime - Global->forcecalctime,
             ((float)(Global->tracktime-Global->partitiontime-
                      Global->treebuildtime-Global->forcecalctime))/
             Global->tracktime);

      {exit(0);};
    }
  }
 }

/*
 * ANLINIT : initialize ANL macros
 */
ANLinit(){
    //Cria a estrutura controladora da memória
    if ((mem_ctrl_id = shmget(IPC_PRIVATE, sizeof(struct shMemCtrl), (SHM_R | SHM_W))) < 0){
      printf("Falha ao criar estrutura de memória compartilhada\n");
      exit(1);
    }

    if ((ctrl = shmat(mem_ctrl_id, NULL, 0)) == (void*) -1){
      printf("Falha ao ler estrutura de memória compartilhada na criação\n");
      exit(1);
    }
    shmctl(mem_ctrl_id, IPC_RMID, (struct shmid_ds *) NULL);


    //Cria a memória especificada
  	if ((ctrl->gldefsid = shmget(IPC_PRIVATE, sizeof(struct GlobalDefs), (SHM_R | SHM_W))) < 0){
  		printf("Falha ao criar memória compartilhada de nós\n");
  		exit(1);
  	}

    if ((ctrl->glid = shmget(IPC_PRIVATE, sizeof(struct GlobalMemory), (SHM_R | SHM_W))) < 0){
  		printf("Falha ao criar memória compartilhada de nós\n");
  		exit(1);
  	}

    if ((ctrl->localid = shmget(IPC_PRIVATE, (sizeof(struct local_memory) * MAX_PROC), (SHM_R | SHM_W))) < 0){
  		printf("Falha ao criar memória compartilhada de nós\n");
  		exit(1);
  	}

    getMemAdds(0, 0, 0);

    (Global->Barload).counter = 0;
    (Global->Barload).cycle = 0;
    sem_init(&(Global->Barload).sem_count, 1, 1);
    sem_init(&(Global->Barload).sem_bar, 1, 0);

    (Global->Bartree).counter = 0;
    (Global->Bartree).cycle = 0;
    sem_init(&(Global->Bartree).sem_count, 1, 1);
    sem_init(&(Global->Bartree).sem_bar, 1, 0);

    (Global->Barcom).counter = 0;
    (Global->Barcom).cycle = 0;
    sem_init(&(Global->Barcom).sem_count, 1, 1);
    sem_init(&(Global->Barcom).sem_bar, 1, 0);

    (Global->Baraccel).counter = 0;
    (Global->Baraccel).cycle = 0;
    sem_init(&(Global->Baraccel).sem_count, 1, 1);
    sem_init(&(Global->Baraccel).sem_bar, 1, 0);

    (Global->Barstart).counter = 0;
    (Global->Barstart).cycle = 0;
    sem_init(&(Global->Barstart).sem_count, 1, 1);
    sem_init(&(Global->Barstart).sem_bar, 1, 0);

    (Global->Barpos).counter = 0;
    (Global->Barpos).cycle = 0;
    sem_init(&(Global->Barpos).sem_count, 1, 1);
    sem_init(&(Global->Barpos).sem_bar, 1, 0);

    /* Inicializa semaforos */
    sem_init(&(Global->CountSem), 1, 1);
    sem_init(&(Global->io_sem), 1, 1);
 }

/*
 * INIT_ROOT: Processor 0 reinitialize the global root at each time step
 */
void init_root (unsigned int ProcessId){
  int i;

  Global->G_root=Local[0].ctab;
  Type(Global->G_root) = CELL;
  Done(Global->G_root) = FALSE;
  Level(Global->G_root) = IMAX >> 1;
  for (i = 0; i < NSUB; i++) {
    Subp(Global->G_root)[i] = NULL;
  }
  Local[0].mynumcell=1;
}

int Log_base_2(int number) {
  int cumulative;
  int out;

  cumulative = 1;
  for (out = 0; out < 20; out++) {
    if (cumulative == number) {
      return(out);
    }
    else {
      cumulative = cumulative * 2;
    }
  }

  fprintf(stderr,"Log_base_2: couldn't find log2 of %d\n", number);
  exit(-1);
}


/*
 * TAB_INIT : allocate body and cell data space
 */
tab_init(){
  //printf("Tab init start\n");
  cellptr pc;
  int i;
  char *starting_address, *ending_address;

  /* remover alocação dentro do for e fazer com memória contígua. será necessário fazer
  distribuição depois do fork */

  /*allocate leaf/cell space */
  globalDefs->maxleaf = (int) ((double) globalDefs->fleaves * globalDefs->nbody);
  globalDefs->maxcell = globalDefs->fcells * globalDefs->maxleaf;

  if ((ctrl->ctabid = shmget(IPC_PRIVATE, (globalDefs->maxcell * sizeof(cell)), (SHM_R | SHM_W))) < 0){
    printf("Falha ao criar estrutura de memória compartilhada\n");
    exit(1);
  }

  if ((Local[0].ctab = shmat(ctrl->ctabid, NULL, 0)) == (void*) -1){
    printf("Falha ao ler estrutura de memória compartilhada na criação\n");
    exit(1);
  }
  shmctl(ctrl->ctabid, IPC_RMID, (struct shmid_ds *) NULL);
  //printf("Alocado cell\n");

  if ((ctrl->ltabid = shmget(IPC_PRIVATE, (globalDefs->maxcell * sizeof(cell)), (SHM_R | SHM_W))) < 0){
    printf("Falha ao criar estrutura de memória compartilhada\n");
    exit(1);
  }

  if ((Local[0].ltab = shmat(ctrl->ltabid, NULL, 0)) == (void*) -1){
    printf("Falha ao ler estrutura de memória compartilhada na criação\n");
    exit(1);
  }
  shmctl(ctrl->ltabid, IPC_RMID, (struct shmid_ds *) NULL);
  //printf("Alocado leaf\n");

  /*for (i = 0; i < globalDefs->NPROC; ++i) {
    Local[i].ctab = (cellptr) malloc((globalDefs->maxcell / globalDefs->NPROC) * sizeof(cell));;
    Local[i].ltab = (leafptr) malloc((globalDefs->maxleaf / globalDefs->NPROC) * sizeof(leaf));;
  }*/

  /*allocate space for personal lists of body pointers */
  globalDefs->maxmybody = (globalDefs->nbody+globalDefs->maxleaf*MAX_BODIES_PER_LEAF)/globalDefs->NPROC;
  if ((ctrl->mbodyid = shmget(IPC_PRIVATE, (globalDefs->NPROC * globalDefs->maxmybody*sizeof(bodyptr)), (SHM_R | SHM_W))) < 0){
    printf("Falha ao criar estrutura de memória compartilhada\n");
    exit(1);
  }

  if ((Local[0].mybodytab = shmat(ctrl->mbodyid, NULL, 0)) == (void*) -1){
    printf("Falha ao ler estrutura de memória compartilhada na criação\n");
    exit(1);
  }
  shmctl(ctrl->mbodyid, IPC_RMID, (struct shmid_ds *) NULL);
  //printf("Alocado mybody\n");

  //Local[0].mybodytab = (bodyptr*) malloc(globalDefs->NPROC * globalDefs->maxmybody*sizeof(bodyptr));;

  /* space is allocated so that every */
  /* process can have a maximum of maxmybody pointers to bodies */
  /* then there is an array of bodies called bodytab which is  */
  /* allocated in the distribution generation or when the distr. */
  /* file is read */
  globalDefs->maxmycell = globalDefs->maxcell / globalDefs->NPROC;
  globalDefs->maxmyleaf = globalDefs->maxleaf / globalDefs->NPROC;
  if ((ctrl->mcellid = shmget(IPC_PRIVATE, (globalDefs->NPROC * globalDefs->maxmycell*sizeof(cellptr)), (SHM_R | SHM_W))) < 0){
    printf("Falha ao criar estrutura de memória compartilhada\n");
    exit(1);
  }

  if ((Local[0].mycelltab = shmat(ctrl->mcellid, NULL, 0)) == (void*) -1){
    printf("Falha ao ler estrutura de memória compartilhada na criação\n");
    exit(1);
  }
  shmctl(ctrl->mcellid, IPC_RMID, (struct shmid_ds *) NULL);

  //printf("Alocado my cell\n");

  if ((ctrl->mleafid = shmget(IPC_PRIVATE, (globalDefs->NPROC * globalDefs->maxmyleaf*sizeof(leafptr)), (SHM_R | SHM_W))) < 0){
    printf("Falha ao criar estrutura de memória compartilhada\n");
    exit(1);
  }

  if ((Local[0].myleaftab = shmat(ctrl->mleafid, NULL, 0)) == (void*) -1){
    printf("Falha ao ler estrutura de memória compartilhada na criação\n");
    exit(1);
  }
  shmctl(ctrl->mleafid, IPC_RMID, (struct shmid_ds *) NULL);
  //printf("Alocado my leaf\n");

  //Local[0].mycelltab = (cellptr*) malloc(globalDefs->NPROC * globalDefs->maxmycell*sizeof(cellptr));;
  //Local[0].myleaftab = (leafptr*) malloc(globalDefs->NPROC * globalDefs->maxmyleaf*sizeof(leafptr));;

  if ((ctrl->cellid = shmget(IPC_PRIVATE, (sizeof(struct CellSemType)), (SHM_R | SHM_W))) < 0){
    printf("Falha ao criar estrutura de memória compartilhada\n");
    exit(1);
  }

  if ((globalDefs->CellSem = shmat(ctrl->cellid, NULL, 0)) == (void*) -1){
    printf("Falha ao ler estrutura de memória compartilhada na criação\n");
    exit(1);
  }
  shmctl(ctrl->cellid, IPC_RMID, (struct shmid_ds *) NULL);

  //globalDefs->CellSem = (struct CellSemType *) malloc(sizeof(struct CellSemType));;
  //printf("Alocado cellsem\n");

  {
    unsigned long	i, Error;
    for (i = 0; i < MAXLOCK; i++) {
      Error = sem_init(&(globalDefs->CellSem->CL[i]), 1, 1);
      if (Error != 0) {
        printf("Error while initializing array of semaphores.\n");
        exit(-1);
      }
    }
  };
  //printf("Tab init end\n");
}

/*
 * SLAVESTART: main task for each processor
 */
void SlaveStart(){
/* POSSIBLE ENHANCEMENT:  Here is where one might pin processes to
   processors to avoid migration */
   getMemAdds(1, 1, 1);
   /* initialize mybodytabs */
   Local[ProcessId].mybodytab = Local[0].mybodytab + (globalDefs->maxmybody * ProcessId);
   /* note that every process has its own copy   */
   /* of mybodytab, which was initialized to the */
   /* beginning of the whole array by proc. 0    */
   /* before create                              */
   Local[ProcessId].mycelltab = Local[0].mycelltab + (globalDefs->maxmycell * ProcessId);
   Local[ProcessId].myleaftab = Local[0].myleaftab + (globalDefs->maxmyleaf * ProcessId);
   Local[ProcessId].ctab = Local[0].ctab + (globalDefs->maxcell/globalDefs->NPROC * ProcessId);
   Local[ProcessId].ltab = Local[0].ltab + (globalDefs->maxleaf/globalDefs->NPROC * ProcessId);
   Local[ProcessId].tout = Local[0].tout;
   Local[ProcessId].tnow = Local[0].tnow;
   Local[ProcessId].nstep = Local[0].nstep;

   find_my_initial_bodies(globalDefs->bodytab, globalDefs->nbody, ProcessId);

   /* main loop */
   while (Local[ProcessId].tnow < globalDefs->tstop + 0.1 * globalDefs->dtime) {
     stepsystem(ProcessId);
   }
}

/*
 * STARTRUN: startup hierarchical N-body code.
 */

startrun(){
   string getparam();
   int getiparam();
   bool getbparam();
   double getdparam();
   int seed;

   globalDefs->infile = getparam("in");
   if (*(globalDefs->infile) != NULL) {
     inputdata();
   }
   else {
     globalDefs->nbody = getiparam("nbody");
     if (globalDefs->nbody < 1) {
       error1("startrun: absurd nbody\n");
     }
     seed = getiparam("seed");
   }

   globalDefs->outfile = getparam("out");
   globalDefs->dtime = getdparam("dtime");
   globalDefs->dthf = 0.5 * globalDefs->dtime;
   globalDefs->eps = getdparam("eps");
   globalDefs->epssq = globalDefs->eps*globalDefs->eps;
   globalDefs->tol = getdparam("tol");
   globalDefs->tolsq = globalDefs->tol*globalDefs->tol;
   globalDefs->fcells = getdparam("fcells");
   globalDefs->fleaves = getdparam("fleaves");
   globalDefs->tstop = getdparam("tstop");
   globalDefs->dtout = getdparam("dtout");
   globalDefs->NPROC = getiparam("NPROC");
   Local[0].nstep = 0;
   pranset(seed);
   testdata();
   setbound();
   Local[0].tout = Local[0].tnow + globalDefs->dtout;
}

/*
 * TESTDATA: generate Plummer model initial conditions for test runs,
 * scaled to units such that M = -4E = G = 1 (Henon, Hegge, etc).
 * See Aarseth, SJ, Henon, M, & Wielen, R (1974) Astr & Ap, 37, 183.
 */

 #define MFRAC  0.999                /* mass cut off at MFRAC of total */

testdata(){
   real rsc, vsc, sqrt(), xrand(), pow(), rsq, r, v, x, y;
   vector cmr, cmv;
   register bodyptr p;
   int rejects = 0;
   int k;
   int halfnbody, i;
   float offset;
   register bodyptr cp;
   double tmp;

   globalDefs->headline = "Hack code: Plummer model";
   Local[0].tnow = 0.0;
   //globalDefs->bodytab = (bodyptr) malloc(globalDefs->nbody * sizeof(body));;

   if ((ctrl->btabid = shmget(IPC_PRIVATE, (globalDefs->nbody * sizeof(body)), (SHM_R | SHM_W))) < 0){
     printf("Falha ao criar estrutura de memória compartilhada\n");
     exit(1);
   }

   if ((globalDefs->bodytab = shmat(ctrl->btabid, NULL, 0)) == (void*) -1){
     printf("Falha ao ler estrutura de memória compartilhada na criação\n");
     exit(1);
   }
   shmctl(ctrl->btabid, IPC_RMID, (struct shmid_ds *) NULL);

   if (globalDefs->bodytab == NULL) {
     error1("testdata: not enuf memory\n");
   }
   rsc = 9 * PI / 16;
   vsc = sqrt(1.0 / rsc);

   CLRV(cmr);
   CLRV(cmv);

   halfnbody = globalDefs->nbody / 2;
   if (globalDefs->nbody % 2 != 0) halfnbody++;
   for (p = globalDefs->bodytab; p < globalDefs->bodytab+halfnbody; p++) {
     Type(p) = BODY;
     Mass(p) = 1.0 / globalDefs->nbody;
     Cost(p) = 1;

     r = 1 / sqrt(pow(xrand(0.0, MFRAC), -2.0/3.0) - 1);
     /*   reject radii greater than 10 */
     while (r > 9.0) {
       rejects++;
       r = 1 / sqrt(pow(xrand(0.0, MFRAC), -2.0/3.0) - 1);
     }

     pickshell(Pos(p), rsc * r);
     ADDV(cmr, cmr, Pos(p));

     do {
       x = xrand(0.0, 1.0);
       y = xrand(0.0, 0.1);
     } while (y > x*x * pow(1 - x*x, 3.5));

     v = sqrt(2.0) * x / pow(1 + r*r, 0.25);
     pickshell(Vel(p), vsc * v);
     ADDV(cmv, cmv, Vel(p));
   }

   offset = 4.0;

   for (p = globalDefs->bodytab + halfnbody; p < globalDefs->bodytab+globalDefs->nbody; p++) {
     Type(p) = BODY;
     Mass(p) = 1.0 / globalDefs->nbody;
     Cost(p) = 1;

     cp = p - halfnbody;
     for (i = 0; i < NDIM; i++){
       Pos(p)[i] = Pos(cp)[i] + offset;
       ADDV(cmr, cmr, Pos(p));
       Vel(p)[i] = Vel(cp)[i];
       ADDV(cmv, cmv, Vel(p));
     }
   }

   DIVVS(cmr, cmr, (real) globalDefs->nbody);
   DIVVS(cmv, cmv, (real) globalDefs->nbody);

   for (p = globalDefs->bodytab; p < globalDefs->bodytab+globalDefs->nbody; p++) {
     SUBV(Pos(p), Pos(p), cmr);
     SUBV(Vel(p), Vel(p), cmv);
   }
}

/*
 * PICKSHELL: pick a random point on a sphere of specified radius.
 */
/* vec = coordinate vector chosen */
/* rad = radius of chosen point */
void pickshell(real vec[], real rad){
  register int k;
  double rsq, xrand(), sqrt(), rsc;

  do {
    for (k = 0; k < NDIM; k++) {
      vec[k] = xrand(-1.0, 1.0);
    }

    DOTVP(rsq, vec, vec);
  } while (rsq > 1.0);

  rsc = rad / sqrt(rsq);
  MULVS(vec, vec, rsc);
}

int intpow(int i, int j){
    int k;
    int temp = 1;

    for (k = 0; k < j; k++){
      temp = temp*i;
    }

    return temp;
}

/*
 * STEPSYSTEM: advance N-body system one time-step.
 */

void stepsystem (unsigned int ProcessId){
  int i;
  real Cavg;
  bodyptr p,*pp;
  vector acc1, dacc, dvel, vel1, dpos;
  int intpow();
  unsigned int time;
  unsigned int trackstart, trackend;
  unsigned int partitionstart, partitionend;
  unsigned int treebuildstart, treebuildend;
  unsigned int forcecalcstart, forcecalcend;

  if ((ProcessId == 0) && (Local[ProcessId].nstep >= 2)) {
    {
      struct timeval	FullTime;
      gettimeofday(&FullTime, NULL);
      (trackstart) = (unsigned long)(FullTime.tv_usec + FullTime.tv_sec * 1000000);
    };
  }

  if (ProcessId == 0) {
    init_root(ProcessId);
  } else {
    Local[ProcessId].mynumcell = 0;
    Local[ProcessId].mynumleaf = 0;
  }

  /* start at same time */
  {
    unsigned long	Error, Cycle;
    int		i, Cancel, Temp;

    //IMPLEMENTAÇÃO SEMAFOROS
    sem_wait(&(Global->Barcom).sem_count);

    if((Global->Barcom).counter == (globalDefs->NPROC - 1)){
      /* Se entrou é a ultima thread */
      (Global->Barcom).counter = 0;
       //printf("Barcom\tOi eu sou a thread %d OMG!!! eu sou a ultima!!! :o\n\n\n", ProcessId );

      sem_post(&(Global->Barcom).sem_count);

      /* Libera todas as threads*/
      for (i = 0; i < (globalDefs->NPROC - 1); i++) {
        sem_post(&(Global->Barcom).sem_bar);
      }

      }
    else {

      /* NÃO é a ultima thread, então será bloqueada na barreira */
      //printf("Barcom\tOi eu sou a thread %d e não sou a ultima :)\n", ProcessId );
      (Global->Barcom).counter++;
      sem_post(&(Global->Barcom).sem_count);

      sem_wait(&(Global->Barcom).sem_bar);
    }
    //printf("-----Liberou %d-----\n", ProcessId);
  };

  if ((ProcessId == 0) && (Local[ProcessId].nstep >= 2)) {
    {
      struct timeval	FullTime;

      gettimeofday(&FullTime, NULL);
      (treebuildstart) = (unsigned long)(FullTime.tv_usec + FullTime.tv_sec * 1000000);
    };
  }

  /* load bodies into tree   */
  maketree(ProcessId);
  if ((ProcessId == 0) && (Local[ProcessId].nstep >= 2)) {
    {
      struct timeval	FullTime;
      gettimeofday(&FullTime, NULL);

      (treebuildend) = (unsigned long)(FullTime.tv_usec + FullTime.tv_sec * 1000000);
    };
    Global->treebuildtime += treebuildend - treebuildstart;
  }

  Housekeep(ProcessId);

  Cavg = (real) Cost(Global->G_root) / (real)globalDefs->NPROC ;
  Local[ProcessId].workMin = (int) (Cavg * ProcessId);
  Local[ProcessId].workMax = (int) (Cavg * (ProcessId + 1) + (ProcessId == (globalDefs->NPROC - 1)));

  if ((ProcessId == 0) && (Local[ProcessId].nstep >= 2)) {
    {
      struct timeval	FullTime;
      gettimeofday(&FullTime, NULL);
      (partitionstart) = (unsigned long)(FullTime.tv_usec + FullTime.tv_sec * 1000000);
    };
  }

  Local[ProcessId].mynbody = 0;
  find_my_bodies(Global->G_root, 0, BRC_FUC, ProcessId);

  /*     B*RRIER(Global->Barcom,1); */
  if ((ProcessId == 0) && (Local[ProcessId].nstep >= 2)) {
    {
      struct timeval	FullTime;
      gettimeofday(&FullTime, NULL);
      (partitionend) = (unsigned long)(FullTime.tv_usec + FullTime.tv_sec * 1000000);
    };
    Global->partitiontime += partitionend - partitionstart;
  }

  if ((ProcessId == 0) && (Local[ProcessId].nstep >= 2)) {
    {
      struct timeval	FullTime;

      gettimeofday(&FullTime, NULL);
      (forcecalcstart) = (unsigned long)(FullTime.tv_usec + FullTime.tv_sec * 1000000);
    };
  }

  ComputeForces(ProcessId);

  if ((ProcessId == 0) && (Local[ProcessId].nstep >= 2)) {
    {
      struct timeval	FullTime;

      gettimeofday(&FullTime, NULL);
      (forcecalcend) = (unsigned long)(FullTime.tv_usec + FullTime.tv_sec * 1000000);
    };

    Global->forcecalctime += forcecalcend - forcecalcstart;
  }

  /* advance my bodies */
  for (pp = Local[ProcessId].mybodytab;
    pp < Local[ProcessId].mybodytab+Local[ProcessId].mynbody; pp++) {
      p = *pp;
      MULVS(dvel, Acc(p), globalDefs->dthf);
      ADDV(vel1, Vel(p), dvel);
      MULVS(dpos, vel1, globalDefs->dtime);
      ADDV(Pos(p), Pos(p), dpos);
      ADDV(Vel(p), vel1, dvel);

      for (i = 0; i < NDIM; i++) {
        if (Pos(p)[i]<Local[ProcessId].min[i]) {
          Local[ProcessId].min[i]=Pos(p)[i];
        }
        if (Pos(p)[i]>Local[ProcessId].max[i]) {
          Local[ProcessId].max[i]=Pos(p)[i] ;
        }
      }
    }

    sem_wait(&(Global->CountSem));

    for (i = 0; i < NDIM; i++) {
      if (Global->min[i] > Local[ProcessId].min[i]) {
        Global->min[i] = Local[ProcessId].min[i];
      }
      if (Global->max[i] < Local[ProcessId].max[i]) {
        Global->max[i] = Local[ProcessId].max[i];
      }
    }

    sem_post(&(Global->CountSem));

    /* bar needed to make sure that every process has computed its min */
    /* and max coordinates, and has accumulated them into the global   */
    /* min and max, before the new dimensions are computed	       */

    {
      unsigned long	Error, Cycle;
      int	i, Cancel, Temp, value;

      //IMPLEMENTAÇÃO SEMAFOROS

      sem_wait(&(Global->Barpos).sem_count);

      if((Global->Barpos).counter == (globalDefs->NPROC - 1)){
        /* Se entrou é a ultima thread */
        //printf("Barpos\tOi eu sou a thread %d OMG!!! eu sou a ultima!!! :o\n\n\n", ProcessId );
        (Global->Barpos).counter = 0;

        sem_post(&(Global->Barpos).sem_count);

        /* Libera todas as threads*/
        for (i = 0; i < (globalDefs->NPROC - 1); i++) {
          sem_post(&(Global->Barpos).sem_bar);
        }

        } else {
        /* NÃO é a ultima thread, então será bloqueada na barreira */
        //printf("Barpos\tOi eu sou a thread %d e não sou a ultimaaa :)\n", ProcessId );
        (Global->Barpos).counter++;
        sem_post(&(Global->Barpos).sem_count);

        sem_wait(&(Global->Barpos).sem_bar);
        }
    };
    //printf("-----Liberou %d-----\n", ProcessId);


    if ((ProcessId == 0) && (Local[ProcessId].nstep >= 2)) {
      {
        struct timeval	FullTime;

        gettimeofday(&FullTime, NULL);
        (trackend) = (unsigned long)(FullTime.tv_usec + FullTime.tv_sec * 1000000);
      };
      Global->tracktime += trackend - trackstart;
    }
    if (ProcessId==0) {
      Global->rsize=0;
      SUBV(Global->max,Global->max,Global->min);
      for (i = 0; i < NDIM; i++) {
        if (Global->rsize < Global->max[i]) {
          Global->rsize = Global->max[i];
        }
      }
      ADDVS(Global->rmin,Global->min,-Global->rsize/100000.0);
      Global->rsize = 1.00002*Global->rsize;
      SETVS(Global->min,1E99);
      SETVS(Global->max,-1E99);
    }
    Local[ProcessId].nstep++;
    Local[ProcessId].tnow = Local[ProcessId].tnow + globalDefs->dtime;
  }

void ComputeForces (unsigned int ProcessId){
  bodyptr p,*pp;
  vector acc1, dacc, dvel, vel1, dpos;

  for (pp = Local[ProcessId].mybodytab;
       pp < Local[ProcessId].mybodytab+Local[ProcessId].mynbody;pp++) {

         p = *pp;
         SETV(acc1, Acc(p));
         Cost(p)=0;
         hackgrav(p,ProcessId);
         Local[ProcessId].myn2bcalc += Local[ProcessId].myn2bterm;
         Local[ProcessId].mynbccalc += Local[ProcessId].mynbcterm;
         if (!Local[ProcessId].skipself) {       /*   did we miss self-int?  */
           Local[ProcessId].myselfint++;        /*   count another goofup   */
         }
         if (Local[ProcessId].nstep > 0) {
           /*   use change in accel to make 2nd order correction to vel      */
           SUBV(dacc, Acc(p), acc1);
           MULVS(dvel, dacc, globalDefs->dthf);
           ADDV(Vel(p), Vel(p), dvel);
         }
       }
     }

/*
 * FIND_MY_INITIAL_BODIES: puts into mybodytab the initial list of bodies
 * assigned to the processor.
 */
void find_my_initial_bodies(bodyptr btab, int nbody, unsigned int ProcessId){
  int Myindex;
  int intpow();
  int equalbodies;
  int extra,offset,i;

  Local[ProcessId].mynbody = nbody / globalDefs->NPROC;
  extra = nbody % globalDefs->NPROC;
  if (ProcessId < extra) {
    Local[ProcessId].mynbody++;
    offset = Local[ProcessId].mynbody * ProcessId;
  }

  if (ProcessId >= extra) {
    offset = (Local[ProcessId].mynbody+1) * extra + (ProcessId - extra)
    * Local[ProcessId].mynbody;
  }

  for (i=0; i < Local[ProcessId].mynbody; i++) {
    Local[ProcessId].mybodytab[i] = &(btab[offset+i]);
  }

  {
    unsigned long	Error, Cycle;
    int	i, Cancel, Temp, value;

    //IMPLEMENTAÇÃO SEMAFOROS
    //sem_getvalue(&(Global->Barstart).sem_count, &value);
    //printf("Esperando count %d, val: %d\n", ProcessId, value);
    sem_wait(&(Global->Barstart).sem_count);

    if((Global->Barstart).counter == (globalDefs->NPROC - 1)){
      /* Se entrou é a ultima thread */
      //printf("Barstart1\tOi eu sou a thread %d OMG!!! eu sou a ultimaaa!!! :o\n\n\n", ProcessId );
      (Global->Barstart).counter = 0;

      sem_post(&(Global->Barstart).sem_count);
      //printf("Liberou count %d\n", ProcessId);

      /* Libera todas as threads*/
      for (i = 0; i < (globalDefs->NPROC - 1); i++) {
        sem_post(&(Global->Barstart).sem_bar);
        //sem_getvalue(&(Global->Barstart).sem_bar, &value);
        //printf("Liberando bar %d, val: %d\n", i, value);
      }

    } else {

      /* NÃO é a ultima thread, então será bloqueada na barreira */
      //printf("Barstart1\tOi eu sou a thread %d e não sou a ultimaaa :)\n", ProcessId );
      (Global->Barstart).counter++;
      //printf("count de %d: %d\n", ProcessId, (Global->Barstart).counter);
      sem_post(&(Global->Barstart).sem_count);
      //printf("Liberou count %d\n", ProcessId);
      //printf("Esperando bar %d\n", ProcessId);

      sem_wait(&(Global->Barstart).sem_bar);
      }
  };
  //printf("--------Liberou %d------\n", ProcessId);
}

void find_my_bodies(nodeptr mycell, int work, int direction, unsigned ProcessId){
  int i;
  leafptr l;
  nodeptr qptr;

  if (Type(mycell) == LEAF) {
    l = (leafptr) mycell;
    for (i = 0; i < l->num_bodies; i++) {
      if (work >= Local[ProcessId].workMin - .1) {
        if((Local[ProcessId].mynbody+2) > globalDefs->maxmybody) {
          error3("find_my_bodies: Processor %d needs more than %d bodies; increase fleaves\n",ProcessId, globalDefs->maxmybody);
        }
        Local[ProcessId].mybodytab[Local[ProcessId].mynbody++] =
        Bodyp(l)[i];
      }
      work += Cost(Bodyp(l)[i]);
      if (work >= Local[ProcessId].workMax-.1) {
        break;
      }
    }
  }
  else {
    for(i = 0; (i < NSUB) && (work < (Local[ProcessId].workMax - .1)); i++){
      qptr = Subp(mycell)[Child_Sequence[direction][i]];
      if (qptr!=NULL) {
        if ((work+Cost(qptr)) >= (Local[ProcessId].workMin -.1)) {
          find_my_bodies(qptr,work, Direction_Sequence[direction][i],
            ProcessId);
          }
          work += Cost(qptr);
        }
      }
    }
}

/*
 * HOUSEKEEP: reinitialize the different variables (in particular global
 * variables) between each time step.
 */

Housekeep(unsigned ProcessId){
  Local[ProcessId].myn2bcalc = Local[ProcessId].mynbccalc = Local[ProcessId].myselfint = 0;
  SETVS(Local[ProcessId].min,1E99);
  SETVS(Local[ProcessId].max,-1E99);
}

/*
 * SETBOUND: Compute the initial size of the root of the tree; only done
 * before first time step, and only processor 0 does it
 */
setbound(){
  int i;
  real side ;
  bodyptr p;

  SETVS(Local[0].min,1E99);
  SETVS(Local[0].max,-1E99);
  side=0;

  for (p = globalDefs->bodytab; p < globalDefs->bodytab+globalDefs->nbody; p++) {
    for (i=0; i<NDIM;i++) {
      if (Pos(p)[i]<Local[0].min[i]) Local[0].min[i]=Pos(p)[i] ;
      if (Pos(p)[i]>Local[0].max[i])  Local[0].max[i]=Pos(p)[i] ;
    }
  }

  SUBV(Local[0].max,Local[0].max,Local[0].min);
  for (i=0; i<NDIM;i++) if (side<Local[0].max[i]) side=Local[0].max[i];
  ADDVS(Global->rmin,Local[0].min,-side/100000.0);
  Global->rsize = 1.00002*side;
  SETVS(Global->max,-1E99);
  SETVS(Global->min,1E99);
}

void getMemAdds(int ct, int btab, int lcl){
  if(ct){
		if((ctrl = shmat(mem_ctrl_id, NULL, 0)) == (void*) -1){
			printf("Falha ao ler estrutura de memória compartilhada\n");
			exit(1);
		}
		shmctl(mem_ctrl_id, IPC_RMID, (struct shmid_ds *) NULL);
	}

  if(btab){
    if ((globalDefs->bodytab = shmat(ctrl->btabid, NULL, 0)) == (void*) -1){
      printf("Falha ao ler estrutura de memória compartilhada na criação\n");
      exit(1);
    }
    shmctl(ctrl->btabid, IPC_RMID, (struct shmid_ds *) NULL);
  }

	if((globalDefs = shmat(ctrl->gldefsid, NULL, 0)) == (void*) -1){
		printf("Falha ao ler global defs, id: %d\n", ctrl->gldefsid);
		exit(1);
	}
	shmctl(ctrl->gldefsid, IPC_RMID, (struct shmid_ds *) NULL);

  if((Global = shmat(ctrl->glid, NULL, 0)) == (void*) -1){
		printf("Falha ao ler global, id: %d\n", ctrl->glid);
		exit(1);
	}
	shmctl(ctrl->glid, IPC_RMID, (struct shmid_ds *) NULL);

  if((Local = shmat(ctrl->localid, NULL, 0)) == (void*) -1){
		printf("Falha ao ler local, id: %d\n", ctrl->localid);
		exit(1);
	}
	shmctl(ctrl->localid, IPC_RMID, (struct shmid_ds *) NULL);

  if(lcl){
    if ((Local[0].ctab = shmat(ctrl->ctabid, NULL, 0)) == (void*) -1){
      printf("Falha ao ler estrutura de memória compartilhada na criação\n");
      exit(1);
    }
    shmctl(ctrl->ctabid, IPC_RMID, (struct shmid_ds *) NULL);

    if ((Local[0].ltab = shmat(ctrl->ltabid, NULL, 0)) == (void*) -1){
      printf("Falha ao ler estrutura de memória compartilhada na criação\n");
      exit(1);
    }
    shmctl(ctrl->ltabid, IPC_RMID, (struct shmid_ds *) NULL);

    if ((Local[0].mybodytab = shmat(ctrl->mbodyid, NULL, 0)) == (void*) -1){
      printf("Falha ao ler estrutura de memória compartilhada na criação\n");
      exit(1);
    }
    shmctl(ctrl->mbodyid, IPC_RMID, (struct shmid_ds *) NULL);

    if ((Local[0].mycelltab = shmat(ctrl->mcellid, NULL, 0)) == (void*) -1){
      printf("Falha ao ler estrutura de memória compartilhada na criação\n");
      exit(1);
    }
    shmctl(ctrl->mcellid, IPC_RMID, (struct shmid_ds *) NULL);
    if ((Local[0].myleaftab = shmat(ctrl->mleafid, NULL, 0)) == (void*) -1){
      printf("Falha ao ler estrutura de memória compartilhada na criação\n");
      exit(1);
    }
    shmctl(ctrl->mleafid, IPC_RMID, (struct shmid_ds *) NULL);
    if ((globalDefs->CellSem = shmat(ctrl->cellid, NULL, 0)) == (void*) -1){
      printf("Falha ao ler estrutura de memória compartilhada na criação\n");
      exit(1);
    }
    shmctl(ctrl->cellid, IPC_RMID, (struct shmid_ds *) NULL);
  }
}

void Help (){
   printf("There are a total of twelve parameters, and all of them have default values.\n");
   printf("\n");
   printf("1) infile (char*) : The name of an input file that contains particle data.  \n");
   printf("    The format of the file is:\n");
   printf("\ta) An int representing the number of particles in the distribution\n");
   printf("\tb) An int representing the dimensionality of the problem (3-D)\n");
   printf("\tc) A double representing the current time of the simulation\n");
   printf("\td) Doubles representing the masses of all the particles\n");
   printf("\te) A vector (length equal to the dimensionality) of doubles\n");
   printf("\t   representing the positions of all the particles\n");
   printf("\tf) A vector (length equal to the dimensionality) of doubles\n");
   printf("\t   representing the velocities of all the particles\n");
   printf("\n");
   printf("    Each of these numbers can be separated by any amount of whitespace.\n");
   printf("\n");
   printf("2) nbody (int) : If no input file is specified (the first line is blank), this\n");
   printf("    number specifies the number of particles to generate under a plummer model.\n");
   printf("    Default is 16384.\n");
   printf("\n");
   printf("3) seed (int) : The seed used by the random number generator.\n");
   printf("    Default is 123.\n");
   printf("\n");
   printf("4) outfile (char*) : The name of the file that snapshots will be printed to. \n");
   printf("    This feature has been disabled in the SPLASH release.\n");
   printf("    Default is NULL.\n");
   printf("\n");
   printf("5) dtime (double) : The integration time-step.\n");
   printf("    Default is 0.025.\n");
   printf("\n");
   printf("6) eps (double) : The usual potential softening\n");
   printf("    Default is 0.05.\n");
   printf("\n");
   printf("7) tol (double) : The cell subdivision tolerance.\n");
   printf("    Default is 1.0.\n");
   printf("\n");
   printf("8) fcells (double) : The total number of cells created is equal to \n");
   printf("    fcells * number of leaves.\n");
   printf("    Default is 2.0.\n");
   printf("\n");
   printf("9) fleaves (double) : The total number of leaves created is equal to  \n");
   printf("    fleaves * nbody.\n");
   printf("    Default is 0.5.\n");
   printf("\n");
   printf("10) tstop (double) : The time to stop integration.\n");
   printf("    Default is 0.075.\n");
   printf("\n");
   printf("11) dtout (double) : The data-output interval.\n");
   printf("    Default is 0.25.\n");
   printf("\n");
   printf("12) NPROC (int) : The number of processors.\n");
   printf("    Default is 1.\n");
}
