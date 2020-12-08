#Just realized that this has to be written in python and not C.
#Student: Hayden Coffey
#ECE 517: Final Project, Python Version
import math
import copy
import random
import sys
from array import array

history = 0
oracle = 1
rl = 2

#Temporal Difference=========================================
lr = 0.1
discount = 0.9
epsilon = 100
m1 = 0
m2 = 1


class state:
    old_device = 0
    new_device = 0
    hits = 0
    p1_hits = 0


class qvalue:
    Q = []
    x0 = 0
    x1 = 0
    y = 0
    z = 0


def setQValue(s, Q, update):
    Q.Q[s.hits + s.p1_hits * Q.x0 + s.old_device * Q.x0 * Q.x1 +
        s.new_device * Q.x0 * Q.x1 * Q.y] = update


def getQValue(s, Q):
    return Q.Q[s.hits + s.p1_hits * Q.x0 + s.old_device * Q.x0 * Q.x1 +
               s.new_device * Q.x0 * Q.x1 * Q.y]


def updateQValue(index, ps_count, s, Q, reward):
    q = getQValue(s[index], Q[index])
    q_new = getQValue(s[index + ps_count], Q[index])
    update = q + lr * ((1.0 * reward) + discount * q_new - q)

    setQValue(s[index], Q[index], update)


def getAction(s, Q):
    tmp = copy.deepcopy(s)
    tmp.new_device = 0
    m1_q = getQValue(tmp, Q)
    tmp.new_device = 1
    m2_q = getQValue(tmp, Q)

    r = random.randint(0, 10000 - 1)

    if r >= epsilon:
        if m1_q > m2_q:
            return m1
        else:
            return m2
    else:
        return r % 2


def rl_schedule_page(s, Q):
    return getAction(s, Q)


#Page Simulation=============================================
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
        if i != len(args) and i + 1 != len(args):
            return args[i + 1]
    except ValueError:
        pass
    return -1


def power_of_two(val):
    return (val & (val - 1) == 0) and val != 0


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


def history_scheduler():
    tmp_phypage = 0
    phys_pages.sort(key=lambda x: x.epoch_hits, reverse=True)

    for i in range(total_physpages):
        tmp_phypage = page_table[phys_pages[i].virtpage].phypage
        if (tmp_phypage < m1_pages
                and i >= m1_pages) or (tmp_phypage >= m1_pages
                                       and i < m1_pages):
            page_table[phys_pages[i].virtpage].mispredict += 1

        page_table[phys_pages[i].virtpage].phypage = i


def schedule_epoch(n):
    global oracle_time, m1_delay, m2_delay
    hits = 0
    m1_c = 0
    m2_c = m1_pages
    ps_index = 0
    ps_epoch = 0
    j = 0
    z = 0
    matched = False
    epoch_delay = 0

    if n == history:
        history_scheduler()
    elif n == oracle:
        phys_pages.sort(key=lambda x: x.epoch_hits, reverse=True)
        for i in range(total_physpages):

            page_table[phys_pages[i].virtpage].phypage = i
            if i < m1_pages:
                oracle_time += m1_delay * phys_pages[i].epoch_hits
            else:
                oracle_time += m2_delay * phys_pages[i].epoch_hits
    elif n == rl:
        #Calculate delay over past epoch
        for page in range(total_physpages):
            if page < m1_pages:
                epoch_delay += m1_delay * phys_pages[page].epoch_hits
            else:
                epoch_delay += m2_delay * phys_pages[page].epoch_hits

        #Calculate how many selected pages are present
        #TODO: Can we over allocate instead?
        for i in range(total_physpages):
            matched = False
            for k in range(ps_count):
                if selected_pages[k] == phys_pages[i].virtpage:
                    matched = True
                    ps_index = k
                    break

            if matched:
                ps_epoch += 1

        phys_pages_buf = []
        for i in range(total_physpages - ps_epoch):
            phys_pages_buf.append(phys_page())

        selec_page_buf = []
        for i in range(ps_epoch):
            selec_page_buf.append(phys_page())

        j = z = 0
        for i in range(total_physpages):
            matched = False

            for k in range(ps_count):
                if selected_pages[k] == phys_pages[i].virtpage:
                    matched = True
                    selec_page_buf[z] = phys_pages[i]
                    page_table[selec_page_buf[z].virtpage].chosen_index = k
                    z += 1

            if matched:
                continue

            phys_pages_buf[j] = phys_pages[i]
            j += 1

        #Sort non-selected pages
        phys_pages_buf.sort(key=lambda x: x.epoch_hits, reverse=True)

        #Make decisions for selected pages
        for i in range(ps_epoch):
            ps_index = page_table[selec_page_buf[i].virtpage].chosen_index
            hits = selec_page_buf[i].epoch_hits
            if hits > HIT_CAP:
                hits = HIT_CAP - 1

            #Calculate new state
            sp_states[ps_index + ps_count].p1_hits = int(
                sp_states[ps_index + ps_count].hits)
            sp_states[ps_index + ps_count].hits = int(int(hits) / int(HIT_DIV))
            sp_states[ps_index + ps_count].old_device = int(
                page_table[selected_pages[ps_index]].phypage / m1_pages)
            sp_states[ps_index + ps_count].new_device = int(
                rl_schedule_page(sp_states[ps_index], sp_qval[ps_index]))

            updateQValue(ps_index, ps_count, sp_states, sp_qval, -epoch_delay)

            #Update old state
            sp_states[ps_index] = sp_states[ps_index + ps_count]

            #If action is assigning to m1
            if sp_states[ps_index].new_device == m1:
                phys_pages[m1_c] = (selec_page_buf[i])
                page_table[selected_pages[ps_index]].phypage = m1_c
                m1_c += 1
            #Else assign to m2
            else:
                phys_pages[m2_c] = (selec_page_buf[i])
                page_table[selected_pages[ps_index]].phypage = m2_c
                m2_c += 1

        #Fill in remaining pages
        for i in range(total_physpages - ps_epoch):
            if m1_c < m1_pages:
                phys_pages[m1_c] = (phys_pages_buf[i])
                page_table[phys_pages[m1_c].virtpage].phypage = m1_c
                m1_c += 1
            else:
                phys_pages[m2_c] = (phys_pages_buf[i])
                page_table[phys_pages[m2_c].virtpage].phypage = m2_c
                m2_c += 1
    reset_lru()
    reset_page_hits()


def page_selector(fileName):
    global ps_count, sp_states, sp_qval, selected_pages
    if ps_count == 0:
        return

    ELEM_SIZE = 8
    with open(fileName, "rb") as f:
        benefitArrayRaw = f.read()
    benefitArray = [
        benefitArrayRaw[s:s + ELEM_SIZE]
        for s in range(0, len(benefitArrayRaw), ELEM_SIZE)
    ]

    records = []
    for i in range(total_virtpages):
        records.append(page_record())
        records[i].benefit = int.from_bytes(benefitArray[i], "little")
        records[i].vpn = i

    sp_states = []
    for i in range(ps_count * 2):
        sp_states.append(state())

    records.sort(key=lambda x: x.benefit, reverse=True)

    #for i in records:
    #    print(i.vpn, ":", i.benefit)
    
    #exit(1)

    if selected_pages == None or sp_qval == None:
        selected_pages = [0] * ps_count
        sp_qval = []
        for i in range(ps_count):
            sp_qval.append(qvalue())
            selected_pages[i] = records[i].vpn

            sp_qval[i].Q = [0.0] * int(
                int(HIT_CAP / HIT_DIV) * int(HIT_CAP / HIT_DIV) * 2 * 2)
            sp_qval[i].x0 = int(HIT_CAP / HIT_DIV)
            sp_qval[i].x1 = int(HIT_CAP / HIT_DIV)
            sp_qval[i].y = 2
            sp_qval[i].z = 2


def reset_pages(scheduler):
    global num_pages_ref
    num_pages_ref = 0
    for i in range(total_virtpages):
        page_table[i].phypage = 0
        page_table[i].chosen_index = 0
        page_table[i].dirty = 0
        page_table[i].mispredict = 0
        page_table[i].resident = 0
        page_table[i].total_hits = 0

    for i in range(total_physpages):
        phys_pages[i].epoch_hits = 0
        phys_pages[i].lru = 0
        phys_pages[i].virtpage = 0

    #TODO Add once rl is written
    if scheduler == rl:
        for i in range(ps_count):
            sp_states[i].p1_hits = 0
            sp_states[i].hits = 0
            sp_states[i].new_device = 0
            sp_states[i].old_device = 0


def load_model(fileName):
    pass


def save_model(fileName):
    pass


def init_arrays():
    for _ in range(MAX_PAGES):
        phys_pages.append(phys_page())
        page_table.append(Page_table())


def main():
    global EPOCHS, EPOCHS_RAN, page_hits, page_faults
    global oracle_time
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
            print("{:<16} {:9}".format("Oracle Time", oracle_time))

        epoch += 1
        EPOCHS_RAN += 1
        if scheduler != rl or epoch >= EPOCHS:
            break

    if scheduler == rl:
        if saveModelName != -1:
            save_model(saveModelName)


if __name__ == "__main__":
    main()