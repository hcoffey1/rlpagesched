#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/cdefs.h>

#define MAX_PAGES 1024
#define TRUE 1
#define FALSE 0

#define ORACLE
//#define ORACLE_PROFILE

typedef unsigned int uint;
typedef unsigned long ulong;

enum SCHEDULER{history, oracle_profile, oracle, rl};

uint total_physpages;
uint total_virtpages;
uint m1_pages;
uint m2_pages;
uint m1_delay;
uint m2_delay;

uint epoch_intv;

uint page_size;
uint addr_size;
uint tlb_line_shift;
ulong addr_mask;

ulong page_faults = 0;
ulong diskrefs = 0;
ulong page_hits = 0;
ulong num_pages_ref = 0;
ulong oracle_time = 0;

uint FIRST = TRUE;

struct {
  unsigned long phypage;   /* physical page number               */
  unsigned short resident; /* is page resident in memory         */
  unsigned short dirty;    /* has page been updated since loaded */
  ulong mispredict;
} page_table[MAX_PAGES];

typedef struct phys_page{
  //uint index;
  ulong virtpage;
  ulong lru;
  ulong hits;
} phys_page;

phys_page phys_pages[MAX_PAGES];
ulong ORACLE_HITS[MAX_PAGES];

//Taken and modified from: https://www.geeksforgeeks.org/quick-sort/
// A utility function to swap two elements 
void swap(phys_page* a, phys_page* b) 
{ 
    phys_page t = *a; 
    *a = *b; 
    //a->index = t.index;
    //t.index = b->index;
    *b = t; 
} 
#if 1
/* This function takes last element as pivot, places 
   the pivot element at its correct position in sorted 
    array, and places all smaller (smaller than pivot) 
   to left of pivot and all greater elements to right 
   of pivot */
int partition (phys_page * arr, int low, int high) 
{ 
    int pivot = arr[high].hits;    // pivot 
    int i = (low - 1);  // Index of smaller element 
  
    for (int j = low; j <= high- 1; j++) 
    { 
        // If current element is smaller than the pivot 
        if (arr[j].hits > pivot) 
        { 
            i++;    // increment index of smaller element 
            swap(&arr[i], &arr[j]); 
        } 
    } 
    swap(&arr[i + 1], &arr[high]); 
    return (i + 1); 
} 
  
/* The main function that implements QuickSort 
 arr[] --> Array to be sorted, 
  low  --> Starting index, 
  high  --> Ending index */
void quickSort(phys_page * arr, int low, int high) 
{ 
    if (low < high) 
    { 
        /* pi is partitioning index, arr[p] is now 
           at right place */
        int pi = partition(arr, low, high); 
  
        // Separately sort elements before 
        // partition and after partition 
        quickSort(arr, low, pi - 1); 
        quickSort(arr, pi + 1, high); 
    } 
} 
 #endif 


/*
 * logtwo - return log2 of argument
 */
static int logtwo(unsigned a) {
  register int i;

  for (i = -1; a; i++)
    a >>= 1;
  return (i);
}

/*
 * power_of_two - indicates if a value is a positive power of two
 */
int power_of_two(unsigned int val) {
  int i, j;
  unsigned int v;

  v = val;
  j = 0;
  for (i = 0; i < sizeof(unsigned int) * 8; i++) {
    if (v & 1)
      j++;
    v = v >> 1;
  }
  return (j == 1);
}

void reset_lru() {
  for (int i = 0; i < total_physpages; i++) {
    phys_pages[i].lru = 0;
  }
}

void reset_page_hits() {
  for (int i = 0; i < total_physpages; i++) {
    phys_pages[i].hits = 0;
  }
}

/*
 * update_page_LRU - update the page LRU information
 */
static void update_page_LRU(int page) {
  int i;

  for (i = 0; i < total_physpages; i++)
    phys_pages[i].lru++;
  phys_pages[page].lru = 0;
}

/*
 * proc_page_lookup - process a page table lookup
 * output: page - physical page frame
 */
static int proc_page_lookup(char access_type, unsigned long address,
                            unsigned *page) {
  // register unsigned long c_address;
  unsigned long virtpage;
  int i, j, lru, hit;

  /* determine how to index into the page table */
  virtpage = address >> tlb_line_shift;

  /* if the page table is resident, then we know the physical page number */
  if (page_table[virtpage].resident) {
    *page = page_table[virtpage].phypage;
    page_hits++;
    hit = TRUE;
  } else {

    /* update the counters */
    hit = FALSE;
    page_faults++;
    diskrefs++;

    /* if the all of the physical pages have not yet been used */
    if (num_pages_ref < total_physpages)
    {
      page_table[virtpage].phypage = num_pages_ref++;
      phys_pages[page_table[virtpage].phypage].virtpage = virtpage;
    }
    else {

      /* find the least recently accessed physical page */
      lru = 0;
      for (i = m1_pages; i < total_physpages; i++)
        if (phys_pages[i].lru >= lru) {
          lru = phys_pages[i].lru;
          j = i;
        }

      /* now find the virtual page containing that page number */
      for (i = 0; i < total_virtpages; i++)
        if (page_table[i].resident && page_table[i].phypage == j)
          break;
      if (i == total_virtpages) {
        fprintf(stderr, "could not find physical page in page table\n");
      }

      /* if the page is dirty, then increment the number of disk
         references */
      if (page_table[i].dirty)
        diskrefs++;

      /* update information for the newly resident virtual page */
      page_table[virtpage].phypage = j;
      phys_pages[j].virtpage = virtpage;
      phys_pages[j].hits = 0;
      page_table[i].resident = FALSE;
      //phys_pages[page_table[i].phypage].virtpage = 0;
    }

    /* mark the page as resident and that it is not dirty */
    page_table[virtpage].resident = TRUE;
    page_table[virtpage].dirty = FALSE;

    /* update which physical page is used */
    *page = page_table[virtpage].phypage;
  }

  /* mark the page as the most recently used */
  update_page_LRU(*page);

  return hit;
}

int read_config(char *fileName) {
  FILE *f = fopen(fileName, "r");
  if (f == NULL) {
    printf("Failed to open config file\n");
    return 1;
  }

  if (fscanf(f, "Virt Pages:\t%u\n", &total_virtpages) != 1) {
    fprintf(stderr, "Failed to read in total virt pages.\n");
    return -1;
  }

  if (fscanf(f, "Page Total:\t%u\n", &total_physpages) != 1) {
    fprintf(stderr, "Failed to read in total phys pages.\n");
    return -1;
  }
  if (fscanf(f, "M1:\t%u\n", &m1_pages) != 1) {
    fprintf(stderr, "Failed to read in m1 phys page count.\n");
    return -1;
  }
  if (fscanf(f, "M1 Delay:\t%u\n", &m1_delay) != 1) {
    fprintf(stderr, "Failed to read in m1 delay.\n");
    return -1;
  }
  if (fscanf(f, "M2:\t%u\n", &m2_pages) != 1) {
    fprintf(stderr, "Failed to read in m2 phys page count.\n");
    return -1;
  }
  if (fscanf(f, "M2 Delay:\t%u\n", &m2_delay) != 1) {
    fprintf(stderr, "Failed to read in m2 delay.\n");
    return -1;
  }

  if (m1_pages + m2_pages != total_physpages) {
    fprintf(stderr, "Memory partition page counts sum does not match total.\n");
    return -1;
  }

  if (fscanf(f, "Page Size:\t%u\n", &page_size) != 1) {
    fprintf(stderr, "Failed to read in page size.\n");
    return -1;
  }

  if (!power_of_two(page_size)) {
    fprintf(stderr, "Page size is not a power of 2.\n");
    return -1;
  }

  tlb_line_shift = logtwo(page_size);

  if (fscanf(f, "Addr Size:\t%u\n", &addr_size) != 1) {
    fprintf(stderr, "Failed to read in address size.\n");
    return -1;
  }

  if (!power_of_two(addr_size)) {
    fprintf(stderr, "Address size is not a power of 2.\n");
    return -1;
  }

  addr_mask = (1ul << addr_size) - 1;

  if (fscanf(f, "Epoch Intv:\t%u\n", &epoch_intv) != 1) {
    fprintf(stderr, "Failed to read in epoch interval.\n");
    return -1;
  }

  fclose(f);

  return 0;
}

void schedule_epoch(enum SCHEDULER n, FILE * profOut, FILE * profIn) {

  switch (n) {
    // History Page Scheduler
  case history:
    quickSort(phys_pages, 0, total_physpages - 1);
    for (int i = 0; i < total_physpages; i++) {
      page_table[phys_pages[i].virtpage].phypage = i;
    }
    break;

  // Oracle Profiler
  case oracle_profile:
    if (__glibc_unlikely(FIRST)) {
      FIRST = FALSE;
      break;
    }
    for (int i = 0; i < total_physpages; i++) {
      fwrite(&phys_pages[i].hits, sizeof(ulong), 1, profOut);
    }
    break;

  // Oracle Scheduler
  case oracle:
    //fread(ORACLE_HITS, total_physpages, sizeof(ulong), profIn);
    //for (int i = 0; i < total_physpages; i++) {
      //phys_pages[i].hits = ORACLE_HITS[i];
    //}
    quickSort(phys_pages, 0, total_physpages - 1);
    for (int i = 0; i < total_physpages; i++) {
      page_table[phys_pages[i].virtpage].phypage = i;

      if (i < m1_pages) {
        oracle_time += m1_delay*phys_pages[i].hits;
      } else {
        oracle_time += m2_delay*phys_pages[i].hits;
      }
    }
    break;

  case rl:
    break;
  }

#if 0
  for(int i = 0; i < total_physpages; i++)
  {
    printf("%lu\n", phys_pages[i].hits);
  }

  exit(1);
#endif

  reset_lru();
  reset_page_hits();
}

/*
 * main - drives the cache simulator
 */
int main(int argc, char **argv) {
  FILE *f = NULL, *profIn = NULL, *profOut = NULL;
  ulong instAddr, memAddr;
  uint cycle = 0;
  ulong time = 0;
  uint page;
  char accessType;

  if (read_config(argv[1]) != 0) {
    return 1;
  }

  f = fopen(argv[2], "r");
  if (f == NULL) {
    printf("Failed to open trace file\n");
    return 1;
  }

#ifdef ORACLE_PROFILE
  profOut = fopen("oracle.profile", "wb");
#endif

#ifdef ORACLE
  profIn = fopen("oracle.profile", "rb");
#endif

  while (fscanf(f, "%lx: %c %lx\n", &instAddr, &accessType, &memAddr) != EOF) {

    if (cycle >= epoch_intv) {
      #ifdef ORACLE
      schedule_epoch(oracle, profOut, profIn);
      #endif
      #ifndef ORACLE
      schedule_epoch(history, profOut, profIn);
      #endif
    }

    memAddr &= addr_mask;
    proc_page_lookup(accessType, memAddr, &page);
    phys_pages[page].hits++;

    if (page < m1_pages) {
      time += m1_delay;
    } else {
      time += m2_delay;
    }

    cycle++;
  }

  printf("STATISTICS\n");
  printf("%-16s %9lu\n", "Page Hits", page_hits);
  printf("%-16s %9lu\n", "Page Faults", page_faults);
#ifndef ORACLE
  printf("%-16s %9lu\n", "Time", time);
#endif

#ifdef ORACLE
  schedule_epoch(oracle, profOut, profIn);
  printf("%-16s %9lu\n", "Oracle Time", oracle_time);
#endif

#ifdef ORACLE_PROFILE
  fclose(profOut);
#endif

#ifdef ORACLE
  fclose(profIn);
#endif

  return 0;
}
