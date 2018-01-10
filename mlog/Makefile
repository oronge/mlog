CXX = g++
CXXFLAGS = -Wall -W -Wno-unused-parameter -pg -g -O3 -D__STDC_FORMAT_MACROS -fPIC -std=c++11

OBJECT = mlog
OUTPUT = ./output
INCLUDE_PATH = -I./include
LIB_PATH = ./lib
LIBS = -lpthread

LIBRARY = libmlog.a

.PHONY: all clean


BASE_OBJS := $(wildcard src/*.cc)
BASE_OBJS += $(wildcard src/*.c)
BASE_OBJS += $(wildcard src/*.cpp)
OBJS = $(patsubst %.cc,%.o,$(BASE_OBJS))

all: $(LIBRARY)
	@echo "Success, go, go, go..."


$(LIBRARY): $(OBJS)
	rm -rf $(LIB_PATH)
	mkdir $(LIB_PATH)
	rm -rf $@
	ar -rcs $@ $(OBJS)
	mv ./libmlog.a $(LIB_PATH)
#	make -C ./example

$(OBJECT): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(OBJS): %.o : %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATH) -c $< -o $@ 

$(TOBJS): %.o : %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATH) -c $< $(INCLUDE_PATH) -o $@ 

clean: 
	rm -rf src/*.o
	rm -rf $(LIB_PATH)
	rm -rf $(LIBRARY)
	rm -rf $(OBJECT)
#	make clean -C ./example

