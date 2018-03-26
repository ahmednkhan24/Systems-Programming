#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

struct memory_region{
  size_t * start;
  size_t * end;
};

struct memory_region global_mem;
struct memory_region heap_mem;
struct memory_region stack_mem;

void walk_region_and_mark(void* start, void* end);

//how many ptrs into the heap we have
#define INDEX_SIZE 1000
void* heapindex[INDEX_SIZE];


//grabbing the address and size of the global memory region from proc 
void init_global_range(){
  char file[100];
  char * line=NULL;
  size_t n=0;
  size_t read_bytes=0;
  size_t start, end;

  sprintf(file, "/proc/%d/maps", getpid());
  FILE * mapfile  = fopen(file, "r");
  if (mapfile==NULL){
    perror("opening maps file failed\n");
    exit(-1);
  }

  int counter=0;
  while ((read_bytes = getline(&line, &n, mapfile)) != -1) {
    if (strstr(line, "hw4")!=NULL){
      ++counter;
      if (counter==3){
        sscanf(line, "%lx-%lx", &start, &end);
        global_mem.start=(size_t*)start;
        // with a regular address space, our globals spill over into the heap
        global_mem.end=malloc(256);
        free(global_mem.end);
      }
    }
    else if (read_bytes > 0 && counter==3) {
      if(strstr(line,"heap")==NULL) {
        // with a randomized address space, our globals spill over into an unnamed segment directly following the globals
        sscanf(line, "%lx-%lx", &start, &end);
        printf("found an extra segment, ending at %zx\n",end);						
        global_mem.end=(size_t*)end;
      }
      break;
    }
  }
  fclose(mapfile);
}


//marking related operations

int is_marked(size_t* chunk) {
  return ((*chunk) & 0x2) > 0;
}

void mark(size_t* chunk) {
  (*chunk)|=0x2;
}

void clear_mark(size_t* chunk) {
  (*chunk)&=(~0x2);
}

// chunk related operations

#define chunk_size(c)  ((*((size_t*)c))& ~(size_t)3 ) 
void* next_chunk(void* c) { 
  if(chunk_size(c) == 0) {
    printf("Panic, chunk is of zero size.\n");
  }
  if((c+chunk_size(c)) < sbrk(0))
    return ((void*)c+chunk_size(c));
  else 
    return 0;
}
int in_use(void *c) { 
  return (next_chunk(c) && ((*(size_t*)next_chunk(c)) & 1));
}

// index related operations

#define IND_INTERVAL ((sbrk(0) - (void*)(heap_mem.start - 1)) / INDEX_SIZE)

void build_heap_index() 
{
  // TODO optional
}


// Heap after first round of allocations: 101421120, free 1, inuse 10003
// Heap after first gc(): 816, free 0, inuse 2
// Heap after second round of allocations: 101420816, free 0, inuse 10002
// Heap after clearing all but one, and gc(): 101420816, free 0, inuse 10002
// Heap after clearing last one, and gc(): 816, free 0, inuse 2
// Heap after third round of allocations: 101420816, free 0, inuse 10002
// Heap after clearing all but one, and gc(): 101420816, free 0, inuse 10002
// Heap after clearing last one, and gc(): 816, free 0, inuse 2
// Now checking stack root set handling.
// Recursive1: at depth 50, 395600, free 0, inuse 54
// After Recursive1 816, free 0, inuse 2
// Recursive2: At depth 50, 520160, free 0, inuse 103
// After Recursive2 816, free 0, inuse 2

// the actual collection code
/*
  pseudo code from page 868

  Sweep should be almost identical to the psuedo code we went over in class

  void sweep(ptr b, ptr end)
  {
    while (b < end)
    {
      if (blockMarked(b))
        unmarkBlock(b);
      else if (blockAllocated(b))
        free(b);
      b = nextBlock(b);
    }
  }
*/
void sweep() 
{
  size_t *b = heap_mem.start - 1;

  b = next_chunk(b);

  while (b != NULL && b < sbrk(0))
  {
    size_t *next = next_chunk(b);

    if (is_marked(b))
      clear_mark(b);
    else if (in_use(b))
      free(b+1);

    b = next;
  }
  return;
}

// For is pointer, you will take in a size t value, and either return NULL if it is not within the
// heap or not allocated, and otherwise return a pointer to the header of the chunk containing
// it. We give you the start and ending addresses of the heap, so figuring out if the pointer is in
// that range should be trivial. Next, I recommend starting at the beginning of the heap, and
// going through block by block, similar to what you do in the sweep stage. You are looking for
// the largest block address that is smaller than your pointer, as that will be the address of the
// block containing it.
// //determine if what "looks" like a pointer actually points to a block in the heap
size_t * is_pointer(size_t * ptr) 
{
  // TODO

  // starting at the beginning of the heap
  size_t *beginHeap = heap_mem.start - 1;

  beginHeap = next_chunk(beginHeap);

  // return NULL if it is not within the heap or not allocated
  if (ptr == NULL)
    return NULL;
  else if (ptr < beginHeap)
    return NULL;
  else if (ptr >= sbrk(0))
    return NULL;

  // otherwise return a pointer to the header of the chunk containing it
  else
  {
    size_t *n = (size_t *)(next_chunk(beginHeap));

    // looking for the largest block address that is smaller than your pointer
    while (n < ptr)
    {
      beginHeap = (size_t *)(next_chunk(beginHeap));

      n = (size_t *)(next_chunk(beginHeap));
    }
  }
  return beginHeap;
}

int length(size_t *b)
{
  return (chunk_size(b)/sizeof(size_t) - 1);
}
/*
  pseudo code from page 868

  void mark(ptr p)
  {
    if ((b = isPtr(p)) == NULL)
      return;
    if (blockMarked(b))
      return;
    markBlock(b);
    len = length(b);
    for (i = 0; i < len; i++)
      mark(b[i]);
    return;
  }
*/
void recursiveMarker(size_t *p)
{
  size_t *b = is_pointer(p);

  if (b == NULL)
    return;
  if (is_marked(b))
    return;

  mark(b);

  int len = length(b);
  int i;
  for (i = 0; i < len; i++)
  {
    recursiveMarker(b[i]);
  }
  return;
}

// For walk region and mark, extend the psuedo code we went over in class so that instead of
// just marking from one pointer, it takes a start and end address, and performs the operations
// below on every pointer within that range.
void walk_region_and_mark(void* start, void* end) 
{
  // TODO
  // ITERATE THROUGH THE CHUNKS AND THEN CALLS THE HELPER WHICH IS MARK FROM THE BOOK
 
  size_t *cur;
  for (cur = start; cur != end; cur++)
  {
    recursiveMarker(*cur);
  }
  return;
}

// standard initialization 

void init_gc() {
  size_t stack_var;
  init_global_range();
  heap_mem.start=malloc(512);
  //since the heap grows down, the end is found first
  stack_mem.end=(size_t *)&stack_var;
}

void gc() {
  size_t stack_var;
  heap_mem.end=sbrk(0);
  //grows down, so start is a lower address
  stack_mem.start=(size_t *)&stack_var;

  // build the index that makes determining valid ptrs easier
  // implementing this smart function for collecting garbage can get bonus;
  // if you can't figure it out, just comment out this function.
  // walk_region_and_mark and sweep are enough for this project.
  build_heap_index();

  //walk memory regions
  walk_region_and_mark(global_mem.start,global_mem.end);
  walk_region_and_mark(stack_mem.start,stack_mem.end);
  sweep();
}