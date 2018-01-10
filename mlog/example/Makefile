CXX = g++
CXXFLAGS = -pg -g -Wall -Wno-format -g -O3 -std=c++11 -D__XDEBUG__

OBJECT = log_sample log_sample2 log_sample3

LIB_PATH = -L../lib/

LIBS = -lmlog \
	-lpthread

INCLUDE_PATH = -I../include/

OBJS = log_sample.o log_sample2.o log_sample3.o

.PHONY: all clean

all: $(OBJECT)
	@echo "Success, go, go, go..."

log_sample: log_sample.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(INCLUDE_PATH) $(LIB_PATH) $(LIBS)

log_sample2: log_sample2.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(INCLUDE_PATH) $(LIB_PATH) $(LIBS)

log_sample3: log_sample3.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(INCLUDE_PATH) $(LIB_PATH) $(LIBS)

$(OBJS): %.o : %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDE_PATH) 

clean: 
	rm -rf $(OBJECT)
	rm -f $(OBJS)

