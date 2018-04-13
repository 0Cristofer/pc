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

#include <stdio.h>
#include "stdinc.h"
#include <errno.h>

#define HZ 60.0
#define MULT 1103515245
#define ADD 12345
#define MASK (0x7FFFFFFF)
#define TWOTO31 2147483648.0

local int A = 1;
local int B = 0;
local int randx = 1;
local int lastrand;   /* the last random number */

/*
 * XRAND: generate floating-point random number.
 */

double prand();

double xrand(double xl, double xh){
   long random ();
   double x;

   return (xl + (xh - xl) * prand());
}

void pranset(int seed){
   int proc;

   A = 1;
   B = 0;
   randx = (A*seed+B) & MASK;
   A = (MULT * A) & MASK;
   B = (MULT*B + ADD) & MASK;
}

/*
Return a random double in [0, 1.0)
*/
double prand(){
   lastrand = randx;
   randx = (A*randx+B) & MASK;
   return((double)lastrand/TWOTO31);
}

/*
 * CPUTIME: compute CPU time in min.
 */

#include <sys/types.h>
#include <sys/times.h>

double cputime(){
   struct tms buffer;

   if (times(&buffer) == -1)
      error("times() call failed\n");

   return (buffer.tms_utime / (60.0 * HZ));
}

/*
 * ERROR: scream and die quickly.
 */

error(char msg, char a1, char a2, char a3, char a4) {
   //extern int errno;

   fprintf(stderr, msg, a1, a2, a3, a4);
   if (errno != 0)
      perror("Error");
   exit(0);
}
