// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct Memory{
  struct spinlock lock;
  struct run *freelist;
  char name[10];
} kmem,kmems[NCPU];

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  for(int i=0;i<NCPU;i++){
    snprintf(kmems[i].name, 9, "kmem%d", i);
    initlock(&kmems[i].lock, kmems[i].name);
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
    return kfree_v2(pa);
//   struct run *r;

//   if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
//     panic("kfree");

//   // Fill with junk to catch dangling refs.
//   memset(pa, 1, PGSIZE);

//   r = (struct run*)pa;

//   acquire(&kmem.lock);
//   r->next = kmem.freelist;
//   kmem.freelist = r;
//   release(&kmem.lock);
}

void kfree_v2(void *pa){
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;
  push_off();
  int id=cpuid();
  struct Memory* kmem=&kmems[id];
  pop_off();
  acquire(&kmem->lock);
  r->next = kmem->freelist;
  kmem->freelist = r;
  release(&kmem->lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
    return kalloc_v2();
//   struct run *r;

//   acquire(&kmem.lock);
//   r = kmem.freelist;
//   if(r)
//     kmem.freelist = r->next;
//   release(&kmem.lock);

//   if(r)
//     memset((char*)r, 5, PGSIZE); // fill with junk
//   return (void*)r;
}

void* steal(int id);
void * kalloc_from(struct Memory* kmem);

void * kalloc_from(struct Memory* kmem){
  struct run *r;
  acquire(&kmem->lock);
  r = kmem->freelist;
  if(r)
    kmem->freelist = r->next;
  release(&kmem->lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
void* steal(int id){
    struct run* r;
    for(int i=0;i<NCPU;i++){
        if(i==id)continue;
        struct Memory* kmem = &kmems[i];
        r=kalloc_from(kmem);
        if(r) return r;
    }
    return 0;
}

void* kalloc_v2(void){
  struct run *r;
  push_off();
  int id=cpuid();
  struct Memory* kmem=&kmems[id];
  pop_off();
  acquire(&kmem->lock);
  r = kmem->freelist;
  if(r){
    kmem->freelist = r->next;
    memset((char*)r, 5, PGSIZE); // fill with junk
    release(&kmem->lock);
  }else{
    release(&kmem->lock);
    r = steal(id);
  }
  return (void*)r;
}


