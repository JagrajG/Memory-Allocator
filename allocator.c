#include <stdio.h>
#include "allocator.h"

struct Header *head = NULL;

uint8_t heap[HEAP_SIZE];

// keep track of how many of the HEAP_Size is used so we do not exceed it
size_t heap_used = 0;

struct Header *find_free_block(size_t size)
{
    struct Header *current = head;

    if (size == 0)
    {
        return NULL;
    }

    while (current != NULL)
    {
        if (current->is_free == 1 && current->size >= size)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void *my_malloc(size_t size)
{
    if (size == 0)
    {
        return NULL;
    }

    struct Header *free_block = find_free_block(size);
    if (free_block != NULL)
    {
        split_block(free_block, size);

        free_block->is_free = 0;

        return (void *)(free_block + 1);
    }

    size_t total_size = sizeof(struct Header) + size;

    if (total_size + heap_used > HEAP_SIZE)
    {
        return NULL;
    }

    // create the meta data
    // create a pointer to a Header called header
    // header takes the shape of Header
    // heap + heap_used is used to find the next open spot in memory
    struct Header *header = (struct Header *)(heap + heap_used);
    header->size = size;
    header->is_free = 0;
    header->next = NULL;

    // make head point to the head of the LL
    if (head == NULL)
    {
        head = header;
    }
    else
    {
        struct Header *current = head;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = header;
    }

    heap_used += total_size;

    return (void *)(header + 1);
}

void coalesce_free_blocks(void)
{
    struct Header *current = head;

    if (current == NULL)
    {
        return;
    }
    while (current != NULL && current->next != NULL)
    {
        if (current->is_free == 1 && current->next->is_free == 1)
        {
            struct Header *next_block = current->next;
            current->size = current->size + sizeof(struct Header) + next_block->size;
            current->next = next_block->next;
        }
        else
        {
            current = current->next;
        }
    }
}

void split_block(struct Header *block, size_t requested_size)
{

    if (block->size < requested_size + sizeof(struct Header) + 1)
    {
        return;
    }

    size_t leftover = block->size - requested_size - sizeof(struct Header);

    struct Header *old_next = block->next;

    uint8_t *new_header_address = (uint8_t *)(block + 1) + requested_size;
    struct Header *new_block = (struct Header *)new_header_address;

    new_block->size = leftover;
    new_block->is_free = 1;
    new_block->next = old_next;

    block->size = requested_size;
    block->next = new_block;
}

void my_free(void *ptr)
{
    if (ptr == NULL)
    {
        return;
    }

    struct Header *header = ((struct Header *)ptr) - 1;
    header->is_free = 1;

    coalesce_free_blocks();
}

void heap_dump(void)
{
    struct Header *header = head;
    int block = 0;

    while (header != NULL)
    {
        printf("Block: %d | size: %zu | is_free: %u | next: %p\n",
               block,
               header->size,
               header->is_free,
               (void *)header->next);

        header = header->next;
        block++;
    }

    printf("heap_used: %zu / %d\n", heap_used, HEAP_SIZE);
}
