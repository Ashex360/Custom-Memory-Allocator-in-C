#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Use quotes for local header
#include "allocator.h"

int main() {
    printf("=== CUSTOM MEMORY ALLOCATOR TEST ===\n");
    
    // Initialize allocator
    allocator_init();
    
    printf("\n1. Basic Allocation Test:\n");
    void* ptr1 = my_malloc(100);
    if (ptr1) {
        memset(ptr1, 'A', 100);
        printf("   Initialized 100 bytes with 'A'\n");
    }
    
    void* ptr2 = my_malloc(200);
    if (ptr2) {
        memset(ptr2, 'B', 200);
        printf("   Initialized 200 bytes with 'B'\n");
    }
    
    print_memory_stats();
    
    printf("\n2. Free Test:\n");
    my_free(ptr1);
    printf("   Freed first block\n");
    
    print_memory_stats();
    
    printf("\n3. Reuse Freed Memory:\n");
    void* ptr3 = my_malloc(50);
    if (ptr3) {
        memset(ptr3, 'C', 50);
        printf("   Reused freed space for 50 bytes\n");
    }
    
    print_memory_stats();
    
    printf("\n4. Multiple Allocations:\n");
    void* blocks[3];
    for (int i = 0; i < 3; i++) {
        blocks[i] = my_malloc(80);
        if (blocks[i]) {
            printf("   Allocated block %d\n", i);
        }
    }
    
    print_memory_stats();
    
    printf("\n5. Cleanup:\n");
    my_free(ptr2);
    my_free(ptr3);
    for (int i = 0; i < 3; i++) {
        my_free(blocks[i]);
    }
    
    printf("   All memory freed\n");
    
    print_memory_stats();
    allocator_destroy();
    
    printf("\n=== TEST COMPLETE ===\n");
    printf("Press Enter to exit...");
    getchar();
    
    return 0;
}
