
EXECUTABLES=sort generate
SRCS=words.cpp
ALL_SRCS=$(SRCS) $(addsuffix .cpp,$(EXECUTABLES))
PCH=sort.hpp

CXX?=g++
CPPFLAGS=-g -O3 -std=c++17 -Wall
LDFLAGS=-g -std=c++17 -Wall
OBJS=$(subst .cpp,.o,$(SRCS))
ALL_OBJS=$(subst .cpp,.o,$(ALL_SRCS))



all: sort generate

generate: $(PCH).gch $(ALL_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $@.o

sort: $(PCH).gch $(ALL_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $@.o

depend: .depend

.depend: $(ALL_SRCS)
	$(CXX) $(CPPFLAGS) -MM $^>./.depend;

clean:
	rm -f $(ALL_OBJS) $(PCH).gch $(EXECUTABLES) .depend

$(PCH).gch: $(PCH)
	$(CXX) -x c++-header $(CPPFLAGS) $(PCH) -o $(PCH).gch

include .depend
