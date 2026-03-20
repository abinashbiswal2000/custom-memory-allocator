#ifndef ALLOCATOR_H
#define ALLOCATOR_H


#include <stdlib.h>


void heapInit();
void *myMalloc (size_t size);
void myFree (void *ptr);
void printHeap ();
// void *myCalloc (size_t num, size_t size);
// void *myRealloc (void *ptr, size_t size);


#endif