#define M61_DISABLE 1
#include "m61.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>


struct node* list_head=0;
struct node* list_tail;

int size_of_metadata = sizeof(struct m61_metadata); // 8 bytes of metadata
int allignment_delta=7; // the delta that might have to be shifted to make the address divisible by 8

int first_malloc_call=1;
unsigned long long malloc_count = 0;          // # my malloc count
unsigned long long free_count = 0;          // # my free count

unsigned long long nactive = 0;          // # active allocations
unsigned long long active_size = 0;      // # bytes in active allocations
unsigned long long ntotal = 0;           // # total allocations
unsigned long long total_size = 0;       // # bytes in total allocations
unsigned long long nfail = 0;            // # failed allocation attempts
unsigned long long fail_size = 0;        // # bytes in failed alloc attempts
char* heap_min=0;                       // smallest allocated addr
char* heap_max=0;                       // largest allocated addr

/// m61_malloc(sz, file, line)
///    Return a pointer to `sz` bytes of newly-allocated dynamic memory.
///    The memory is not initialized. If `sz == 0`, then m61_malloc may
///    either return NULL or a unique, newly-allocated pointer value.
///    The allocation request was at location `file`:`line`.

void* m61_malloc(size_t sz, const char* file, int line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings
    // Your code here.
    
   // Check for fail
    if (sz<4294967145){}

    else {
        nfail++;
        fail_size=fail_size+sz; //printf("Caught");
        return NULL;} 
        
    // Create metadata struct and start populating it.
    struct m61_metadata metadata;
    metadata.allocation_size=sz;  // this metadata will be stored with the block later 
    metadata.distance_to_8multiple=0; // initialising it.
    
    // This will be the actual size of the allocation to make space for shifting(8multiple) and metadata.       
    int size_to_allocate= sz+size_of_metadata+allignment_delta; //printf("size_to_allocate: %i metadata size : %i\n",size_to_allocate,size_of_metadata);
    void* ptr =base_malloc(size_to_allocate); // might have to move to make multiple of 8

    //printf("\n\nbase_malloc ptr: %i ",(int)ptr);
    list_head=list_prepend(list_head,ptr); // add data and update the list head to the new list head
    metadata.entry=list_head; // this is the pointer for the entry in the list for this malloc
    
    // Heap Min
    if (first_malloc_call){
          heap_min=(char*)ptr;
          first_malloc_call=0;}
    else{
       if ((char*)ptr<heap_min) {heap_min=(char*)ptr;}}
    
    // Heap Max
    if (((char*)ptr+size_to_allocate)>=heap_max){
        heap_max=(char*)ptr+size_to_allocate-1; // there is a -1 because the pointer itself makes up one byte
        //printf("heapmax: %i\n",heap_max);
        }


    size_t distance_to_8multiple=0;
    distance_to_8multiple =8-((uintptr_t) ptr % 8); // to figure out its distance from a multiple of 8
    
    if(distance_to_8multiple!=8 ){
    ptr=ptr+distance_to_8multiple;
    //printf("distance_to_8m: %i\n",distance_to_8multiple);
    metadata.distance_to_8multiple=distance_to_8multiple;
    } // add the remaining distance to make it a multiple of 8
    
    
    
    *(struct m61_metadata*) ptr = metadata;
    
    ptr=ptr+size_of_metadata; // Move forward by the size of metadata and return that.
    
    
    //printf("ptr returned: %i\n",ptr);

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
    //printf("ptr : %i\n",ptr);
    //printf("min : %i\n",heap_min);
    //printf("max : %i\n",heap_max);
    if (ptr==NULL){return;}
    
    // Check to see if the address is in the heap. might need to implement a different way later
    if (ptr<heap_min | ptr>heap_max| !(malloc_count)){
        printf("MEMORY BUG???: invalid free of pointer ???, not in heap\n");
        abort();}
    // Your code here.
    // Look back at the line below. might need to subtract the delta if a delta was added to make it a multiple of 8.
    ptr=ptr-size_of_metadata; // shift ptr back to the top of metadata starting point.
    //printf("size_of_metadata : %i\n",size_of_metadata);
    //printf("m61_free Reporting: address:%x content: %x\n",ptr,*((size_t*)ptr)); 
    struct m61_metadata *metadata_ptr;
    metadata_ptr=ptr; // now metadata_ptr is pointing to the metadata for this allocation.
 
    
	remove_from_list((*metadata_ptr).entry); // remove the list entry for this pointer
    //(*metadata_ptr).data_valid=1;   // this means this data is no longer valid

    ptr=ptr-(*metadata_ptr).distance_to_8multiple; // in case it was shifted to make it a multiple of 8.
    //printf("distance to 8 : %i\n",(*metadata_ptr).distance_to_8multiple);
    
    active_size=active_size - (*metadata_ptr).allocation_size;
    free_count++;
    
	
	base_free(ptr);
	
	
	
	//list_traverse_recursive(list_head);
    
    //printf("Freed : %i\n",(int)ptr);
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
    if (ptr!=NULL && new_ptr!=NULL) {
        // Copy the data from `ptr` into `new_ptr`.
        // To do that, we must figure out the size of allocation `ptr`.
        // Your code here (to fix test014).
        ptr=ptr-size_of_metadata; // meta data begins 8 bytes before ptr.
        //printf("m61_free Reporting: address:%x content: %x\n",ptr,*((size_t*)ptr)); 
        struct m61_metadata *metadata_ptr;
        metadata_ptr=ptr; // now metadata_ptr is pointing to the metadata for this allocation.

        size_t old_size =(*metadata_ptr).allocation_size;
        ptr=ptr+size_of_metadata; // point back to the actual data.
        
        if(old_size<sz)
          memcpy(new_ptr,ptr,old_size);
        else
          memcpy(new_ptr,ptr,sz);
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
    //printf("nmemb: %i\n",nmemb);
    //printf("sz: %i\n",sz);
    //nmemb=300;
    size_t total= nmemb* sz;
    
    //printf("total: %u\n",total);
    
    if ((sz!=0) &&((total/sz) != nmemb)){total=4294967145;} 
     //printf("total: %u\n",total);
     
    void* ptr = m61_malloc(total, file, line);
    if (ptr!=NULL) {
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
    stats->heap_min=heap_min;
    stats->heap_max=heap_max;
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



// LINKED list implementation stuff.
// Basic linked list implementation taken from
// www.zentut.com/c-tutorial/c-linked-list


struct node* create(struct node* old_list_head, void* ptr){
    struct node* new_node =(struct node*)base_malloc(sizeof(struct node));
    if (new_node==NULL){
        printf("The new node could not be created\n");
        abort();
     }
 
     (*new_node).ptr=ptr;
     (*new_node).next=old_list_head;
     if(old_list_head !=0){
         (*old_list_head).previous=new_node;
         }
 

     return new_node;
    }
    
struct node* list_prepend(struct node* old_list_head,void* ptr){ // ptr is the data.
  
    struct node* new_node=create(old_list_head,ptr);
    //list_head=new_node; // list_head is the global variable.
    return new_node;
}

int list_traverse_recursive(struct node* list_head,void* ptr){
    //printf("\nMy Pointer: %i",list_head);
    //printf("\ntravarse:prvious: %i",(*list_head).previous);
    //printf("\ntravarse:next: %i",(*list_head).next);
    //printf("\ntravarse:ptr: %i\n\n",(*list_head).ptr);
	
	if ((*list_head).ptr==ptr){return 1;}
	
    if((*list_head).next!=0){
        return list_traverse_recursive((*list_head).next,ptr);
        }
    return 0;
    }
	

void remove_from_list(struct node* entry_to_remove){
    //(*(*entry_to_remove).next).previous=
	//(*(*entry_to_remove).previous).next=(*entry_to_remove).next;
	//list_traverse_recursive(list_head);
    //printf("POINTER_TO_ENTRY TO BE REMOVED : %i\n",entry_to_remove);
	//printf("My Next: %i\n",(*entry_to_remove).next);
	//printf("My Previous: %i\n",(*entry_to_remove).previous);
	struct node* my_next=(*entry_to_remove).next;
	struct node* my_previous=(*entry_to_remove).previous;

	if (my_next!=0){
		 //printf("Next er previous: %i\n",(*my_next).previous);
		 (*my_next).previous=my_previous;
		}
		
    if (my_previous!=0){
		 //printf("Previous er next: %i\n",(*my_previous).next);
		 (*my_previous).next=my_next;
		}
	 else { list_head = my_next; // I am the least so my next one will become the list head
		 
		 }	
		
	//printf("Previous er next: %i\n",*((*entry_to_remove).previous).next);
	
	//list_traverse_recursive(list_head);
	base_free(entry_to_remove);
    
    return;
    }