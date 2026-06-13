#include <stdio.h>
#include "allocator.h"

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

    /*
    Test 1: Basic allocation
    */
    void *a = my_malloc(100);
    void *b = my_malloc(50);

    printf("After allocating a=100 and b=50:\n");
    heap_dump();

    check(a != NULL, "my_malloc(100) returns non-NULL");
    check(b != NULL, "my_malloc(50) returns non-NULL");
    check(heap_used == 198, "heap_used is 198 after 100 and 50 byte allocations");
    check(head != NULL, "head is not NULL after allocation");
    check(head->size == 100, "first block size is 100");
    check(head->next != NULL, "second block exists");
    check(head->next->size == 50, "second block size is 50");

    /*
    Test 2: Free first block
    */
    my_free(a);

    printf("\nAfter freeing a:\n");
    heap_dump();

    check(head->is_free == 1, "first block is marked free");
    check(head->next->is_free == 0, "second block is still used");

    /*
    Test 3: Reuse + split
    Original free block is 100 bytes.
    Request 40 bytes.
    Expected:
    40 used + 24 header + 36 free
    */
    void *c = my_malloc(40);

    printf("\nAfter allocating c=40 into freed a block:\n");
    heap_dump();

    check(c == a, "c reused a's old block");
    check(heap_used == 198, "heap_used did not increase after reuse/split");
    check(head->size == 40, "first block was shrunk to 40 bytes");
    check(head->is_free == 0, "first block is used");
    check(head->next != NULL, "split created leftover block");
    check(head->next->size == 36, "leftover block has 36 bytes");
    check(head->next->is_free == 1, "leftover block is free");
    check(head->next->next != NULL, "leftover block points to original second block");
    check(head->next->next->size == 50, "original second block is still 50 bytes");

    /*
    Current layout:
    Block 0: 40 used
    Block 1: 36 free
    Block 2: 50 used
    */

    /*
    Test 4: Coalescing adjacent free blocks

    Free c.
    Block 0 and Block 1 should merge:
    40 + 24 + 36 = 100
    */
    my_free(c);

    printf("\nAfter freeing c, block 0 and leftover block should coalesce:\n");
    heap_dump();

    check(head->is_free == 1, "first block is free after freeing c");
    check(head->size == 100, "first two blocks coalesced back into 100 bytes");
    check(head->next != NULL, "coalesced block still points to original second block");
    check(head->next->size == 50, "next block is still the 50-byte block");
    check(head->next->is_free == 0, "50-byte block is still used");
    check(heap_used == 198, "heap_used did not change after coalescing");

    /*
    Test 5: Coalesce all blocks

    Free b.
    Now the 100-byte free block and 50-byte free block should merge:
    100 + 24 + 50 = 174
    */
    my_free(b);

    printf("\nAfter freeing b, all blocks should coalesce:\n");
    heap_dump();

    check(head->is_free == 1, "head block is free");
    check(head->size == 174, "all blocks coalesced into one 174-byte free block");
    check(head->next == NULL, "only one block remains after full coalescing");
    check(heap_used == 198, "heap_used still remains 198 after full coalescing");

    /*
    Test 6: Reuse coalesced block

    Request 120 bytes from the 174-byte free block.
    Expected split:
    120 used + 24 header + 30 free
    */
    void *f = my_malloc(120);

    printf("\nAfter allocating f=120 from coalesced block:\n");
    heap_dump();

    check(f == a, "f reused the original start of the heap");
    check(head->size == 120, "head block was split to 120 bytes");
    check(head->is_free == 0, "head block is used");
    check(head->next != NULL, "split created leftover block after 120-byte allocation");
    check(head->next->size == 30, "leftover block has 30 bytes");
    check(head->next->is_free == 1, "leftover block is free");
    check(heap_used == 198, "heap_used did not increase after reusing coalesced block");

    /*
    Test 7: Edge cases
    */
    void *zero = my_malloc(0);
    check(zero == NULL, "my_malloc(0) returns NULL");

    void *too_big = my_malloc(4096);
    check(too_big == NULL, "my_malloc(4096) returns NULL");

    my_free(NULL);
    check(1, "my_free(NULL) does not crash");

    return 0;
}