#include "allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static allocator_t allocator;

// Alignment helper
static size_t align_up(size_t size) {
    return (size + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
}

// Initialize the allocator
void allocator_init(void) {
    allocator.heap_start = malloc(HEAP_SIZE);
    if (!allocator.heap_start) {
        fprintf(stderr, "Failed to allocate heap memory\n");
        exit(1);
    }
    
    allocator.heap_end = (char*)allocator.heap_start + HEAP_SIZE;
    
    block_meta* initial_block = (block_meta*)allocator.heap_start;
    initial_block->size = HEAP_SIZE;
    initial_block->free = 1;
    initial_block->next = NULL;
    initial_block->prev = NULL;
    
    allocator.free_list = initial_block;
    allocator.allocated_bytes = 0;
    allocator.free_bytes = HEAP_SIZE - sizeof(block_meta);
    allocator.fragmentation_count = 0;
    
    printf("Allocator initialized with %lu bytes heap\n", HEAP_SIZE);
}

// Cleanup allocator
void allocator_destroy(void) {
    free(allocator.heap_start);
    printf("Allocator destroyed\n");
}

// Find a free block using first-fit strategy
block_meta* find_free_block(size_t size) {
    block_meta* current = allocator.free_list;
    
    while (current) {
        if (current->free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Add block to free list
void add_to_free_list(block_meta* block) {
    if (!allocator.free_list) {
        allocator.free_list = block;
        block->prev = NULL;
        block->next = NULL;
        return;
    }
    
    block_meta* current = allocator.free_list;
    block_meta* prev = NULL;
    
    while (current && current < block) {
        prev = current;
        current = current->next;
    }
    
    if (prev) {
        prev->next = block;
    } else {
        allocator.free_list = block;
    }
    
    block->prev = prev;
    block->next = current;
    
    if (current) {
        current->prev = block;
    }
}

// Remove block from free list
void remove_from_free_list(block_meta* block) {
    if (!block) return;
    
    if (block->prev) {
        block->prev->next = block->next;
    } else {
        allocator.free_list = block->next;
    }
    
    if (block->next) {
        block->next->prev = block->prev;
    }
    
    block->prev = NULL;
    block->next = NULL;
}

// Split a block if it's too large
void split_block(block_meta* block, size_t size) {
    size_t remaining_size = block->size - size;
    
    if (remaining_size >= sizeof(block_meta) + MIN_BLOCK_SIZE) {
        block_meta* new_block = (block_meta*)((char*)block + size);
        new_block->size = remaining_size;
        new_block->free = 1;
        new_block->prev = block;
        new_block->next = block->next;
        
        block->size = size;
        block->next = new_block;
        
        if (new_block->next) {
            new_block->next->prev = new_block;
        }
        
        add_to_free_list(new_block);
        allocator.fragmentation_count++;
    }
}

// Merge adjacent free blocks
void coalesce_free_blocks(block_meta* block) {
    if (block->next && 
        (char*)block + block->size == (char*)block->next &&
        block->next->free) {
        
        block->size += block->next->size;
        block->next = block->next->next;
        
        if (block->next) {
            block->next->prev = block;
        }
        
        allocator.fragmentation_count--;
    }
    
    if (block->prev && 
        (char*)block->prev + block->prev->size == (char*)block &&
        block->prev->free) {
        
        block->prev->size += block->size;
        block->prev->next = block->next;
        
        if (block->next) {
            block->next->prev = block->prev;
        }
        
        block = block->prev;
        allocator.fragmentation_count--;
    }
    
    if (block->free && !(block->prev && block->prev->free)) {
        remove_from_free_list(block);
        add_to_free_list(block);
    }
}

// Custom malloc implementation
void* my_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    size_t total_size = align_up(size + sizeof(block_meta));
    if (total_size < MIN_BLOCK_SIZE) {
        total_size = MIN_BLOCK_SIZE;
    }
    
    printf("\n[my_malloc] Requested: %lu bytes\n", size);
    
    block_meta* block = find_free_block(total_size);
    
    if (!block) {
        printf("[ERROR] No suitable block found\n");
        return NULL;
    }
    
    split_block(block, total_size);
    block->free = 0;
    remove_from_free_list(block);
    
    allocator.allocated_bytes += block->size - sizeof(block_meta);
    allocator.free_bytes -= block->size;
    
    void* ptr = (char*)block + sizeof(block_meta);
    
    printf("[SUCCESS] Allocated %lu bytes at %p\n", 
           block->size - sizeof(block_meta), ptr);
    
    return ptr;
}

// Custom free implementation
void my_free(void* ptr) {
    if (!ptr) {
        return;
    }
    
    block_meta* block = (block_meta*)((char*)ptr - sizeof(block_meta));
    
    if (block->free) {
        printf("[ERROR] Double free detected\n");
        return;
    }
    
    printf("\n[my_free] Freeing memory at %p\n", ptr);
    
    block->free = 1;
    allocator.allocated_bytes -= block->size - sizeof(block_meta);
    allocator.free_bytes += block->size;
    
    add_to_free_list(block);
    coalesce_free_blocks(block);
}

// Print memory statistics
void print_memory_stats(void) {
    printf("\n========== MEMORY STATISTICS ==========\n");
    printf("Heap size: %lu bytes\n", HEAP_SIZE);
    printf("Allocated: %lu bytes\n", allocator.allocated_bytes);
    printf("Free: %lu bytes\n", allocator.free_bytes);
    printf("Fragmentation count: %lu\n", allocator.fragmentation_count);
    
    block_meta* current = (block_meta*)allocator.heap_start;
    void* heap_end = (char*)allocator.heap_start + HEAP_SIZE;
    size_t total_blocks = 0;
    
    while ((void*)current < heap_end) {
        total_blocks++;
        printf("Block %lu: %p, Size: %lu, %s\n",
               total_blocks, 
               (void*)current,
               current->size,
               current->free ? "FREE" : "ALLOCATED");
        
        current = (block_meta*)((char*)current + current->size);
    }
    
    printf("=======================================\n");
}
