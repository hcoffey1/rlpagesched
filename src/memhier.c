#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#define MAX_PAGES 1024
#define TRUE 1
#define FALSE 0

typedef unsigned int uint;
typedef unsigned long ulong;

uint total_physpages;
uint total_virtpages;
uint m1_pages;
uint m2_pages;
uint page_size;
uint addr_size;
uint tlb_line_shift;
ulong addr_mask;

ulong page_faults = 0;
ulong diskrefs = 0;
ulong page_hits = 0;
ulong num_pages_ref = 0;

struct {
  unsigned long phypage;   /* physical page number               */
  unsigned short resident; /* is page resident in memory         */
  unsigned short dirty;    /* has page been updated since loaded */
} page_table[MAX_PAGES];

ulong pages[MAX_PAGES];

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

/*
 * update_page_LRU - update the page LRU information
 */
static void update_page_LRU(int page) {
#if 0
   int i;

   for (i = 0; i < numphypages; i++)
      pages[i]++;
   pages[page] = 0;
#endif
}

/*
 * proc_page_lookup - process a page table lookup
 */
static int proc_page_lookup(char access_type, unsigned long address,
                            unsigned *page) {
#if 1
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
      page_table[virtpage].phypage = num_pages_ref++;
    else {

      /* find the least recently accessed physical page */
      lru = 0;
      for (i = 0; i < total_physpages; i++)
        if (pages[i] > lru) {
          lru = pages[i];
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
      page_table[i].resident = FALSE;
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
#endif
  return 0;
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
  if (fscanf(f, "M2:\t%u\n", &m2_pages) != 1) {
    fprintf(stderr, "Failed to read in m2 phys page count.\n");
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

  fclose(f);

  return 0;
}

/*
 * main - drives the cache simulator
 */
int main(int argc, char **argv) {
  FILE *f;
  ulong instAddr, memAddr;
  char accessType;


  if (read_config(argv[1]) != 0) {
    return 1;
  }

  f = fopen(argv[2], "r");
  if (f == NULL) {
    printf("Failed to open trace file\n");
    return 1;
  }

  while(fscanf(f, "%lx: %c %lx\n", &instAddr, &accessType, &memAddr) != EOF)
  {
    printf("%lx\n", addr_mask);
    memAddr &= addr_mask;
    printf("%lx : %c %lx %lx\n", instAddr, accessType, memAddr, memAddr >> tlb_line_shift);
  }

  return 0;
}
