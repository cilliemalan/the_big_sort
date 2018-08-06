
PROJECT=sort
SRCS=sort.cpp
PCH=sort.hpp

CXX?=g++
CPPFLAGS=-g -O3 -std=c++17 -Wall
LDFLAGS=-g -std=c++17 -Wall
OBJS=$(subst .cpp,.o,$(SRCS))



all: $(PROJECT)

$(PROJECT): $(PCH).gch $(OBJS)
	$(CXX) $(LDFLAGS) -o $(PROJECT) $(OBJS)

depend: .depend

.depend: $(SRCS)
	$(CXX) $(CPPFLAGS) -MM $^>./.depend;

clean:
	rm -f $(OBJS) $(PCH).gch $(PROJECT) .depend

$(PCH).gch: $(PCH)
	$(CXX) -x c++-header $(CPPFLAGS) $(PCH) -o $(PCH).gch

include .depend
