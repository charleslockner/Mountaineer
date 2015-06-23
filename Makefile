OS := $(shell uname -s)
CC=icpc
ENGINE_NAME=libengine.a

BIN_DIR=bin
SRC_DIR=src
OBJ_DIR=obj
LIB_DIR=lib
INC_DIR=$(SRC_DIR)/include

ENGINE=$(BIN_DIR)/$(ENGINE_NAME)
GAME=$(BIN_DIR)/game

INC=-I$(INC_DIR) -I$(LIB_DIR)/include -I$(LIB_DIR)/include/eigen -I$(LIB_DIR)/include/ceres/internal/miniglog
HEADER=-DMACOSX -MMD
DEBUG=-g
OPT=-O3
WARN=-ansi -pedantic #-w3 -wn383 -wn1418 -wn304
CFLAGS=-std=c++11 -c $(INC) $(WARN) $(OPT) $(DEBUG) $(HEADER)

SRC=$(shell find $(SRC_DIR) -maxdepth 1 -type f -name "*.cpp" -exec basename {} .po \;)
OBJS=$(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC))

.PHONY: ENGINE game terrain rigid run test clean

engine: $(ENGINE)

terrain:
	make -C terrain
	./$(BIN_DIR)/terrain

rigid:
	make -C rigid
	./$(BIN_DIR)/rigid

game: $(ENGINE)
	make -C game

run: $(ENGINE)
	make -C game
	./$(GAME)

test:
	make -C test run

clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR) *.DS_Store *~
	make -C converter clean
	make -C test clean
	make -C game clean
	make -C terrain clean
	make -C rigid clean

-include $(OBJS:.o=.d)

$(ENGINE): $(OBJS)
	@mkdir -p $(@D)
	@rm -f $(ENGINE)
	ar -cq $(ENGINE) $(OBJS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<
