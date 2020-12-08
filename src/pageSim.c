#include "CommandLine.h"
#include "rl.h"
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/cdefs.h>

#define MAX_PAGES 1024
#define HIT_DIV 100
#define HIT_CAP 10000
#define TRUE 1
#define FALSE 0

typedef unsigned int uint;
typedef unsigned long ulong;

typedef enum SCHEDULER { history, oracle, rl } SCHEDULER;

ulong EPOCHS = 10;
ulong EPOCHS_RAN = 0;

uint total_physpages;
uint total_virtpages;
uint m1_pages;
uint m2_pages;
uint m1_delay;
uint m2_delay;

uint ps_count;
ulong *selected_pages = NULL;
state *sp_states = NULL;
qvalue *sp_qval = NULL;

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

typedef struct page_record {
  ulong benefit;
  ulong vpn;
} page_record;

struct {
  unsigned long phypage;   /* physical page number               */
  unsigned short resident; /* is page resident in memory         */
  unsigned short dirty;    /* has page been updated since loaded */
  ulong mispredict;
  ulong total_hits;
  unsigned int chosen_index;
} page_table[MAX_PAGES];

typedef struct phys_page {
  // uint index;
  ulong virtpage;
  ulong lru;
  ulong epoch_hits;
} phys_page;

phys_page phys_pages[MAX_PAGES];
ulong ORACLE_HITS[MAX_PAGES];

// Taken and modified from: https://www.geeksforgeeks.org/quick-sort/
// A utility function to swap two elements
void swap(phys_page *a, phys_page *b) {
  phys_page t = *a;
  *a = *b;
  *b = t;
}
void swap_pr(page_record *a, page_record *b) {
  page_record t = *a;
  *a = *b;
  *b = t;
}
#if 1
/* This function takes last element as pivot, places
   the pivot element at its correct position in sorted
    array, and places all smaller (smaller than pivot)
   to left of pivot and all greater elements to right
   of pivot */
int partition(phys_page *arr, int low, int high) {
  int pivot = arr[high].epoch_hits; // pivot
  int i = (low - 1);                // Index of smaller element

  for (int j = low; j <= high - 1; j++) {
    // If current element is smaller than the pivot
    if (arr[j].epoch_hits > pivot) {
      i++; // increment index of smaller element
      swap(&arr[i], &arr[j]);
    }
  }
  swap(&arr[i + 1], &arr[high]);
  return (i + 1);
}

int partition_pr(page_record *arr, int low, int high) {
  ulong pivot = arr[high].benefit; // pivot
  int i = (low - 1);               // Index of smaller element

  for (int j = low; j <= high - 1; j++) {
    // If current element is smaller than the pivot
    if (arr[j].benefit > pivot) {
      i++; // increment index of smaller element
      swap_pr(&arr[i], &arr[j]);
    }
  }
  swap_pr(&arr[i + 1], &arr[high]);
  return (i + 1);
}

/* The main function that implements QuickSort
 arr[] --> Array to be sorted,
  low  --> Starting index,
  high  --> Ending index */
void quickSort(phys_page *arr, int low, int high) {
  if (low < high) {
    /* pi is partitioning index, arr[p] is now
       at right place */
    int pi = partition(arr, low, high);

    // Separately sort elements before
    // partition and after partition
    quickSort(arr, low, pi - 1);
    quickSort(arr, pi + 1, high);
  }
}

void quickSort_pr(page_record *arr, int low, int high) {
  if (low < high) {
    /* pi is partitioning index, arr[p] is now
       at right place */
    int pi = partition_pr(arr, low, high);

    // Separately sort elements before
    // partition and after partition
    quickSort_pr(arr, low, pi - 1);
    quickSort_pr(arr, pi + 1, high);
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
    phys_pages[i].epoch_hits = 0;
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
    if (num_pages_ref < total_physpages) {
      page_table[virtpage].phypage = num_pages_ref++;
      phys_pages[page_table[virtpage].phypage].virtpage = virtpage;
    } else {

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
        // printf("%d\n", j);
        // exit(1);
      }

      /* if the page is dirty, then increment the number of disk
         references */
      if (page_table[i].dirty)
        diskrefs++;

      /* update information for the newly resident virtual page */
      page_table[virtpage].phypage = j;
      phys_pages[j].virtpage = virtpage;
      phys_pages[j].epoch_hits = 0;
      page_table[i].resident = FALSE;
      // phys_pages[page_table[i].phypage].virtpage = 0;
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

  if (fscanf(f, "Page Select:\t%u\n", &ps_count) != 1) {
    fprintf(stderr, "Failed to read in page select count.\n");
    return -1;
  }

  fclose(f);

  return 0;
}

void history_scheduler() {
  int tmp_phypage;
  quickSort(phys_pages, 0, total_physpages - 1);
  for (int i = 0; i < total_physpages; i++) {

    // Calculate mispredictions
    tmp_phypage = page_table[phys_pages[i].virtpage].phypage;
    if ((tmp_phypage < m1_pages && i >= m1_pages) ||
        (tmp_phypage >= m1_pages && i < m1_pages)) {
      page_table[phys_pages[i].virtpage].mispredict++;
    }

    // Update physpage number
    page_table[phys_pages[i].virtpage].phypage = i;
  }
}

void schedule_epoch(enum SCHEDULER n) {
  ulong hits;
  int m1_c = 0, m2_c = m1_pages;
  int ps_index;
  int ps_epoch = 0;
  int j = 0;
  int z = 0;
  int matched = FALSE;
  long epoch_delay = 0;
  phys_page *phys_pages_buf;
  phys_page *selec_page_buf;

  switch (n) {
    // History Page Schedule    state x;r
  case history:
    history_scheduler();
    break;

  // Oracle Scheduler, best case scenario
  case oracle:
    quickSort(phys_pages, 0, total_physpages - 1);
    for (int i = 0; i < total_physpages; i++) {
      page_table[phys_pages[i].virtpage].phypage = i;

      if (i < m1_pages) {
        oracle_time += m1_delay * phys_pages[i].epoch_hits;
      } else {
        oracle_time += m2_delay * phys_pages[i].epoch_hits;
      }
    }
    break;

  // Hopefully we do some RL here?
  case rl:

    // Calculate delay over past epoch
    for (int page = 0; page < total_physpages; page++) {
      if (page < m1_pages) {
        epoch_delay += m1_delay * phys_pages[page].epoch_hits;
      } else {
        epoch_delay += m2_delay * phys_pages[page].epoch_hits;
      }
    }

    // Calculate how many selected pages are present
    for (int i = 0; i < total_physpages; i++) {

      matched = FALSE;
      for (int k = 0; k < ps_count; k++) {
        if (selected_pages[k] == phys_pages[i].virtpage) {
          matched = TRUE;
          ps_index = k;
          break;
        }
      }

      if (matched) {
        ps_epoch++;
      }
    }

    // printf("%d %d\n", ps_epoch, ps_count);
    // Allocate buffer, copy over pages not chosen, and sort
    phys_pages_buf = malloc(sizeof(phys_page) * (total_physpages - ps_epoch));
    selec_page_buf = malloc(sizeof(phys_page) * ps_epoch);

    j = z = 0;
    for (int i = 0; i < total_physpages; i++) {

      matched = FALSE;
      for (int k = 0; k < ps_count; k++) {
        if (selected_pages[k] == phys_pages[i].virtpage) {
          matched = TRUE;
          selec_page_buf[z] = phys_pages[i];
          page_table[selec_page_buf[z].virtpage].chosen_index = k;
          z++;
          break;
        }
      }

      if (matched) {
        continue;
      }

      phys_pages_buf[j] = phys_pages[i];
      j++;
    }

    // Sort non-selected pages
    quickSort(phys_pages_buf, 0, total_physpages - 1 - ps_epoch);

#if 1
    // Make decisions for selected pages
    for (int i = 0; i < ps_epoch; i++) {
      // printf("ps epoch : %d\n", ps_epoch);
      ps_index = page_table[selec_page_buf[i].virtpage].chosen_index;
      hits = selec_page_buf[i].epoch_hits;
      if (hits > HIT_CAP) {
        hits = HIT_CAP - 1;
      }

      // New state
      // printf("pind : %d\n", ps_index+ps_count);
      sp_states[ps_index + ps_count].p1_hits =
          sp_states[ps_index + ps_count].hits;
      sp_states[ps_index + ps_count].hits = hits / HIT_DIV;
      sp_states[ps_index + ps_count].old_device =
          page_table[selected_pages[ps_index]].phypage / m1_pages;
      // printf("Old page: %d\n", page_table[selected_pages[ps_index]].phypage);
      sp_states[ps_index + ps_count].new_device =
          rl_schedule_page(&sp_states[ps_index], &sp_qval[ps_index]);

#if 1
      updateQValue(ps_index, ps_count, sp_states, sp_qval, -epoch_delay);
#endif

      // Update old State
      sp_states[ps_index] = sp_states[ps_index + ps_count];
#if 1
      if (sp_states[ps_index].new_device == m1) {
        phys_pages[m1_c] = selec_page_buf[i];
        page_table[selected_pages[ps_index]].phypage = m1_c;
        m1_c++;
      } else {
        phys_pages[m2_c] = selec_page_buf[i];
        page_table[selected_pages[ps_index]].phypage = m2_c;
        m2_c++;
      }
    }

    // Fill in remaining pages
    for (int i = 0; i < total_physpages - ps_epoch; i++) {
      if (m1_c < m1_pages) {
        phys_pages[m1_c] = phys_pages_buf[i];

        // Update physpage number
        // printf("%d %lx\n", m1_c, phys_pages[m1_c].virtpage);
        page_table[phys_pages[m1_c].virtpage].phypage = m1_c;
        m1_c++;
      } else {
        phys_pages[m2_c] = phys_pages_buf[i];

        // Update physpage number
        page_table[phys_pages[m2_c].virtpage].phypage = m2_c;
        m2_c++;
      }
    }
#endif
#endif
    free(phys_pages_buf);
    free(selec_page_buf);
    break;
  }

#if 0
  for(int i = 0; i < total_physpages; i++)
  {
      matched = FALSE;
      for (int k = 0; k < ps_count; k++) {
        if (selected_pages[k] == phys_pages[i].virtpage) {
          matched = TRUE;
          break;
        }
      }

      if (matched) {
        printf("*");
      }
    printf("%lu\n", phys_pages[i].epoch_hits);
  }
  exit(1);
#endif

  reset_lru();
  reset_page_hits();
}

void page_selector(char *fileName) {

  if (ps_count == 0) {
    return;
  }
  // Read in benefit data
  FILE *f = fopen(fileName, "rb");
  ulong *benefitArray = malloc(sizeof(ulong) * total_virtpages);
  fread(benefitArray, total_virtpages, sizeof(ulong), f);
  fclose(f);
  #if 0
  for(int i = 0; i < total_virtpages; i++)
  {
    printf("%lu\n", benefitArray[i]);
  }
  exit(1);
  #endif

  // Associate benefit data with VPNs
  struct page_record *records =
      malloc(sizeof(struct page_record) * total_virtpages);
  for (int i = 0; i < total_virtpages; i++) {
    records[i].benefit = benefitArray[i];
    records[i].vpn = i;
  }

  sp_states = malloc(sizeof(state) * ps_count * 2);

  quickSort_pr(records, 0, total_virtpages - 1);

  if (selected_pages == NULL || sp_qval == NULL) {
    selected_pages = malloc(sizeof(ulong) * ps_count);
    sp_qval = malloc(sizeof(qvalue) * ps_count);
    for (int i = 0; i < ps_count; i++) {
      selected_pages[i] = records[i].vpn;

      // Initialize Q values for each page
      sp_qval[i].Q = malloc(sizeof(double) * (HIT_CAP / HIT_DIV) *
                            (HIT_CAP / HIT_DIV) * 2 * 2);
      sp_qval[i].x0 = (HIT_CAP / HIT_DIV);
      sp_qval[i].x1 = (HIT_CAP / HIT_DIV);
      sp_qval[i].y = 2;
      sp_qval[i].z = 2;
    }
  }
#if 0
   printf("Total elem: %lu\n", sp_qval[0].x * sp_qval[0].y * sp_qval[0].z);
   exit(1);
#endif
#if 0
  for (int i = 0; i < total_virtpages; i++) {
    printf("%16lu : %16lu\n", records[i].vpn, records[i].benefit);
  }
  exit(1);
#endif

  free(benefitArray);
  free(records);
}

void reset_pages(int scheduler) {
  num_pages_ref = 0;
  for (int i = 0; i < total_virtpages; i++) {
    page_table[i].phypage = 0;
    page_table[i].chosen_index = 0;
    page_table[i].dirty = 0;
    page_table[i].mispredict = 0;
    page_table[i].resident = 0;
    page_table[i].total_hits = 0;
  }
  for (int i = 0; i < total_physpages; i++) {
    phys_pages[i].epoch_hits = 0;
    phys_pages[i].lru = 0;
    phys_pages[i].virtpage = 0;
  }

  if (scheduler == rl) {
    for (int i = 0; i < ps_count; i++) {
      sp_states[i].p1_hits = 0;
      sp_states[i].hits = 0;
      sp_states[i].new_device = 0;
      sp_states[i].old_device = 0;
    }
  }
}

void save_model(char *fileName) {
  FILE *f = NULL;
  f = fopen(fileName, "wb");
  if (f == NULL) {
    fprintf(stderr, "Failed to open model file:%s.\n", fileName);
    return;
  }

  // Write header info
  fwrite(&ps_count, 1, sizeof(uint), f);
  fwrite(&EPOCHS_RAN, 1, sizeof(ulong), f);

  fwrite(selected_pages, ps_count, sizeof(ulong), f);

  for (int i = 0; i < ps_count; i++) {
    fwrite(sp_qval + i, 1, sizeof(qvalue), f);
    fwrite(sp_qval[i].Q,
           sp_qval[i].x0 * sp_qval[i].x1 * sp_qval[i].y * sp_qval[i].z,
           sizeof(double), f);
  }

  fclose(f);
}

int load_model(char *fileName) {
  uint tmp_ps_count;

  FILE *f = NULL;
  f = fopen(fileName, "rb");
  if (f == NULL) {
    fprintf(stderr, "Failed to open model file.\n");
    return -1;
  }

  fread(&tmp_ps_count, 1, sizeof(uint), f);
  fread(&EPOCHS_RAN, 1, sizeof(ulong), f);

  if (tmp_ps_count != ps_count) {
    fprintf(stderr, "ERROR: Model's selected page does not match config.\n");
    fprintf(stderr, "MODEL: %u CONFIG: %u.\n", tmp_ps_count, ps_count);
    return -1;
  }

  selected_pages = malloc(sizeof(ulong) * ps_count);
  sp_qval = malloc(sizeof(qvalue) * ps_count);

  fread(selected_pages, ps_count, sizeof(ulong), f);

  for (int i = 0; i < ps_count; i++) {
    fread(sp_qval + i, 1, sizeof(qvalue), f);

    sp_qval[i].Q = malloc(sizeof(double) * (HIT_CAP / HIT_DIV) *
                          (HIT_CAP / HIT_DIV) * 2 * 2);

    fread(sp_qval[i].Q,
          sp_qval[i].x0 * sp_qval[i].x1 * sp_qval[i].y * sp_qval[i].z,
          sizeof(double), f);
  }

  fclose(f);

  printf("LOADED MODEL:\n");
  printf("PS COUNT: %u\n", tmp_ps_count);
  printf("EPOCHS RAN: %lu\n", EPOCHS_RAN);
}
/*
 * main - drives the cache simulator
 */
int main(int argc, char **argv) {
  FILE *f = NULL, *benLog;
  ulong instAddr, memAddr;
  uint cycle = 0;
  ulong time = 0;
  ulong epoch = 0;
  uint page;
  char accessType;
  SCHEDULER scheduler;

  char *configFileName;
  char *traceFileName;
  char *schedulerArg;
  char *saveModelName;
  char *loadModelName;
  char *epochCount;

  configFileName = getCmdOption(argv, argc, "-c");
  if (configFileName == 0) {
    fprintf(stderr, "No config file specified.\n");
    return 1;
  }

  traceFileName = getCmdOption(argv, argc, "-t");
  if (configFileName == 0) {
    fprintf(stderr, "No trace file specified.\n");
    return 1;
  }

  schedulerArg = getCmdOption(argv, argc, "-s");
  if (schedulerArg == 0) {
    fprintf(stderr, "No scheduler specified.\n");
    return 1;
  }

  scheduler = atoi(schedulerArg);

  if (read_config(configFileName) != 0) {
    return 1;
  }

  if (scheduler == rl) {
    saveModelName = getCmdOption(argv, argc, "-SM");
    loadModelName = getCmdOption(argv, argc, "-LM");
    epochCount = getCmdOption(argv, argc, "-e");
    if (loadModelName != 0) {
      load_model(loadModelName);
    }

    if (epochCount != 0) {
      EPOCHS = atoi(epochCount);
    }

    page_selector("benefit.log");
  }

  do {
    f = fopen(traceFileName, "r");
    if (f == NULL) {
      printf("Failed to open trace file\n");
      return 1;
    }

    cycle = 0;
    time = 0;
    page_hits = 0;
    page_faults = 0;
    reset_pages(scheduler);

    int fret;
    while (TRUE) {
      fret = (fscanf(f, "%lx: %c %lx\n", &instAddr, &accessType, &memAddr));
      // printf("fret : %d\n", fret);
      if (fret == 0 || fret == EOF) {
        break;
      }

      if (cycle >= epoch_intv) {
        schedule_epoch(scheduler);
        // printf("%d\n", i);
        cycle = 0;
      }

      memAddr &= addr_mask;
      proc_page_lookup(accessType, memAddr, &page);
      phys_pages[page].epoch_hits++;
      page_table[phys_pages[page].virtpage].total_hits++;

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
    if (scheduler != oracle) {
      printf("%-16s %9lu\n", "Time", time);
    } else {
      schedule_epoch(oracle);
      printf("%-16s %9lu\n", "Oracle Time", oracle_time);
    }

#if 0
  benLog = fopen("benefit.log", "wb");
  for (int i = 0; i < total_virtpages; i++) {
    ulong tmp = (page_table[i].mispredict * page_table[i].total_hits);
    //    printf("%lu\n", tmp);
    fwrite(&tmp, sizeof(ulong), 1, benLog);
  }
  fclose(benLog);
#endif
    fclose(f);
    epoch++;
    EPOCHS_RAN++;
  } while (scheduler == rl && epoch < EPOCHS);

  if (scheduler == rl) {
    if (saveModelName != 0) {
      save_model(saveModelName);
    }
  }

  if (selected_pages != NULL) {
    free(selected_pages);
    /*for (int i = 0; i < ps_count; i++) {
      // Initialize Q values for each page
      free(sp_qval[i].Q);
    }
    free(sp_states);
    free(sp_qval);*/
  }

  return 0;
}
