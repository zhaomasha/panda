vpath %.cpp ../src
vpath %.hpp ../inc

ifdef dbg
DEBUG:=-ggdb3 -rdynamic
endif

CXXFLAGS=-I ../inc $(DEBUG) 
INC_SERVER=panda_subgraph.hpp panda_head.hpp
SRC_SERVER=panda_subgraph.cpp panda_types.cpp test.cpp
SRC_GXX=$(addprefix ../src/,$(SRC_SERVER))

test_exe:$(INC_SERVER) $(SRC_SERVER) 
	g++ $(CXXFLAGS) $(SRC_GXX) -o test_exe
