OS := $(shell uname -s)
CC=icpc
EXENAME=test

BIN_DIR=../bin
SRC_DIR=../src
OBJ_DIR=../obj
TEST_SRC_DIR=src
TEST_OBJ_DIR=obj
LIB_DIR=../lib
INC_DIR=$(SRC_DIR)/include

EXE=$(BIN_DIR)/$(EXENAME)

INC=-I$(INC_DIR) -I$(LIB_DIR)/include -I$(LIB_DIR)/include/eigen
HEADER=-DMACOSX -MMD
DEBUG=-g
OPT=-O3
WARN=-ansi -pedantic
CFLAGS=-std=c++11 -c $(INC) $(WARN) $(OPT) $(DEBUG) $(HEADER)

LIB=-L$(LIB_DIR)
ifeq ($(OS),Darwin)
FRAME_FWS=-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo
LIB+=$(FRAME_FWS)
endif
ifeq ($(OS),Linux)
LIB+=-lGL -lXrandr -lXi -lXinerama -lXcursor
endif

TEST_SRC=$(shell find $(TEST_SRC_DIR) -maxdepth 1 -type f -name "*.cpp" -exec basename {} .po \;)
TEST_OBJS=$(patsubst %.cpp,$(TEST_OBJ_DIR)/%.o,$(TEST_SRC))

OBJS=$(OBJ_DIR)/geometry.o $(OBJ_DIR)/model.o $(OBJ_DIR)/grid.o

.PHONY: exe run clean

exe: $(EXE)

run: $(EXE)
	./$(EXE)

clean:
	rm -rf $(EXE) $(TEST_OBJ_DIR) *.DS_Store *~

-include $(OBJS:.o=.d)
-include $(TEST_OBJS:.o=.d)

$(EXE): $(OBJS) $(TEST_OBJS)
	$(CC) -o $(EXE) $(OBJS) $(TEST_OBJS) $(LIB)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<

$(TEST_OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<
