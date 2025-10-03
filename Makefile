UNAME_S := $(shell uname -s)
CXXFLAGS = -std=c++2a -Wall -pedantic -Wformat -I./lib/imgui -I./lib/rlImGui

ifeq ($(UNAME_S),Linux)
	LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif
ifeq ($(UNAME_S),Darwin)
	LIBS = -lm -lpthread -ldl -framework IOKit -framework Cocoa -framework OpenGL `pkg-config --libs --cflags raylib`
endif

MAINSRC=$(wildcard src/main.cpp)
OBJ=$(addsuffix .o,$(basename $(MAINSRC)))

.PHONY: all debug clean test

all: CXXFLAGS += -O3
all: main

debug: CXXFLAGS += -g -O0 -fno-omit-frame-pointer
debug: main

main: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

EDITOR_SRC=$(wildcard src/level_editor.cpp)
EDITOR_OBJ=$(addsuffix .o,$(basename $(EDITOR_SRC)))

editor: $(EDITOR_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f ./src/*.o
	rm -f ./main
	rm -f ./editor
