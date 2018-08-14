
EXECUTABLES=generate check-sorted
SRCS=words.cpp mappedfile.cpp
ALL_SRCS=$(SRCS) $(addsuffix .cpp,$(EXECUTABLES))
PCH=common.hpp

CXX?=g++
CPPFLAGS=-g -O3 -std=c++17 -Wall -pthread
LDFLAGS=-g -std=c++17 -Wall -lpthread -pthread
OBJS=$(subst .cpp,.o,$(SRCS))
ALL_OBJS=$(subst .cpp,.o,$(ALL_SRCS))



all: generate check-sorted

generate: $(PCH).gch $(ALL_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $@.o

check-sorted: $(PCH).gch $(ALL_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $@.o

.depend: $(ALL_SRCS)
	$(CXX) $(CPPFLAGS) -MM $^>./.depend;

clean:
	rm -f $(ALL_OBJS) $(PCH).gch $(EXECUTABLES) .depend

$(PCH).gch: $(PCH)
	$(CXX) -x c++-header $(CPPFLAGS) $(PCH) -o $(PCH).gch

include .depend
