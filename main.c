#include <stdio.h>
#include "allocator.h"

int main (void) {

    heapInit();

    printHeap();

    int *arr = (int *)myMalloc(3 * sizeof(int));
    printHeap();
    
    char *brr = (char *)myMalloc(390 * sizeof(char));
    printHeap();
    
    myFree(arr);
    printHeap();
    
    myFree(brr);
    printHeap();
    
    return 0;
}