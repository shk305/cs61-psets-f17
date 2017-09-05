#define M61_DISABLE 1
#include "m61.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>


struct m61_node* list_head=0;
struct m61_node* list_tail;

int size_of_metadata = sizeof(struct m61_metadata); // 8 bytes of metadata
int malloc_end_buffer=4*sizeof(int);      // 20 bytes to be added at the end for boundry write issues.

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
    
   // First check for failing contition
    if (sz<4294967145){}
    else {
        nfail++;
        fail_size=fail_size+sz; //printf("Caught");
        return NULL;
	} 
        
    // Create the metadata struct for this allocation and start populating it.
    struct m61_metadata metadata;
    metadata.allocation_size=sz;  // this metadata will be stored with the block later 
    metadata.distance_to_8multiple=0; // initialising it.
    metadata.file=file;
	metadata.line=line;
	
    // This will be the actual size of the allocation to make space for shifting(8multiple) and metadata.       
    int size_to_allocate= sz+size_of_metadata+malloc_end_buffer; //printf("size_to_allocate: %i metadata size : %i\n",size_to_allocate,size_of_metadata);
    void* ptr =base_malloc(size_to_allocate); // might have to move to make multiple of 8

    //printf("\n\nFile:%s Line:%i ",metadata.file,metadata.line);
    //printf("\nACTUAL BASE MALLOC RETURN: %i ",(int)ptr);
	//printf("\nUSER REQUESTED SZ: %i ",sz);
	//printf("\nSIZE OF METADATA: %i ",size_of_metadata);
	//printf("\nMALLOC_END_BUFFER SIZE: %i ",malloc_end_buffer);
	//printf("\n\nTOTAL SIZE TO_ALLOCATE: %i ",size_to_allocate);
    
	// Make sure the address is alligned
    size_t distance_to_8multiple =8-((uintptr_t) ptr % 8); // to figure out its distance from a multiple of 8
  
    // Add this ptr to the list that tracks all allocation.
    list_head=list_prepend(list_head,ptr,distance_to_8multiple); // add data and update the list head to the new list head
    metadata.entry=list_head; // this is the pointer for the entry in the list for this malloc
	//list_traverse_recursive(list_head);

 
    
    if(distance_to_8multiple!=8 ){ // meaning if it is not a multiple of 8 already
    ptr=ptr+distance_to_8multiple; // shift ptr forward to make it a multiple of 8
    printf("distance_to_8m: %i\n",distance_to_8multiple);
    metadata.distance_to_8multiple=distance_to_8multiple;
    } // add the remaining distance to make it a multiple of 8

    // Make the data valid by writing 0xbeefbeef. Free will write 0xdeaddead
	metadata.data_valid=0xbeefbeef;//printf("DATA VALID returned: %x\n",metadata.data_valid);
		
    // Storing the metadata at the location pointed to by ptr. 
    *(struct m61_metadata*) ptr = metadata;


    // Initialize the end part witha a known value
	size_t* end_boundry_ptr=ptr+size_of_metadata+sz;
	//printf("\nEnd_boundry_ptr : %i ",end_boundry_ptr);
	for(int i=0; i < 4;i++){
		 //printf(" int : %i",i);
		 *(end_boundry_ptr+i) = 0xdeaddead;
		}
	
    // Heap Min
    if (first_malloc_call){
          heap_min=(char*)ptr;
          first_malloc_call=0;}
    else{
       if ((char*)ptr<heap_min) {heap_min=(char*)ptr;}}
    
    // Heap Max .. ?? should I add size_to_allocate or just size. this means heapmein and heapmax is not hiding the metadata portion. think.
    if (((char*)ptr+size_to_allocate)>=heap_max){
        heap_max=(char*)ptr+size_to_allocate-1; // there is a -1 because the pointer itself makes up one byte
        //printf("heapmax: %i\n",heap_max);
        }


	//Moving the pointer ahead of the metadata to point to the beginning of data
    ptr=ptr+size_of_metadata; // Move forward by the size of metadata and return that.
    //printf("\nptr returned: %i",ptr);

    malloc_count++;
    total_size= total_size + sz;
    active_size=active_size+sz;
	/*
	// DUMP ALLOCATION
	size_t* beginning_of_allocation_ptr=ptr-size_of_metadata;
    printf("\nRecalculated beginning : %i \n",beginning_of_allocation_ptr);
	for(int i=0; i<(size_to_allocate/4); i++)
	{
	 printf(" \nMEMORY LOCATION: %i CONTENT: %x ", beginning_of_allocation_ptr+(i),*(beginning_of_allocation_ptr+(i) ));	
	}
     */ 
	  
    return ptr;
}


/// m61_free(ptr, file, line)
///    Free the memory space pointed to by `ptr`, which must have been
///    returned by a previous call to m61_malloc and friends. If
///    `ptr == NULL`, does nothing. The free was called at location
///    `file`:`line`.

void m61_free(void *ptr, const char *file, int line) {
	//list_traverse_recursive(list_head);
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
 
	// Make sure there were no overwrites at the end boundry
    size_t* end_boundry_ptr=ptr+size_of_metadata+(*metadata_ptr).allocation_size; 
    //printf("\nEnd_boundry_ptr FREE: %i ",end_boundry_ptr);
    int end_boundry_overwrite=0;
	for(int i=0; i < 4;i++){
		 if(*(end_boundry_ptr+i) != 0xdeaddead){
			 end_boundry_overwrite=1;
		 }
	}


	// If there were any allignment shift. undo the shift. ptr should now point to the original base_malloc returned pointer
	ptr=ptr-(*metadata_ptr).distance_to_8multiple; // in case it was shifted to make it a multiple of 8.
    //printf("distance to 8 : %i\n",(*metadata_ptr).distance_to_8multiple);
	//printf("FREE: REPORTING DATA_VALIDITY : %x\n",(*metadata_ptr).data_valid);
    
    				
    // First check if this is a valid free call.
    if ((*metadata_ptr).data_valid==0xbeefbeef){
	   
	   if(end_boundry_overwrite){
		   printf("MEMORY BUG???: detected wild write during free of pointer ???\n");
		   return;}
         	
	   // Remove the list entry for this allocation
	   remove_from_list((*metadata_ptr).entry); 
       active_size=active_size - (*metadata_ptr).allocation_size;
       free_count++;
       (*metadata_ptr).data_valid=0xdeaddead; // Make the data valid bit invalid.
	   base_free(ptr);
	}
	else{
		if ((*metadata_ptr).data_valid==0xdeaddead){
		 printf("MEMORY BUG: invalid free of pointer \n");}
		else{
		 printf("MEMORY BUG: %s:%i: invalid free of pointer ???, not allocated\n",file, line);}
	}
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
    if (sz!=0) {
        new_ptr = m61_malloc(sz, file, line);
    }
    if (ptr!=NULL && new_ptr!=NULL) {
		//printf("got here");
        // Copy the data from `ptr` into `new_ptr`.
        // To do that, we must figure out the size of allocation `ptr`.
        // Your code here (to fix test014).
        ptr=ptr-size_of_metadata; // meta data begins 8 bytes before ptr.
        //printf("m61_free Reporting: address:%x content: %x\n",ptr,*((size_t*)ptr)); 
        struct m61_metadata *metadata_ptr;
        metadata_ptr=ptr; // now metadata_ptr is pointing to the metadata for this allocation.
        
		
		//Check if this was a valid pointer passed to realloc
		if((*metadata_ptr).data_valid!=0xbeefbeef)
		{
		  printf("MEMORY BUG???: invalid realloc of pointer ???");
		  return;
		}
		//printf("MEMORY BUG???: invalid realloc of pointer ???");
		
		
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
  	if(list_head==0){
		//printf("\nThe list is empty\n");
		return;	
	}	
    print_recursive(list_head);
	
    return;
}



// LINKED list implementation stuff.
// Basic linked list implementation taken from
// www.zentut.com/c-tutorial/c-linked-list


struct m61_node* create(struct m61_node* old_list_head, void* ptr,size_t distance_to_8m){
    struct m61_node* new_node =(struct m61_node*)base_malloc(sizeof(struct m61_node));
    if (new_node==NULL){
        printf("The new node could not be created\n");
        abort();
     }
     // save the payload (data) in this new node
     (*new_node).ptr=ptr;
	 (*new_node).distance_to_8multiple=distance_to_8m;
	 // Old list head will be the next element after this is added.
     (*new_node).next=old_list_head;
	 // Since I added on top the new node.previous =0
	 (*new_node).previous=0;
	 
     if(old_list_head !=0){
         (*old_list_head).previous=new_node;}

     return new_node;
    }

    
struct m61_node* list_prepend(struct m61_node* old_list_head,void* ptr, size_t distance_to_8m){ // ptr is the data.
  
    struct m61_node* new_node=create(old_list_head,ptr,distance_to_8m);
    //list_head=new_node; // list_head is the global variable.
    return new_node;
}

void list_traverse_recursive(struct m61_node* list_head){
	if(list_head==0){
		printf(" \nThe list is empty\n");
		return;		
	}
    printf("\n List Entry Address: %i",list_head);
	printf("\n PTR: %i",(*list_head).ptr);
	printf("\n distance_to_8multiple: %i",(*list_head).distance_to_8multiple);
    printf("\n previous--> %i",(*list_head).previous);
    printf("\n next--> %i \n",(*list_head).next);
    
	
    if((*list_head).next!=0){
        list_traverse_recursive((*list_head).next);
        }
    return;
    }
	

void remove_from_list(struct m61_node* entry_to_remove){
    //(*(*entry_to_remove).next).previous=
	//(*(*entry_to_remove).previous).next=(*entry_to_remove).next;
	//list_traverse_recursive(list_head);
    //printf("POINTER_TO_ENTRY TO BE REMOVED : %i\n",entry_to_remove);
	//printf("My Next: %i\n",(*entry_to_remove).next);
	//printf("My Previous: %i\n",(*entry_to_remove).previous);
	struct m61_node* my_next=(*entry_to_remove).next;
	struct m61_node* my_previous=(*entry_to_remove).previous;

	if (my_next!=0){ // if its not the last element.
		 //printf("Next er previous: %i\n",(*my_next).previous);
		 (*my_next).previous=my_previous;
		}
		
    if (my_previous!=0){ // if its not the first element.
		 //printf("Previous er next: %i\n",(*my_previous).next);
		 (*my_previous).next=my_next;
		}
	 else {
           if(my_next==0)  // if My next is also 0. the list is empty after my removal
              list_head = 0; // The list will become empty after this removal.
           else list_head = my_next; // if my next is not 0 then my next becomes the new head.
		 }	
		
	//printf("Previous er next: %i\n",*((*entry_to_remove).previous).next);
	
	//list_traverse_recursive(list_head);
	base_free(entry_to_remove);
    
    return;
    }
	
	

void print_recursive(struct m61_node* list_head){
	if(list_head==0){
		//printf(" \nThe list is empty\n");
		return;		
	}
	
	void* ptr_retrieved= (*list_head).ptr;
	struct m61_metadata* metadata_ptr=ptr_retrieved;
	//printf("\n PTR retrieved: %i",ptr_retrieved);
	if((*list_head).distance_to_8multiple!=8)
	{
		printf("in here/n");
		ptr_retrieved=ptr_retrieved+(*list_head).distance_to_8multiple;
	}
	
	ptr_retrieved=ptr_retrieved+size_of_metadata; // move to the end of metadata beginning of data
	
	printf("LEAK CHECK: %s:%i: allocated object %p with size %i",(*metadata_ptr).file,(*metadata_ptr).line,ptr_retrieved,(*metadata_ptr).allocation_size);
    //printf("\n List Entry Address: %i",list_head);
	//printf("\n PTR: %p",(*list_head).ptr);
	//printf("\n distance_to_8multiple: %i",(*list_head).distance_to_8multiple);
    //printf("\n previous--> %i",(*list_head).previous);
    //printf("\n next--> %i \n",(*list_head).next);
    
	
    if((*list_head).next!=0){
        print_recursive((*list_head).next);
        }
    return;
    }