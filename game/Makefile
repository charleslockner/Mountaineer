OS := $(shell uname -s)
CC=icpc
EXE_NAME=game

BIN_DIR=../bin
SRC_DIR=src
OBJ_DIR=obj
LIB_DIR=../lib
INC_DIR=../src/include

ENGINE=$(BIN_DIR)/libengine.a
EXE=$(BIN_DIR)/$(EXE_NAME)

INC=-I$(INC_DIR) -I$(LIB_DIR)/include -I$(LIB_DIR)/include/eigen -I$(LIB_DIR)/include/ceres/internal/miniglog
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

.PHONY: exe run clean

exe: $(EXE)

clean:
	rm -rf $(OBJ_DIR) *.DS_Store *~

-include $(OBJS:.o=.d)

$(EXE): $(OBJS) $(ENGINE)
	$(CC) -o $(EXE) $(OBJS) $(ENGINE) $(LIB)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<
