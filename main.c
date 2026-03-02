#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* ─── Block Header ─────────────────────────────────────────────────────────── */

typedef struct block_t {
    size_t          size;   /* usable payload size (bytes)   */
    bool            free;   /* is this block free?           */
    struct block_t *next;   /* next block in the list        */
    struct block_t *prev;   /* previous block in the list    */
} block_t;

#define BLOCK_SIZE      sizeof(block_t)
#define ALIGN(size)     (((size) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1))
#define MIN_SPLIT_SIZE  (BLOCK_SIZE + 8)   /* min leftover to bother splitting */

static block_t *heap_start = NULL;   /* head of the block list */

/* ─── Internal Helpers ─────────────────────────────────────────────────────── */

/* Extend the heap by requesting memory from the OS via sbrk() */
static block_t *extend_heap(size_t size) {
    block_t *blk = (block_t *)sbrk(0);   /* current program break */

    if (sbrk(BLOCK_SIZE + size) == (void *)-1)
        return NULL;   /* OS refused */

    blk->size = size;
    blk->free = false;
    blk->next = NULL;
    blk->prev = NULL;
    return blk;
}

/* Split a large block if the leftover is big enough to be useful */
static void split_block(block_t *blk, size_t size) {
    if (blk->size < size + MIN_SPLIT_SIZE)
        return;   /* not worth splitting */

    block_t *new_blk = (block_t *)((char *)blk + BLOCK_SIZE + size);
    new_blk->size    = blk->size - size - BLOCK_SIZE;
    new_blk->free    = true;
    new_blk->next    = blk->next;
    new_blk->prev    = blk;

    if (blk->next)
        blk->next->prev = new_blk;

    blk->next = new_blk;
    blk->size = size;
}

/* Coalesce adjacent free blocks to reduce fragmentation */
static void coalesce(block_t *blk) {
    /* merge with next block if it's free */
    if (blk->next && blk->next->free) {
        blk->size += BLOCK_SIZE + blk->next->size;
        blk->next  = blk->next->next;
        if (blk->next)
            blk->next->prev = blk;
    }

    /* merge with previous block if it's free */
    if (blk->prev && blk->prev->free) {
        blk->prev->size += BLOCK_SIZE + blk->size;
        blk->prev->next  = blk->next;
        if (blk->next)
            blk->next->prev = blk->prev;
    }
}

/* First-fit search through the free list */
static block_t *find_free_block(size_t size) {
    block_t *cur = heap_start;
    while (cur) {
        if (cur->free && cur->size >= size)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

/* Get block header from a user pointer */
static inline block_t *get_block(void *ptr) {
    return (block_t *)((char *)ptr - BLOCK_SIZE);
}

/* ─── Public API ───────────────────────────────────────────────────────────── */

void *my_malloc(size_t size) {
    if (size == 0) return NULL;

    size = ALIGN(size);

    block_t *blk = find_free_block(size);

    if (blk) {
        /* Reuse an existing free block */
        split_block(blk, size);
        blk->free = false;
    } else {
        /* No suitable block — ask the OS for more memory */
        blk = extend_heap(size);
        if (!blk) return NULL;

        /* Append to the end of the list */
        if (!heap_start) {
            heap_start = blk;
        } else {
            block_t *cur = heap_start;
            while (cur->next) cur = cur->next;
            cur->next = blk;
            blk->prev = cur;
        }
    }

    return (char *)blk + BLOCK_SIZE;   /* return pointer past the header */
}

void my_free(void *ptr) {
    if (!ptr) return;

    block_t *blk = get_block(ptr);
    blk->free = true;
    coalesce(blk);
}

void *my_realloc(void *ptr, size_t size) {
    if (!ptr)        return my_malloc(size);
    if (size == 0) { my_free(ptr); return NULL; }

    size = ALIGN(size);
    block_t *blk = get_block(ptr);

    if (blk->size >= size) {
        /* Current block is already big enough */
        split_block(blk, size);
        return ptr;
    }

    /* Allocate a new, larger block and copy data over */
    void *new_ptr = my_malloc(size);
    if (!new_ptr) return NULL;

    memcpy(new_ptr, ptr, blk->size);
    my_free(ptr);
    return new_ptr;
}

void *my_calloc(size_t nmemb, size_t size) {
    size_t total = nmemb * size;
    void  *ptr   = my_malloc(total);
    if (ptr) memset(ptr, 0, total);
    return ptr;
}

/* ─── Debug Utility ────────────────────────────────────────────────────────── */

void heap_dump(void) {
    printf("\n========== HEAP DUMP ==========\n");
    block_t *cur = heap_start;
    int      idx = 0;
    while (cur) {
        printf("Block %d | addr: %p | size: %4zu | %s\n",
               idx++,
               (void *)cur,
               cur->size,
               cur->free ? "FREE" : "USED");
        cur = cur->next;
    }
    printf("================================\n\n");
}

/* ─── Test Suite ───────────────────────────────────────────────────────────── */

int main(void) {
    printf("=== Custom Allocator Test ===\n\n");

    /* Basic allocation */
    printf("[1] Basic malloc / free\n");
    int *a = my_malloc(sizeof(int) * 4);
    a[0] = 10; a[1] = 20; a[2] = 30; a[3] = 40;
    printf("    a = [%d, %d, %d, %d]\n", a[0], a[1], a[2], a[3]);
    heap_dump();

    /* Multiple allocations */
    printf("[2] Multiple allocations\n");
    char *s1 = my_malloc(32);
    char *s2 = my_malloc(64);
    strcpy(s1, "hello");
    strcpy(s2, "world");
    printf("    s1 = \"%s\", s2 = \"%s\"\n", s1, s2);
    heap_dump();

    /* Free and reuse */
    printf("[3] Free s1, allocate s3 (should reuse s1's block)\n");
    my_free(s1);
    char *s3 = my_malloc(16);
    strcpy(s3, "reused!");
    printf("    s3 = \"%s\"\n", s3);
    heap_dump();

    /* calloc */
    printf("[4] calloc (should be zero-initialised)\n");
    int *c = my_calloc(5, sizeof(int));
    printf("    c = [%d, %d, %d, %d, %d]\n", c[0], c[1], c[2], c[3], c[4]);
    heap_dump();

    /* realloc */
    printf("[5] realloc a from 4 ints to 8 ints\n");
    a = my_realloc(a, sizeof(int) * 8);
    a[4] = 50; a[5] = 60; a[6] = 70; a[7] = 80;
    printf("    a = [%d, %d, %d, %d, %d, %d, %d, %d]\n",
           a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
    heap_dump();

    /* Free everything and check coalescing */
    printf("[6] Free everything — heap should coalesce\n");
    my_free(a);
    my_free(s2);
    my_free(s3);
    my_free(c);
    heap_dump();

    printf("All tests done.\n");
    return 0;
}
