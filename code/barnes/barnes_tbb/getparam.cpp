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
 * GETPARAM.C:
 */

#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
extern pthread_t PThreadTable[];


#include "stdinc.h"

local string *defaults = NULL;        /* vector of "name=value" strings */

/*
 * INITPARAM: ignore arg vector, remember defaults.
 */

void initparam(string *argv, string *defv){
  defaults = defv;
}

/*
 * GETPARAM: export version prompts user for value.
 */

string getparam(string name){
  int scanbind(), i, strlen(), leng;
  string extrvalue(), def;
  char buf[128], *strcpy();
  char* temp;

  if (defaults == NULL)
  error1("getparam: called before initparam\n");
  i = scanbind(defaults, name);
  if (i < 0)
  error2("getparam: %s unknown\n", name);
  def = extrvalue(defaults[i]);
  gets(buf);
  leng = strlen(buf) + 1;
  if (leng > 1) {
    return (strcpy(malloc(leng), buf));
  }
  else {
    return (def);
  }
}

/*
 * GETIPARAM, ..., GETDPARAM: get int, long, bool, or double parameters.
 */

int getiparam(string name){
  string getparam(), val;
  int atoi();

  for (val = ""; *val == NULL;) {
    val = getparam(name);
  }

  return (atoi(val));
}

long getlparam(string name){

  string getparam(), val;
  long atol();

  for (val = ""; *val == NULL;)
    val = getparam(name);

  return (atol(val));
}

bool getbparam(string name){
  string getparam(), val;

  for (val = ""; *val == NULL; )
  val = getparam(name);
  if (strchr("tTyY1", *val) != NULL) {
    return (TRUE);
  }
  if (strchr("fFnN0", *val) != NULL) {
    return (FALSE);
  }
  error3("getbparam: %s=%s not bool\n", name, val);
}

double getdparam(string name){
  string getparam(), val;
  double atof();

  for (val = ""; *val == NULL; ) {
    val = getparam(name);
  }
  return (atof(val));
}

/*
 * SCANBIND: scan binding vector for name, return index.
 */
 int scanbind(string bvec[], string name){
   int i;
   bool matchname();

   for (i = 0; bvec[i] != NULL; i++)
      if (matchname(bvec[i], name))
	     return (i);

   return (-1);
}

/*
 * MATCHNAME: determine if "name=value" matches "name".
 */

bool matchname(string bind, string name){
   char *bp, *np;

   bp = bind;
   np = name;
   while (*bp == *np) {
     bp++;
     np++;
   }
   return (*bp == '=' && *np == NULL);
}

/*
 * EXTRVALUE: extract value from name=value string.
 */
string extrvalue(string arg){
   char *ap;
   ap = (char *) arg;

   while (*ap != NULL)
      if (*ap++ == '=')
	     return ((string) ap);

   return (NULL);
}
