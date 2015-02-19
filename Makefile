CC=g++
EXENAME=game

BIN_DIR=bin
SRC_DIR=src
OBJ_DIR=obj
LIB_DIR=lib
GAME_SRC_DIR=game_src
GAME_OBJ_DIR=$(OBJ_DIR)/game

EXE=$(BIN_DIR)/$(EXENAME)

INC=-I$(SRC_DIR)/include -I$(LIB_DIR)/include -I$(LIB_DIR)/include/eigen -I$(LIB_DIR)/include/ceres/internal/miniglog
HEADER=-DMACOSX -MMD
DEBUG=-g
OPT=-O3
WARN=

FRAME_FWS=-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo
AUDIO_FWS=-framework CoreAudio -framework AudioToolbox -framework AudioUnit -framework CoreServices

CFLAGS=-c $(INC) $(WARN) $(OPT) $(DEBUG) $(HEADER)
LFLAGS=$(FRAME_FWS) $(AUDIO_FWS)

LIB=-L$(LIB_DIR) -lglfw3 -lceres
SRC=$(shell find $(SRC_DIR) -type f -maxdepth 1 -name "*.cpp" -exec basename {} .po \;)
OBJ=$(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC))

GAME_SRC=$(shell find $(GAME_SRC_DIR) -type f -name "*.cpp" -exec basename {} .po \;)
GAME_OBJ=$(patsubst %.cpp,$(GAME_OBJ_DIR)/%.o,$(GAME_SRC))

OBJS=$(GAME_OBJ) $(OBJ)

.PHONY: exe model run clean

exe: $(EXE)

model: $(EXE)
	make -C model_converter run
	./$(EXE)

run: $(EXE)
	./$(EXE)

clean:
	rm -rf $(BIN_DIR)
	rm -rf $(OBJ_DIR)

-include $(OBJS:.o=.d)

$(EXE): $(OBJS)
	mkdir -p $(@D)
	$(CC) $(LFLAGS) -o $(EXE) $(OBJS) $(LIB)

$(GAME_OBJ_DIR)/%.o: $(GAME_SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<
