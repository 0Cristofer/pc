#ifndef SHAREDMEMORYCONTROLLER_H
#define SHAREDMEMORYCONTROLLER_H

#include <sys/shm.h>
#include <sys/types.h>

void createShMem(int size);

void* shAlloc(int size);

void shFree(void* ptr);

#endif