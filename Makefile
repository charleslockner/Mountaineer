OS := $(shell uname -s)
CC=icpc
EXENAME=game

BIN_DIR=bin
SRC_DIR=src
OBJ_DIR=obj
LIB_DIR=lib

EXE=$(BIN_DIR)/$(EXENAME)

INC=-I$(SRC_DIR)/include -I$(LIB_DIR)/include -I$(LIB_DIR)/include/eigen -I$(LIB_DIR)/include/ceres/internal/miniglog
HEADER=-DMACOSX -MMD
DEBUG=-g
OPT=-O3
WARN=-ansi -pedantic
CFLAGS=-std=c++11 -c $(INC) $(WARN) $(OPT) $(DEBUG) $(HEADER)

LIB=-L$(LIB_DIR)
ifeq ($(OS),Darwin)
FRAME_FWS=-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo
LIB+=-lceres_OSX -lglfw3_OSX $(FRAME_FWS)
endif
ifeq ($(OS),Linux)
LIB+=-lceres_LIN -lglfw3_LIN -lGL -lXrandr -lXi -lXinerama -lXcursor
endif

SRC=$(shell find $(SRC_DIR) -maxdepth 1 -type f -name "*.cpp" -exec basename {} .po \;)
OBJS=$(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC))

.PHONY: exe model run clean

exe: $(EXE)

model: $(EXE)
	make -C model_converter run
	./$(EXE)

run: $(EXE)
	./$(EXE)

clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR) *.DS_Store *~
	make -C model_converter clean

-include $(OBJS:.o=.d)

$(EXE): $(OBJS)
	@mkdir -p $(@D)
	$(CC) -o $(EXE) $(OBJS) $(LIB)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<
