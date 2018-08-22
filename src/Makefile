
EXECUTABLES=generate check-sorted
SRCS=words.cpp mappedfile.cpp $(addsuffix .cpp,$(EXECUTABLES))
PCH=common.hpp

CXX?=g++
CPPFLAGS=-g -O3 -std=c++17 -Wall -pthread
LDFLAGS=-g -O3 -static -std=c++17 -Wall -lpthread -pthread
OBJS=$(subst .cpp,.o,$(SRCS))



all: generate check-sorted

generate: $(PCH).gch words.o generate.o
	$(CXX) $(LDFLAGS) -o $@ words.o $@.o

check-sorted: $(PCH).gch mappedfile.o check-sorted.o
	$(CXX) $(LDFLAGS) -o $@ mappedfile.o $@.o


install: all
	cp $(EXECUTABLES) /usr/bin/

.depend: $(SRCS)
	$(CXX) $(CPPFLAGS) -MM $^>./.depend;

clean:
	rm -f $(OBJS) $(PCH).gch $(EXECUTABLES) .depend

$(PCH).gch: $(PCH)
	$(CXX) -x c++-header $(CPPFLAGS) $(PCH) -o $(PCH).gch

include .depend
