# Custom Memory Allocator in C

A custom memory allocator built from scratch in C, implementing `malloc`, `free`, `calloc`, and `realloc` without relying on the standard library. Built in two phases — a static array-based allocator and a syscall-based allocator using `sbrk()`.

---

## Phases

### P1 — Static Array
A fixed 1MB heap backed by a static char array. Focuses on core allocator logic.

### P2 — sbrk() System Call
Heap memory requested directly from the OS at runtime using `sbrk()`.

---

## Features

- `myMalloc(size)` — allocates a block with 8-byte alignment and block splitting
- `myFree(ptr)` — frees a block with forward coalescing
- `myCalloc(num, size)` — allocates and zero-initializes memory
- `myRealloc(ptr, size)` — resizes an existing allocation
- `printHeap()` — prints all block states for debugging

---

## Memory Layout

Each allocation is preceded by a block header storing size and free status:

```
[ header | data ] [ header | data ] ... [ sentinel ]
```

Splitting on allocation and coalescing on free keep fragmentation in check.

---

## Build & Run

```bash
gcc main.c allocator.c -o main
./main
```

---

## Edge Cases Handled

- `myMalloc(0)` returns NULL
- `myFree(NULL)` is a no-op
- Allocation larger than heap returns NULL
