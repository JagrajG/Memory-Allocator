# Custom Memory Allocator

A custom memory allocator written in C to learn low-level memory management, pointer arithmetic, block metadata, linked lists, free block reuse, block splitting, and coalescing.

This project implements a small `malloc` / `free` style allocator using a fixed-size memory pool instead of requesting memory directly from the operating system.

## Features

- Fixed-size 4096-byte heap
- Custom `my_malloc`
- Custom `my_free`
- Block headers for metadata
- Linked list of memory blocks
- First-fit reuse of freed blocks
- Block splitting
- Coalescing adjacent free blocks
- Heap dump debugging
- Tests for allocation, reuse, splitting, coalescing, and edge cases

## Project Structure

```text
custom-memory-allocator/
  allocator.h
  allocator.c
  tests.c
  README.md
```

## File Overview

```text
allocator.h   -> struct definition, function declarations, shared globals
allocator.c   -> allocator implementation
tests.c       -> allocator tests
README.md     -> project documentation
```

## Overview

The allocator manages memory inside a static 4096-byte array:

```c
uint8_t heap[4096];
```

This array acts as a small heap controlled by the allocator.

Instead of asking the operating system for memory, `my_malloc` gives out pieces of this fixed array.

Each allocation is represented as a block:

```text
[ Header ][ User Memory ]
           ↑
           pointer returned by my_malloc
```

The user receives a pointer to the usable memory area, not the header.

The header is hidden metadata used by the allocator to track each block.

## Block Header

Each block starts with this header:

```c
struct Header
{
    size_t size;
    uint8_t is_free;
    struct Header *next;
};
```

The fields are:

```text
size    -> size of the user memory area
is_free -> 0 if the block is used, 1 if the block is free
next    -> pointer to the next block in the linked list
```

The allocator stores the header before the user memory so it can later recover information about the block during `my_free`.

## Memory Layout

Before any allocation:

```text
[ 4096 bytes free ]
```

After:

```c
my_malloc(100);
```

The heap looks like:

```text
[ Header ][ 100 bytes user memory ][ remaining free space ]
```

After:

```c
my_malloc(50);
```

The heap looks like:

```text
[ Header ][ 100 bytes ][ Header ][ 50 bytes ][ remaining free space ]
```

Each block has its own header.

The returned pointer points after the header:

```text
[ Header ][ User Memory ]
  ↑        ↑
  header   returned pointer
```

## Allocation

`my_malloc(size)` works in two main ways.

First, it searches the linked list for a free block large enough to reuse.

If a suitable free block exists, the allocator reuses it.

If the block is larger than needed, the allocator splits it into:

```text
[ used block ][ new free block ]
```

If no reusable block is found, the allocator creates a new block at the next unused position in the heap.

The allocator uses a first-fit strategy, meaning it reuses the first free block that is large enough.

## Freeing

`my_free(ptr)` marks a block as free.

The pointer passed to `my_free` points to user memory, not the header:

```text
[ Header ][ User Memory ]
           ↑
           ptr
```

To find the header, the allocator moves backward by one header:

```c
struct Header *header = ((struct Header *)ptr) - 1;
```

Then the block is marked free:

```c
header->is_free = 1;
```

After freeing, the allocator attempts to coalesce adjacent free blocks.

## Block Splitting

Block splitting reduces wasted space when a free block is larger than the requested allocation.

Example:

```c
void *a = my_malloc(100);
my_free(a);
void *b = my_malloc(40);
```

Without splitting, the full 100-byte block would be reused for only 40 bytes.

With splitting, the block becomes:

```text
[ Header A ][ 40 bytes used ][ Header B ][ 36 bytes free ]
```

The leftover size is:

```text
100 - 40 - sizeof(Header) = 36
```

With a 24-byte header:

```text
100 - 40 - 24 = 36
```

So the original 100-byte free block becomes:

```text
[ used block: 40 bytes ][ free block: 36 bytes ]
```

## Coalescing

Coalescing merges adjacent free blocks to reduce fragmentation.

Example:

```text
[ free 40 ][ free 36 ][ used 50 ]
```

The first two blocks can be merged into:

```text
[ free 100 ][ used 50 ]
```

The merged size includes the second block’s header:

```text
40 + sizeof(Header) + 36 = 100
```

With a 24-byte header:

```text
40 + 24 + 36 = 100
```

Coalescing is useful because it allows the allocator to reuse larger blocks later instead of leaving many small fragmented blocks.

## Important Implementation Ideas

### Header placement

A new block header is placed at the next unused position in the heap:

```c
struct Header *header = (struct Header *)(heap + heap_used);
```

This means:

```text
Go heap_used bytes forward from the start of heap.
Treat that location as the start of a Header.
```

### Returned user pointer

The allocator returns the memory after the header:

```c
return (void *)(header + 1);
```

This points to the usable memory area, not the metadata.

### Recovering the header in my_free

Since `my_malloc` returns `header + 1`, `my_free` reverses that:

```c
struct Header *header = ((struct Header *)ptr) - 1;
```

This moves from the user pointer back to the hidden header.

### Splitting address calculation

The new leftover header starts after the current header and the requested user memory:

```c
uint8_t *new_header_address = (uint8_t *)(block + 1) + requested_size;
struct Header *new_block = (struct Header *)new_header_address;
```

This means:

```text
Go past the old header.
Move forward by requested_size bytes.
Place the new leftover header there.
```

## Build and Run

Compile normally:

```bash
clang -Wall -Wextra -Wpedantic -g allocator.c tests.c -o allocator_tests
```

Run tests:

```bash
./allocator_tests
```

Compile with AddressSanitizer:

```bash
clang -Wall -Wextra -Wpedantic -g -fsanitize=address allocator.c tests.c -o allocator_tests
```

Run with AddressSanitizer:

```bash
./allocator_tests
```

## Example Test Output

```text
Header size: 24

After allocating a=100 and b=50:
Block: 0 | size: 100 | is_free: 0 | next: 0x...
Block: 1 | size: 50 | is_free: 0 | next: 0x0
heap_used: 198 / 4096

After freeing a:
Block: 0 | size: 100 | is_free: 1 | next: 0x...
Block: 1 | size: 50 | is_free: 0 | next: 0x0
heap_used: 198 / 4096

After allocating c=40 into freed a block:
Block: 0 | size: 40 | is_free: 0 | next: 0x...
Block: 1 | size: 36 | is_free: 1 | next: 0x...
Block: 2 | size: 50 | is_free: 0 | next: 0x0
heap_used: 198 / 4096

After freeing c:
Block: 0 | size: 100 | is_free: 1 | next: 0x...
Block: 1 | size: 50 | is_free: 0 | next: 0x0
heap_used: 198 / 4096

After freeing b:
Block: 0 | size: 174 | is_free: 1 | next: 0x0
heap_used: 198 / 4096
```

## Tests Covered

The test file checks:

- `my_malloc(100)` returns non-NULL
- `my_malloc(50)` returns non-NULL
- `heap_used` updates correctly
- `my_free` marks a block free
- freed blocks are reused
- large free blocks are split
- leftover blocks are created correctly
- adjacent free blocks are coalesced
- `my_malloc(0)` returns NULL
- oversized allocations return NULL
- `my_free(NULL)` does not crash

## Current Limitations

This allocator is for learning and is not a full replacement for real `malloc`.

Current limitations:

- Fixed heap size
- No thread safety
- No invalid pointer detection
- No double-free protection
- No full alignment handling yet
- Does not request memory from the operating system
- Not intended for production use

## What I Learned

- How memory allocators store hidden metadata
- How headers are placed before user memory
- How pointer arithmetic works with raw byte arrays
- How to manage blocks using a linked list
- How freed memory can be reused
- How first-fit allocation works
- How block splitting reduces wasted space
- How coalescing reduces fragmentation
- How to inspect allocator state with heap dumps
- How to write basic tests for low-level C code

## Summary

This project helped build a stronger understanding of low-level C programming and memory management.

The allocator supports allocation, freeing, free-block reuse, splitting, coalescing, heap inspection, and basic testing inside a fixed 4096-byte memory pool.
