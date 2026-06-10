#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#define HEAP_SIZE 4096

uint8_t heap[HEAP_SIZE];

struct Header
{
    // size of user memory, not including header
    size_t size;

    // 0 = used, 1 = free
    uint8_t is_free;

    // pointer to next block
    struct Header *next;
};
