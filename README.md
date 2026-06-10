## Memory Allocator

-This project is my take at creating a memory allocator

-Instead of using the OS for memory, it is fixed in an array

-Each block has a header that will contain the meta data(size,is_free,and next)

-The allocator will return a pointer to usable/editable memeory, not to the header

Before any allocation:

[ 4096 bytes free ]

After my_malloc(100):

[ header ][ 100 bytes user memory ][ remaining free space ]

After my_malloc(50):

[ header ][ 100 bytes ][ header ][ 50 bytes ][ remaining free space ]
