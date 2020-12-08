#Just realized that this has to be written in python and not C.
#Student: Hayden Coffey
#ECE 517: Final Project, Python Version
import math
import sys

history = 0
oracle = 1
rl = 2

MAX_PAGES = 1024
HIT_DIV = 100
HIT_CAP = 10000

EPOCHS = 10
EPOCHS_RAN = 0

total_physpages = None
total_virtpages = None
m1_pages = None
m2_pages = None
m1_delay = None
m2_delay = None

ps_count = None
selected_pages = None
sp_states = None
sp_qval = None

epoch_intv = None

page_size = None
addr_size = None
tlb_line_shift = None
addr_mask = None

page_faults = 0
diskrefs = 0
page_hits = 0
num_pages_ref = 0
oracle_time = 0

FIRST = True


class page_record:
    benefit = 0
    vpn = 0


class Page_table:
    phypage = 0 
    resident = 0 
    dirty = 0 
    mispredict = 0 
    total_hits = 0 
    chosen_index = 0 


class phys_page:
    virtpage = 0 
    lru = 0 
    epoch_hits = 0 


page_table = []
phys_pages = []
ORACLE_HITS = [0] * MAX_PAGES

def getCmdOption(args, option):
    try:
        i = args.index(option)
        if i != len(args) and i+1 != len(args):
            return args[i+1]
    except ValueError:
        pass
    return -1


def power_of_two(val):
    return (val & (val-1) == 0) and val != 0



def reset_lru():
    for i in range(total_physpages):
        phys_pages[i].lru = 0


def reset_page_hits():
    for i in range(total_physpages):
        phys_pages[i].epoch_hits = 0


def update_page_LRU(page):
    for i in range(total_physpages):
        phys_pages[i].lru += 1
    phys_pages[page].lru = 0


def proc_page_lookup(access_type, address, page):
    global page_hits, page_faults, diskrefs, num_pages_ref
    virtpage = 0
    k = 0
    j = 0
    lru = 0
    hit = 0

    virtpage = address >> tlb_line_shift

    if page_table[virtpage].resident:
        page[0] = page_table[virtpage].phypage
        page_hits += 1
        hit = True
    else:
        hit = False
        page_faults += 1
        diskrefs += 1

        if num_pages_ref < total_physpages:
            page_table[virtpage].phypage = num_pages_ref
            num_pages_ref += 1
            phys_pages[page_table[virtpage].phypage].virtpage = virtpage
        else:
            lru = 0
            for i in range(m1_pages, total_physpages):
                if phys_pages[i].lru >= lru:
                    lru = phys_pages[i].lru
                    j = i

            for i in range(total_virtpages):
                if page_table[i].resident and page_table[i].phypage == j:
                    break
                k += 1
            if k == total_virtpages:
                print("Could not find physical page in page table")

            if page_table[k].dirty:
                diskrefs += 1

            page_table[virtpage].phypage = j
            phys_pages[j].virtpage = virtpage
            phys_pages[j].epoch_hits = 0
            page_table[k].resident = False

        page_table[virtpage].resident = True
        page_table[virtpage].dirty = False

        page[0] = page_table[virtpage].phypage

    update_page_LRU(page[0])

    return hit


def read_config(fileName):
    global total_virtpages, total_physpages, m1_pages
    global m1_delay, m2_pages, m2_delay, page_size, addr_size
    global epoch_intv, ps_count, tlb_line_shift, addr_mask

    lines = None
    with open(fileName, "r") as f:
        lines = f.readlines()

    for i in range(len(lines)):
        lines[i] = (lines[i].strip()).split(":")

    total_virtpages = int(lines[0][1])
    total_physpages = int(lines[1][1])
    m1_pages = int(lines[2][1])
    m1_delay = int(lines[3][1])
    m2_pages = int(lines[4][1])
    m2_delay = int(lines[5][1])

    if m1_pages + m2_pages != total_physpages:
        print("Memory partition page counts sum does not match total.")
        return -1

    page_size = int(lines[6][1])

    if not power_of_two(page_size):
        print("Page size is not a power of 2.")
        return -1

    tlb_line_shift = int(math.log2(page_size))

    addr_size = int(lines[7][1])

    if not power_of_two(addr_size):
        print("Address size is not a power of 2.")
        return -1

    addr_mask = (1 << addr_size) - 1

    epoch_intv = int(lines[8][1])

    ps_count = int(lines[9][1])

    return 0

def schedule_epoch(n):
    pass

def page_selector(fileName):
    pass

def reset_pages(n):
    pass

def load_model(fileName):
    pass

def save_model(fileName):
    pass

def init_arrays():
    for _ in range(MAX_PAGES):
        phys_pages.append(phys_page())
        page_table.append(Page_table())


def main():
    global EPOCHS, EPOCHS_RAN
    time = 0
    init_arrays()

    configFileName = getCmdOption(sys.argv, "-c")
    if configFileName == -1:
        print("No config file specified")
        return 1

    traceFileName = getCmdOption(sys.argv, "-t")
    if traceFileName == -1:
        print("No trace file specified")
        return 1

    scheduler = int(getCmdOption(sys.argv, "-s"))
    if scheduler == -1:
        print("No scheduler specified")
        return 1

    if read_config(configFileName) != 0:
        print("Failed to read config file.")
        return 1
    
    if scheduler == rl:
        epochCount = int(getCmdOption(sys.argv, "-e"))
        saveModelName = getCmdOption(sys.argv, "-SM")
        loadModelName = getCmdOption(sys.argv, "-LM")
        
        if loadModelName != -1:
            load_model(loadModelName)

        if epochCount != -1:
            EPOCHS = epochCount

        page_selector("benefit.log")

    epoch = 0
    while True:

        cycle = 0
        time = 0
        page_hits = 0
        page_faults = 0
        reset_pages(scheduler)

        with open(traceFileName, "r") as f:
            for line in f:
                line = (line.strip()).split(' ')
                line[0] = line[0].strip(':')
                accessType = line[1]
                memAddr = int(line[2], 0)

                page = [0]

                if cycle >= epoch_intv:
                    schedule_epoch(scheduler)
                    cycle = 0
                
                memAddr &= addr_mask
                proc_page_lookup(accessType, memAddr, page)
                phys_pages[page[0]].epoch_hits += 1
                page_table[phys_pages[page[0]].virtpage].total_hits += 1

                if page[0] < m1_pages:
                    time += m1_delay
                else:
                    time += m2_delay
                
                cycle += 1

        print("STATISTICS\n")
        print("{:<16} {:9}".format("Page Hits", page_hits))
        print("{:<16} {:9}".format("Page Faults", page_faults))
        if scheduler != oracle:
            print("{:<16} {:9}".format("Time", time))
        else:
            schedule_epoch(oracle)
            print("{:<16} {:9}".format("Oracle Time", time))

        epoch += 1
        EPOCHS_RAN += 1
        if scheduler != rl or epoch >= EPOCHS:
            break
    
    if scheduler == rl:
        if saveModelName != -1:
            save_model(saveModelName)

if __name__ == "__main__":
    main()