# Custom C Memory Allocator

A lightweight, educational implementation of a dynamic memory allocator in C. This project replaces the standard library functions (`malloc`, `free`, `realloc`, and `calloc`) with a custom implementation using a **doubly-linked list** and the `sbrk` system call.

---

## 🚀 Features

- **First-Fit Allocation Strategy**  
  Scans the free list for the first block that can satisfy the requested size.

- **Block Splitting**  
  Reduces internal fragmentation by splitting large free blocks into smaller ones when only a portion of the memory is needed.

- **Coalescing**  
  Merges adjacent free blocks during the `free` process to prevent external fragmentation.

- **Memory Alignment**  
  Ensures all allocated pointers are aligned to the system word size (e.g., 8 bytes on 64-bit systems).

- **Debug Utilities**  
  Includes a `heap_dump` function to visualize the current state of the heap, metadata, and block status.

---

## 🏗️ Architecture

The allocator manages memory by prepending a metadata header to every allocated block.

### The Block Header

Each block in the heap is represented by the following structure:

```c
typedef struct block_t {
    size_t          size;   /* Usable payload size */
    bool            free;   /* Status flag */
    struct block_t *next;   /* Pointer to next block */
    struct block_t *prev;   /* Pointer to previous block */
} block_t;
```

When you call `my_malloc(size)`, the allocator:

1. **Aligns** the requested size to the nearest word boundary.
2. **Searches** the linked list for a free block.
3. **Splits** the block if it's significantly larger than requested.
4. **Extends** the heap via `sbrk` if no suitable block is found.
5. **Returns** a pointer to the payload (the memory immediately following the header).

---

## 🛠️ Usage

### API Reference

| Function | Description |
|----------|-------------|
| `void *my_malloc(size_t size)` | Allocates `size` bytes of uninitialized memory. |
| `void my_free(void *ptr)` | Deallocates the memory block pointed to by `ptr`. |
| `void *my_realloc(void *ptr, size_t size)` | Resizes the memory block, moving it if necessary. |
| `void *my_calloc(size_t nmemb, size_t size)` | Allocates memory for an array and zeroes it out. |
| `void heap_dump()` | Prints a visual map of all blocks currently in the heap. |

### Example

```c
#include "main.c"

int main() {
    int *arr = my_malloc(sizeof(int) * 5);
    
    // Use the memory...
    arr[0] = 42;

    heap_dump(); // See what the heap looks like!

    my_free(arr);
    return 0;
}
```

---

## 🚦 Getting Started

### Prerequisites

- A Linux/Unix-based environment (required for `sbrk` and `unistd.h`).
- GCC or any standard C compiler.

### Compilation & Running

To run the built-in test suite:

```bash
gcc -o allocator main.c
./allocator
```

---

## 📝 Roadmap & Improvements

This implementation is designed for educational purposes. For a "production-ready" version, the following could be added:

- [ ] **Thread Safety**  
  Add mutexes to allow usage in multi-threaded programs.

- [ ] **Best-Fit / Next-Fit**  
  Implement alternative search strategies to compare performance.

- [ ] **Segregated Free Lists**  
  Speed up allocation by grouping free blocks by size.

---