CC=g++
NAME=dae_to_ciab
EXE=$(NAME)

OBJDIR=obj

OPTLEVEL=-O1
WARN=
INCLUDES=-Iinclude

CFLAGS=-c $(INCLUDES) $(WARN) $(OPTLEVEL) -g

SRC=dae_to_ciab.cpp
OBJ=$(OBJDIR)/dae_to_ciab.o
LIBS=libassimp.3.1.1.dylib

.PHONY: exe run clean

exe: $(EXE)

run: $(EXE)
	./$(EXE) ../modeling_files/exports/skybox.dae ../assets/models/skybox.ciab

clean:
	rm -rf $(EXE)
	rm -rf $(OBJDIR)

$(EXE): $(OBJ)
	@mkdir -p $(@D)
	$(CC) -o $(EXE) $(OBJ) $(LIBS)

$(OBJ): $(SRC)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<