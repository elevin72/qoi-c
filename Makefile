
CC=gcc
INCL = include
CFLAGS = -I$(INCL) -Wall -pedantic
SRC = src
LDIR = lib
DEPS = $(INCL)/qoi.h
LFLAGS=-lm -lz
ODIR=build

_OBJ=main.o encoder.o decoder.o constants.o spng.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

qoi: $(OBJ)
	$(CC) -o qoi $^ $(LFLAGS)

build/spng.o: $(LDIR)/spng.c $(LDIR)/spng.h
	$(CC) -c -o $@ $<

$(ODIR)/%.o: $(SRC)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(GTKFLAGS) $(GTKLIBFLAGS)

# test: qoi
