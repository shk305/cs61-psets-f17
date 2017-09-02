#define M61_DISABLE 1
#include "m61.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>


unsigned long long malloc_count = 0;          // # my malloc count
unsigned long long free_count = 0;          // # my free count

unsigned long long nactive = 0;          // # active allocations
unsigned long long active_size = 0;      // # bytes in active allocations
unsigned long long ntotal = 0;           // # total allocations
unsigned long long total_size = 0;       // # bytes in total allocations
unsigned long long nfail = 0;            // # failed allocation attempts
unsigned long long fail_size = 0;        // # bytes in failed alloc attempts
//char* heap_min;                       // smallest allocated addr
//char* heap_max;                       // largest allocated addr

/// m61_malloc(sz, file, line)
///    Return a pointer to `sz` bytes of newly-allocated dynamic memory.
///    The memory is not initialized. If `sz == 0`, then m61_malloc may
///    either return NULL or a unique, newly-allocated pointer value.
///    The allocation request was at location `file`:`line`.

void* m61_malloc(size_t sz, const char* file, int line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings
    // Your code here.
    int size_of_metadata = sizeof(struct m61_metadata);
    void* ptr =base_malloc(sz);
    
   // Check for fail
    if (sz<4294967145){}

    else {
        nfail++;
        fail_size=fail_size+sz;
        return ptr;} 
    
    
    // Create metadata struct and populate it.
    struct m61_metadata metadata;
    metadata.allocation_size=sz;
    //printf("allocation size:%x\n",metadata.allocation_size);
    *(struct m61_metadata*) ptr = metadata;
    //printf("BEFORE address:%x content: %x\n",ptr,*((size_t*)ptr));    
    ptr=ptr+size_of_metadata;
    //printf("AFTER address:%x content:%x\n",ptr,*((size_t*)ptr));
    
    

    malloc_count++;
    total_size= total_size + sz;
    active_size=active_size+sz;
      
    return ptr;
}


/// m61_free(ptr, file, line)
///    Free the memory space pointed to by `ptr`, which must have been
///    returned by a previous call to m61_malloc and friends. If
///    `ptr == NULL`, does nothing. The free was called at location
///    `file`:`line`.

void m61_free(void *ptr, const char *file, int line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings
    // Your code here.
    int size_of_metadata = sizeof(struct m61_metadata);
    ptr=ptr-size_of_metadata;
    //printf("m61_free Reporting: address:%x content: %x\n",ptr,*((size_t*)ptr)); 
    size_t freed_size =*((size_t*)ptr);
 
    active_size=active_size - freed_size;
    free_count++;
    base_free(ptr);
}


/// m61_realloc(ptr, sz, file, line)
///    Reallocate the dynamic memory pointed to by `ptr` to hold at least
///    `sz` bytes, returning a pointer to the new block. If `ptr` is NULL,
///    behaves like `m61_malloc(sz, file, line)`. If `sz` is 0, behaves
///    like `m61_free(ptr, file, line)`. The allocation request was at
///    location `file`:`line`.

void* m61_realloc(void* ptr, size_t sz, const char* file, int line) {
    void* new_ptr = NULL;
    if (sz) {
        new_ptr = m61_malloc(sz, file, line);
    }
    if (ptr && new_ptr) {
        // Copy the data from `ptr` into `new_ptr`.
        // To do that, we must figure out the size of allocation `ptr`.
        // Your code here (to fix test014).
    }
    m61_free(ptr, file, line);
    return new_ptr;
}


/// m61_calloc(nmemb, sz, file, line)
///    Return a pointer to newly-allocated dynamic memory big enough to
///    hold an array of `nmemb` elements of `sz` bytes each. The memory
///    is initialized to zero. If `sz == 0`, then m61_malloc may
///    either return NULL or a unique, newly-allocated pointer value.
///    The allocation request was at location `file`:`line`.

void* m61_calloc(size_t nmemb, size_t sz, const char* file, int line) {
    // Your code here (to fix test016).
    void* ptr = m61_malloc(nmemb * sz, file, line);
    if (ptr) {
        memset(ptr, 0, nmemb * sz);
    }
    return ptr;
}


/// m61_getstatistics(stats)
///    Store the current memory statistics in `*stats`.

void m61_getstatistics(struct m61_statistics* stats) {
    // Stub: set all statistics to enormous numbers
    memset(stats, 255, sizeof(struct m61_statistics));
    // Your code here.
    stats->nactive=malloc_count-free_count;
    stats->ntotal= malloc_count;
    stats->nfail=nfail;
    stats->active_size=active_size;
    stats->total_size=total_size;
    stats->fail_size=fail_size;
}


/// m61_printstatistics()
///    Print the current memory statistics.

void m61_printstatistics(void) {
    struct m61_statistics stats;
    m61_getstatistics(&stats);
    
    printf("malloc count: active %10llu   total %10llu   fail %10llu\n",
           stats.nactive, stats.ntotal, stats.nfail);
    printf("malloc size:  active %10llu   total %10llu   fail %10llu\n",
           stats.active_size, stats.total_size, stats.fail_size);
}


/// m61_printleakreport()
///    Print a report of all currently-active allocated blocks of dynamic
///    memory.

void m61_printleakreport(void) {
    // Your code here.
}
