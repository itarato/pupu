UNAME_S := $(shell uname -s)
CXXFLAGS = -std=c++2a -Wall -pedantic -Wformat -I./lib/imgui -I./lib/rlImGui

ifeq ($(UNAME_S),Linux)
	LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif
ifeq ($(UNAME_S),Darwin)
	LIBS = -lm -lpthread -ldl -framework IOKit -framework Cocoa -framework OpenGL `pkg-config --libs --cflags raylib`
endif

MAINSRC=$(wildcard src/main.cpp lib/imgui/*.cpp lib/rlImGui/*.cpp)
OBJ=$(addsuffix .o,$(basename $(MAINSRC)))

TESTSRC=$(wildcard src/tests.cpp)
TESTOBJ=$(addsuffix .o,$(basename $(TESTSRC)))

.PHONY: all debug clean test

all: CXXFLAGS += -O3
all: plogo

debug: CXXFLAGS += -g -O0 -fno-omit-frame-pointer
debug: plogo

plogo: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

test: CXXFLAGS += -g -O0
test: $(TESTOBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f ./src/*.o
	rm -f ./plogo

cleandeep:
	rm -f ./test
	rm -f ./plogo
	rm -f ./src/*.o
	rm -f ./lib/imgui/*.o
	rm -f ./lib/rlImGui/*.o
