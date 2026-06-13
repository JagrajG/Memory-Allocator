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

// function prototypes
void split_block(struct Header *block, size_t requested_size);

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

    struct Header *old_block = block->next;

    uint8_t *new_header_address = (uint8_t *)(block + 1) + requested_size;
    struct Header *new_block = (struct Header *)new_header_address;

    new_block->size = leftover;
    new_block->is_free = 1;
    new_block->next = old_block;

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

void check(int condition, const char *message)
{
    if (condition)
    {
        printf("PASS: %s\n", message);
    }
    else
    {
        printf("FAIL: %s\n", message);
    }
}
int main(void)
{
    printf("Header size: %zu\n\n", sizeof(struct Header));

    void *a = my_malloc(100);
    void *b = my_malloc(50);

    printf("After two allocations:\n");
    heap_dump();

    check(a != NULL, "my_malloc(100) returns non-NULL");
    check(b != NULL, "my_malloc(50) returns non-NULL");
    check(heap_used == 198, "heap_used is 198 after 100 and 50 byte allocations");

    my_free(a);

    printf("\nAfter freeing a:\n");
    heap_dump();

    check(head->is_free == 1, "first block is marked free after my_free(a)");
    check(head->next->is_free == 0, "second block is still used");

    void *c = my_malloc(40);

    printf("\nAfter allocating 40 bytes:\n");
    heap_dump();

    check(c == a, "c reused a's old block");
    check(heap_used == 198, "heap_used did not increase after reuse/split");
    check(head->size == 40, "first block was shrunk to 40 bytes");
    check(head->is_free == 0, "first block is used again");
    check(head->next != NULL, "split created a new leftover block");
    check(head->next->size == 36, "leftover block has 36 bytes");
    check(head->next->is_free == 1, "leftover block is free");
    check(head->next->next != NULL, "leftover block points to original second block");
    check(head->next->next->size == 50, "original second block is still 50 bytes");

    void *d = my_malloc(0);
    check(d == NULL, "my_malloc(0) returns NULL");

    void *e = my_malloc(4096);
    check(e == NULL, "my_malloc(4096) returns NULL");

    my_free(NULL);
    check(1, "my_free(NULL) does not crash");

    return 0;
}