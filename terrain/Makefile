OS := $(shell uname -s)
CC=icpc
EXE_NAME=terrain

BIN_DIR=../bin
SRC_DIR=../src
OBJ_DIR=../obj
LIB_DIR=../lib
INC_DIR=../src/include

EXE=$(BIN_DIR)/$(EXE_NAME)

INC=-I$(INC_DIR) -I$(LIB_DIR)/include -I$(LIB_DIR)/include/eigen
HEADER=-DMACOSX -MMD
DEBUG=-g
OPT=-O2
WARN=-ansi -pedantic
CFLAGS=-std=c++11 -c $(INC) $(WARN) $(OPT) $(DEBUG) $(HEADER)

LIB=-L$(LIB_DIR)
ifeq ($(OS),Darwin)
FRAME_FWS=-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo
LIB+=-lglfw3_OSX $(FRAME_FWS)
endif
ifeq ($(OS),Linux)
LIB+=-lglfw3_LIN -lGL -lXrandr -lXi -lXinerama -lXcursor
endif

OBJS=obj/main.o $(OBJ_DIR)/animation.o $(OBJ_DIR)/attachment_loader.o $(OBJ_DIR)/attachment_loader.o $(OBJ_DIR)/camera.o $(OBJ_DIR)/entity.o $(OBJ_DIR)/geometry.o $(OBJ_DIR)/grid.o $(OBJ_DIR)/loader_ciab.o $(OBJ_DIR)/loader_texture.o $(OBJ_DIR)/model.o $(OBJ_DIR)/reducer.o $(OBJ_DIR)/shader.o $(OBJ_DIR)/shader_builder.o $(OBJ_DIR)/shader_forward.o $(OBJ_DIR)/shader_texture.o $(OBJ_DIR)/terrain.o $(OBJ_DIR)/tiny_obj_loader.o

.PHONY: exe run clean

exe: $(EXE)

clean:
	rm -rf $(OBJ_DIR) *.DS_Store *~

-include obj/main.d
-include $(OBJS:.o=.d)

$(EXE): $(OBJS)
	@mkdir -p $(@D)
	$(CC) -o $(EXE) $(OBJS) $(LIB)

obj/main.o: src/main.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<
