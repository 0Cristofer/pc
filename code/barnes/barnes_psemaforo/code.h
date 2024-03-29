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
 * CODE.H: define various global things for CODE.C.
 */

#ifndef _CODE_H_
#define _CODE_H_

#include "defs.h".

#define PAD_SIZE (PAGE_SIZE / (sizeof(int)))

global int mem_ctrl_id;
global unsigned int ProcessId;

struct shMemCtrl{
  int gldefsid;
  int glid;
  int localid;
  int btabid;
  int ctabid;
  int ltabid;
  int mbodyid;
  int mcellid;
  int mleafid;
  int cellid;
};
global struct shMemCtrl* ctrl;

/* Defined by the input file */
struct GlobalDefs{
  string headline; 	/* message describing calculation */
  string infile; 		/* file name for snapshot input */
  string outfile; 		/* file name for snapshot output */
  real dtime; 		/* timestep for leapfrog integrator */
  real dtout; 		/* time between data outputs */
  real tstop; 		/* time to stop calculation */
  int nbody; 		/* number of bodies in system */
  real fcells; 		/* ratio of cells/leaves allocated */
  real fleaves; 		/* ratio of leaves/bodies allocated */
  real tol; 		/* accuracy parameter: 0.0 => exact */
  real tolsq; 		/* square of previous */
  real eps; 		/* potential softening parameter */
  real epssq; 		/* square of previous */
  real dthf; 		/* half time step */
  int NPROC; 		/* Number of Processors */

  int maxcell;		/* max number of cells allocated */
  int maxleaf;		/* max number of leaves allocated */
  int maxmybody;		/* max no. of bodies allocated per processor */
  int maxmycell;		/* max num. of cells to be allocated */
  int maxmyleaf;		/* max num. of leaves to be allocated */
  bodyptr bodytab; 	/* array size is exactly nbody bodies */

  struct CellSemType {
      sem_t CL[MAXLOCK];        /* locks on the cells*/
  } *CellSem;
};
global struct GlobalDefs* globalDefs;


struct GlobalMemory  {	/* all this info is for the whole system */
    int n2bcalc;       /* total number of body/cell interactions  */
    int nbccalc;       /* total number of body/body interactions  */
    int selfint;       /* number of self interactions             */
    real mtot;         /* total mass of N-body system             */
    real etot[3];      /* binding, kinetic, potential energy      */
    matrix keten;      /* kinetic energy tensor                   */
    matrix peten;      /* potential energy tensor                 */
    vector cmphase[2]; /* center of mass coordinates and velocity */
    vector amvec;      /* angular momentum vector                 */
    cellptr G_root;    /* root of the whole tree                  */
    vector rmin;       /* lower-left corner of coordinate box     */
    vector min;        /* temporary lower-left corner of the box  */
    vector max;        /* temporary upper right corner of the box */
    real rsize;        /* side-length of integer coordinate box   */

struct {
	unsigned long	counter;
	unsigned long	cycle;
  sem_t sem_count;
  sem_t sem_bar;
} (Barstart);
   /* barrier at the beginning of stepsystem  */

struct {
	unsigned long	counter;
	unsigned long	cycle;
  sem_t sem_count;
  sem_t sem_bar;
} (Bartree);
    /* barrier after loading the tree          */

struct {
	unsigned long	counter;
	unsigned long	cycle;
  sem_t sem_count;
  sem_t sem_bar;
} (Barcom);
     /* barrier after computing the c. of m.    */

struct {
	unsigned long	counter;
	unsigned long	cycle;
  sem_t sem_count;
  sem_t sem_bar;
} (Barload);


struct {
	unsigned long	counter;
	unsigned long	cycle;
  sem_t sem_count;
  sem_t sem_bar;
} (Baraccel);
   /* barrier after accel and before output   */

struct {
	unsigned long	counter;
	unsigned long	cycle;
  sem_t sem_count;
  sem_t sem_bar;
} (Barpos);
    /* barrier after computing the new pos     */
    sem_t CountSem; /* Lock on the shared variables            */
    sem_t NcellSem; /* Lock on the counter of array of cells for loadtree */
    sem_t NleafSem;/* Lock on the counter of array of leaves for loadtree */
    sem_t io_sem;
    unsigned long createstart,createend,computestart,computeend;
    unsigned long trackstart, trackend, tracktime;
    unsigned long partitionstart, partitionend, partitiontime;
    unsigned long treebuildstart, treebuildend, treebuildtime;
    unsigned long forcecalcstart, forcecalcend, forcecalctime;
    unsigned int current_id;
    volatile int k; /*for memory allocation in code.C */
};
global struct GlobalMemory *Global;

/* This structure is needed because under the sproc model there is no
 * per processor private address space.
 */
struct local_memory {
   /* Use padding so that each processor's variables are on their own page */
   int pad_begin[PAD_SIZE];

   real tnow;        	/* current value of simulation time */
   real tout;         	/* time next output is due */
   int nstep;      	/* number of integration steps so far */

   int workMin, workMax;/* interval of cost to be treated by a proc */

   vector min, max; 	/* min and max of coordinates for each Proc. */

   int mynumcell; 	/* num. of cells used for this proc in ctab */
   int mynumleaf; 	/* num. of leaves used for this proc in ctab */
   int mynbody;   	/* num bodies allocated to the processor */
   bodyptr* mybodytab;	/* array of bodies allocated / processor */
   int myncell; 	/* num cells allocated to the processor */
   cellptr* mycelltab;	/* array of cellptrs allocated to the processor */
   int mynleaf; 	/* number of leaves allocated to the processor */
   leafptr* myleaftab; 	/* array of leafptrs allocated to the processor */
   cellptr ctab;	/* array of cells used for the tree. */
   leafptr ltab;	/* array of cells used for the tree. */

   int myn2bcalc; 	/* body-body force calculations for each processor */
   int mynbccalc; 	/* body-cell force calculations for each processor */
   int myselfint; 	/* count self-interactions for each processor */
   int myn2bterm; 	/* count body-body terms for a body */
   int mynbcterm; 	/* count body-cell terms for a body */
   bool skipself; 	/* true if self-interaction skipped OK */
   bodyptr pskip;       /* body to skip in force evaluation */
   vector pos0;         /* point at which to evaluate field */
   real phi0;           /* computed potential at pos0 */
   vector acc0;         /* computed acceleration at pos0 */
   vector dr;  		/* data to be shared */
   real drsq;      	/* between gravsub and subdivp */
   nodeptr pmem;	/* remember particle data */

   nodeptr Current_Root;
   int Root_Coords[NDIM];

   real mymtot;      	/* total mass of N-body system */
   real myetot[3];   	/* binding, kinetic, potential energy */
   matrix myketen;   	/* kinetic energy tensor */
   matrix mypeten;   	/* potential energy tensor */
   vector mycmphase[2];	/* center of mass coordinates */
   vector myamvec;   	/* angular momentum vector */

   int pad_end[PAD_SIZE];
};
global struct local_memory* Local;

#endif
