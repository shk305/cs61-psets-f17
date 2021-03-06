#ifndef M61_H
#define M61_H 1
#include <stdlib.h>
#include <inttypes.h>


/// m61_malloc(sz, file, line)
///    Return a pointer to `sz` bytes of newly-allocated dynamic memory.
void* m61_malloc(size_t sz, const char* file, int line);

/// m61_free(ptr, file, line)
///    Free the memory space pointed to by `ptr`.
void m61_free(void* ptr, const char* file, int line);

/// m61_realloc(ptr, sz, file, line)
///    Reallocate the dynamic memory pointed to by `ptr` to hold at least
///    `sz` bytes, returning a pointer to the new block.
void* m61_realloc(void* ptr, size_t sz, const char* file, int line);

/// m61_calloc(nmemb, sz, file, line)
///    Return a pointer to newly-allocated dynamic memory big enough to
///    hold an array of `nmemb` elements of `sz` bytes each. The memory
///    is initialized to zero.
void* m61_calloc(size_t nmemb, size_t sz, const char* file, int line);


/// m61_statistics
///    Structure tracking memory statistics.
struct m61_statistics {
    unsigned long long nactive;         // # active allocations
    unsigned long long active_size;     // # bytes in active allocations
    unsigned long long ntotal;          // # total allocations
    unsigned long long total_size;      // # bytes in total allocations
    unsigned long long nfail;           // # failed allocation attempts
    unsigned long long fail_size;       // # bytes in failed alloc attempts
    char* heap_min;                     // smallest allocated addr
    char* heap_max;                     // largest allocated addr
};

/// shk metadata struct
struct m61_metadata {
    size_t allocation_size;         // # active allocations
    size_t distance_to_8multiple;   // shifted by this to make it a multiple of 8
    struct m61_node* entry;             // Pointer to list entry
    size_t data_valid;              // if deadbeaf. data not valid
	const char* file;               // file name
	int line;                       // Line Number
};

//// LINKED LIST IMPLEMENTATION STUFF

/// Each node of the linked list.
struct m61_node{
        void* ptr;
		size_t distance_to_8multiple;
		size_t data_valid;
        struct m61_node* previous;
        struct m61_node* next; 
    };

/// m61_getstatistics(stats)
///    Store the current memory statistics in `*stats`.
void m61_getstatistics(struct m61_statistics* stats);

/// m61_printstatistics()
///    Print the current memory statistics.
void m61_printstatistics(void);

/// m61_printleakreport()
///    Print a report of all currently-active allocated blocks of dynamic
///    memory.
void m61_printleakreport(void);


#if !M61_DISABLE
// Redefine the `malloc` family of calls to use our versions.
#define malloc(sz)              m61_malloc((sz), __FILE__, __LINE__)
#define free(ptr)               m61_free((ptr), __FILE__, __LINE__)
#define realloc(ptr, sz)        m61_realloc((ptr), (sz), __FILE__, __LINE__)
#define calloc(nmemb, sz)       m61_calloc((nmemb), (sz), __FILE__, __LINE__)
#endif


// `m61.c` should use these functions rather than malloc() and free().
void* base_malloc(size_t sz);
void base_free(void* ptr);
void base_malloc_disable(int is_disabled);


// LIST function prototipes.
struct m61_node* list_prepend(struct m61_node* list_head,void* ptr,size_t distance_to_8m);
void list_traverse_recursive(struct m61_node* list_head);
struct m61_node* create(struct m61_node* next, void* ptr,size_t distance_to_8m);
void remove_from_list(struct m61_node* entry_to_remove);

void pointer_check_recursive(struct m61_node* list_head, void* ptr);


void print_recursive(struct m61_node* list_head);

#endif
