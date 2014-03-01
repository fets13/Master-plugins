CC=g++
CFLAGS=-c -Wall -DRASPBERRY -std=gnu++0x -fpermissive -Iinclude

LDFLAGS=  -ldl

SOURCES=src/Dir.cpp test/main.cpp
SOURCES=$(wildcard src/*.cpp) $(wildcard test/*.cpp)
OBJECTS=$(patsubst src/%.cpp,objs/%.o,$(wildcard src/*.cpp)) $(patsubst test/%.cpp,objs/%.o,$(wildcard test/*.cpp))
EXECUTABLE=test.exe

ROOT_DIRECTORY=plugins
ROOT_DIRECTORY=.
DIRS := ${sort ${dir ${wildcard */ }}}

all:
	@for dir in $(DIRS); do \
 		echo "make $$dir" ; 	\
 		$(MAKE) -C $$dir; 	\
	done

clean:
	@for dir in $(DIRS); do \
 		echo "make $$dir  clean" ; 	\
 		$(MAKE) -C $$dir  clean; 	\
	done

h:
	@echo DIRS=$(DIRS)
	@echo OBJECTS=$(OBJECTS)
	@echo SOURCES=$(SOURCES)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC)  $(OBJECTS) $(LDFLAGS) -o $@


objs/%.o: src/%.cpp include/Kernel.h include/IProtocol.h include/INode.h
	$(CC) $(CFLAGS) $< -o $@

objs/%.o: test/%.cpp
	$(CC) $(CFLAGS) $< -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

new : clean all

