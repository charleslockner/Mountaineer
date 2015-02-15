CC=g++
EXENAME=game

BINDIR=bin
SRCDIR=src
OBJDIR=obj
LIBDIR=lib
GAME_SRCDIR=game_src
GAME_OBJDIR=$(OBJDIR)/game

EXE=$(BINDIR)/$(EXENAME)

OPTLEVEL=-O1
WARN=
INCLUDES=-I$(SRCDIR)/include -I$(LIBDIR)/include -I$(LIBDIR)/eigen

FRAME_FWS=-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo
AUDIO_FWS=-framework CoreAudio -framework AudioToolbox -framework AudioUnit -framework CoreServices

CFLAGS=-c $(INCLUDES) $(WARN) $(OPTLEVEL) -g -DMACOSX -MMD
LFLAGS=$(FRAME_FWS) $(AUDIO_FWS)

LIBS=$(patsubst %,$(LIBDIR)/%,$(shell find $(LIBDIR) -type f -maxdepth 1 -name "*.a" -exec basename {} .po \;))

FOUNDSRC=$(shell find $(SRCDIR) -type f -maxdepth 1 -name "*.cpp" -exec basename {} .po \;)
FOUNDOBJS=$(patsubst %.cpp,$(OBJDIR)/%.o,$(FOUNDSRC))

GAMESRCS=$(shell find $(GAME_SRCDIR) -type f -name "*.cpp" -exec basename {} .po \;)
GAMEOBJS=$(patsubst %.cpp,$(GAME_OBJDIR)/%.o,$(GAMESRCS))

OBJS=$(GAMEOBJS) $(FOUNDOBJS)

.PHONY: exe run clean

exe: $(EXE)

model: $(EXE)
	make -C model_converter run
	./$(EXE)

run: $(EXE)
	./$(EXE)

clean:
	rm -rf $(BINDIR)
	rm -rf $(OBJDIR)

-include $(OBJS:.o=.d)

$(EXE): $(OBJS)
	mkdir -p $(@D)
	$(CC) $(LFLAGS) -o $(EXE) $(OBJS) $(LIBS)

$(GAME_OBJDIR)/%.o: $(GAME_SRCDIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<
