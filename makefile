#Original framework from
#https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

CC=gcc
CFLAGS= -ggdb3  -Wall -I $(IDIR)

BIN=pageSim
SDIR=src
BDIR=bin
LDIR=$(SDIR)/lib
ODIR=$(SDIR)/obj
IDIR=$(SDIR)/inc 

SRC_FILES = $(wildcard $(SDIR)/*.c)
OBJ =	$(patsubst $(SDIR)/%.c, $(ODIR)/%.o, $(SRC_FILES) ) 

_DEPS = 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

$(ODIR)/%.o: $(SDIR)/%.c $(wildcard $(IDIR)/*.hpp)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BDIR)/$(BIN): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o core $(BDIR)/*

