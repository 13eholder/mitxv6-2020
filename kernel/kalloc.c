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

// int refcount[(PHYSTOP-KERNBASE)/PGSIZE];
// struct spinlock reflock;
// #define RCID(pa) (((uint64)pa-KERNBASE)/PGSIZE)
// void refup(void* pa){
//     if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
//         panic("refup pa wrong!");
//     acquire(&reflock);
//     ++refcount[RCID(pa)];
//     release(&reflock);
// }

// void refdown(void* pa){
//     if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
//         panic("refdown pa wrong!");
//     acquire(&reflock);
//     --refcount[RCID(pa)];
//     release(&reflock);  
// }

// int rc(void *pa){
//     acquire(&reflock);
//     int res=refcount[RCID(pa)];
//     release(&reflock);
//     return res;
// }

// void setrc(void *pa,int v){
//     acquire(&reflock);
//     refcount[RCID(pa)]=v;
//     release(&reflock);
// }

struct ref_stru {
  struct spinlock lock;
  int cnt[PHYSTOP / PGSIZE];  // 引用计数
} ref;

int krefcnt(void* pa) {
  return ref.cnt[(uint64)pa / PGSIZE];
}

int kaddrefcnt(void* pa) {
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    return -1;
  acquire(&ref.lock);
  ++ref.cnt[(uint64)pa / PGSIZE];
  release(&ref.lock);
  return 0;
}

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&ref.lock, "ref");
//   initlock(&reflock, "refcount");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    // setrc(p, 1);
    ref.cnt[(uint64)p / PGSIZE] = 1;
    kfree(p);
  }

}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");
  acquire(&ref.lock);
  if(--ref.cnt[(uint64)pa / PGSIZE] == 0) {
    release(&ref.lock);

    r = (struct run*)pa;

    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  } else {
    release(&ref.lock);
  }
  
//    refdown(pa);
//     if(rc(pa)==0 ){
//          // Fill with junk to catch dangling refs.
//         memset(pa, 1, PGSIZE);

//         r = (struct run*)pa;

//         acquire(&kmem.lock);
//         r->next = kmem.freelist;
//         kmem.freelist = r;
//         release(&kmem.lock);
//     }
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
//   if(r)
//     kmem.freelist = r->next;
  if(r) {
    kmem.freelist = r->next;
    acquire(&ref.lock);
    ref.cnt[(uint64)r / PGSIZE] = 1;  // 将引用计数初始化为1
    release(&ref.lock);
  }
  release(&kmem.lock);

  if(r){
    memset((char*)r, 5, PGSIZE); // fill with junk
    // refup(r);
  }
  return (void*)r;
}
