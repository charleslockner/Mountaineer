CC=g++
EXE=dae_to_ciab
LIBDIR=../lib

OPTLEVEL=-O1
WARN=
INCLUDES=-Iinclude
#-I$(LIBDIR)/include

CFLAGS=-c $(INCLUDES) $(WARN) $(OPTLEVEL) -g

SRC=dae_to_ciab.cpp
OBJ=dae_to_ciab.o
LIBS=libassimp.3.1.1.dylib

.PHONY: exe run clean

exe: $(EXE)

run: $(EXE)
	./$(EXE) trex_walk.dae ../assets/models/trex.ciab

clean:
	rm -f $(EXE) $(OBJ)

$(EXE): $(OBJ)
	$(CC) -o $(EXE) $(OBJ) $(LIBS)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<