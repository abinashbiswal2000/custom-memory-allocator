#include <stdlib.h>
#include <stdio.h>



static char heap[1024 * 1024] = {0}; // 1MB
// Static word is used to make sure no other file can access the heap variable using the extern keyword.



typedef struct BlockHeader {
    size_t size;    // size of entire block (header + data)
    int free;       // 1 = free, 0 = used
} BlockHeader;



void heapInit() {
    BlockHeader *h1 = (BlockHeader *)heap;
    h1->free = 1;
    h1->size = (1024 * 1024) - (2 * sizeof(BlockHeader));
    // now set h1->size and h1->free
    
    BlockHeader *h2 = (BlockHeader *)((char *)h1 + sizeof(BlockHeader) + h1->size);
    h2->free = 0;
    h2->size = 0;
    // now set h2->size and h2->free
}


void *myMalloc (size_t size) {

    if (size == 0) {
        return NULL;
    }
    
    // Alignment
    size = (size + 7) & ~7;
    // if (size % 8 != 0) {
    //     size = size + 8 - (size%8);
    // }
    
    BlockHeader * h = (BlockHeader *)heap;

    while (1) {
        if (h->free == 0 && h->size == 0) {
            return NULL;
        } else if (h->free == 1 && h->size >= size) {
            
            // Block Split
            if (h->size > size + sizeof(BlockHeader)) {

                BlockHeader* hNext = (BlockHeader *)((char *)h + sizeof(BlockHeader) + size);
                hNext->free = 1;
                hNext->size = h->size - size - sizeof(BlockHeader);

                h->size = size;
            }

            h->free = 0;
            return (void *)(h + 1);
        } else {
            h = (BlockHeader *)((char *)h + sizeof(BlockHeader) + h->size);
        }
    }

};



void myFree (void *ptr) {
    
    if (ptr == NULL) {
        return;
    }

    BlockHeader *h = (BlockHeader *)ptr;
    h -= 1;
    h->free = 1;
    
    BlockHeader *hNext = (BlockHeader *)((char *)h + sizeof(BlockHeader) + h->size);

    // Coalescing
    while (1) {
        if (hNext->free == 0) {
            return;
        } else {
            h->size = h->size + (sizeof(BlockHeader) + hNext->size);
            hNext = (BlockHeader *)((char *)hNext + sizeof(BlockHeader) + hNext->size);
        }
    }
};



void printHeap () {
    BlockHeader *h = (BlockHeader *)heap;
    printf("--------------\n");
    while (h->size != 0) {
        printf("h->free = %d, h->size = %ld\n", h->free, h->size);
        h = (BlockHeader *)((char *)h + sizeof(BlockHeader) + h->size);
    }
    printf("--------------\n");
}