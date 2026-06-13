#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#define HEAP_SIZE 4096

struct Header
{
    // size of user memory, not including header
    size_t size;

    // 0 = used, 1 = free
    uint8_t is_free;

    // pointer to next block
    struct Header *next;
};

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

    heap_used += header->size + sizeof(struct Header);

    return (void *)(header + 1);
}

void split_block(struct Header *block, size_t requested_size)
{

    if (block->size < requested_size + sizeof(struct Header) + 1)
    {
        return;
    }

    size_t leftover = block->size - requested_size - sizeof(struct Header);

    struct Header *block_size = block->size;
    struct Header *next_block = block->next;
}
void my_free(void *ptr)
{
    if (ptr == NULL)
    {
        return;
    }

    struct Header *header = ((struct Header *)ptr) - 1;
    header->is_free = 1;
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

int main(void)
{
    void *a = my_malloc(100);
    void *b = my_malloc(50);

    printf("After two allocations:\n");
    heap_dump();

    my_free(a);

    printf("\nAfter freeing a:\n");
    heap_dump();

    void *c = my_malloc(80);

    printf("\nAfter allocating 80 bytes:\n");
    heap_dump();

    printf("\na = %p\n", a);
    printf("b = %p\n", b);
    printf("c = %p\n", c);

    if (a == c)
    {
        printf("PASS: c reused a's block\n");
    }
    else
    {
        printf("FAIL: c did not reuse a's block\n");
    }

    return 0;
}