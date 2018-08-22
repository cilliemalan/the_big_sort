
EXECUTABLES=generate check-sorted purger
SRCS=words.cpp mappedfile.cpp
ALL_SRCS=$(SRCS) $(addsuffix .cpp,$(EXECUTABLES))
PCH=common.hpp

CXX?=g++
CPPFLAGS=-g -O3 -std=c++17 -Wall -pthread
LDFLAGS=-g -std=c++17 -Wall -lpthread -pthread
OBJS=$(subst .cpp,.o,$(SRCS))



all: generate check-sorted purger

generate: $(PCH).gch words.o
	$(CXX) $(LDFLAGS) -o $@ words.o $@.o

check-sorted: $(PCH).gch mappedfile.o
	$(CXX) $(LDFLAGS) -o $@ mappedfile.o $@.o

purger: $(PCH).gch
	$(CXX) $(LDFLAGS) -o $@ $@.o


install: all
	cp $(EXECUTABLES) /usr/bin/

.depend: $(ALL_SRCS)
	$(CXX) $(CPPFLAGS) -MM $^>./.depend;

clean:
	rm -f $(ALL_OBJS) $(PCH).gch $(EXECUTABLES) .depend

$(PCH).gch: $(PCH)
	$(CXX) -x c++-header $(CPPFLAGS) $(PCH) -o $(PCH).gch

include .depend
