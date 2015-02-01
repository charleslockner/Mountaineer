CC=g++
NAME=dae_to_ciab

BINDIR=../bin
OBJDIR=../obj
LIBDIR=../lib

EXE=$(BINDIR)/$(NAME)

OPTLEVEL=-O1
WARN=
INCLUDES=-Iinclude
#-I$(LIBDIR)/include

CFLAGS=-c $(INCLUDES) $(WARN) $(OPTLEVEL) -g

SRC=dae_to_ciab.cpp
OBJ=$(OBJDIR)/dae_to_ciab.o
LIBS=libassimp.3.1.1.dylib

.PHONY: exe run clean

exe: $(EXE)

run: $(EXE)
	./$(EXE) ../modeling_files/exports/robot.dae ../assets/models/robot.ciab

clean:
	rm -rf $(BINDIR)
	rm -rf $(OBJDIR)

$(EXE): $(OBJ)
	$(CC) -o $(EXE) $(OBJ) $(LIBS)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<