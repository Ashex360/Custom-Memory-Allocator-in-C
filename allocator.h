#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>

#define HEAP_SIZE (1024 * 1024)  // 1MB heap
#define MIN_BLOCK_SIZE 16        // Minimum block size
#define ALIGNMENT 8              // Memory alignment

// Block metadata structure
typedef struct block_metadata {
    size_t size;                 // Size of the block (including metadata)
    int free;                    // Free flag (1 = free, 0 = allocated)
    struct block_metadata* next; // Next block in the list
    struct block_metadata* prev; // Previous block
} block_meta;

// Main allocator structure
typedef struct {
    void* heap_start;
    void* heap_end;
    block_meta* free_list;       // List of free blocks
    size_t allocated_bytes;
    size_t free_bytes;
    size_t fragmentation_count;
} allocator_t;

// Public API
void* my_malloc(size_t size);
void my_free(void* ptr);
void allocator_init(void);
void allocator_destroy(void);
void print_memory_stats(void);

#endif
