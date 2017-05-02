CXX = g++
CXXFLAGS = -Wall -W -Wno-unused-parameter -pg -g -O3 -D__STDC_FORMAT_MACROS -fPIC -std=c++11

OBJECT = mlog
OUTPUT = ./output

LIBS = -lpthread

LIBRARY = libmlog.a

.PHONY: all clean


BASE_OBJS := $(wildcard ./*.cc)
BASE_OBJS += $(wildcard ./*.c)
BASE_OBJS += $(wildcard ./*.cpp)
OBJS = $(patsubst %.cc,%.o,$(BASE_OBJS))

all: $(LIBRARY)
	@echo "Success, go, go, go..."


$(LIBRARY): $(OBJS)
	rm -rf $(OUTPUT)
	mkdir $(OUTPUT)
	mkdir $(OUTPUT)/include
	mkdir $(OUTPUT)/lib
	rm -rf $@
	ar -rcs $@ $(OBJS)
	cp -rf ./*.h $(OUTPUT)/include
	mv ./libmlog.a $(OUTPUT)/lib/
	make -C ./example

$(OBJECT): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(OBJS): %.o : %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TOBJS): %.o : %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean: 
	rm -rf ./*.o
	rm -rf $(OUTPUT)
	rm -rf $(LIBRARY)
	rm -rf $(OBJECT)
	make clean -C ./example

