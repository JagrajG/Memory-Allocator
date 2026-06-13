#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>

#define HEAP_SIZE 4096

struct Header
{
    size_t size;
    uint8_t is_free;
    struct Header *next;
};

void *my_malloc(size_t size);
void my_free(void *ptr);
void heap_dump(void);

struct Header *find_free_block(size_t size);
void split_block(struct Header *block, size_t requested_size);
void coalesce_free_blocks(void);

extern struct Header *head;
extern uint8_t heap[HEAP_SIZE];
extern size_t heap_used;

#endif