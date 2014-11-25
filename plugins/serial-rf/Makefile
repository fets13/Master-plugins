BASE_DIR=../..
NAME = $(shell basename $$PWD)
OBJS_DIR=$(BASE_DIR)/bin/objs/$(NAME)
DEST_DIR=$(BASE_DIR)/bin/plugins
INC_DIR=$(BASE_DIR)/includes

CXX=g++
CFLAGS=-c -fPIC -g -O2 -Wall -DRASPBERRY -std=gnu++0x -fpermissive -I$(INC_DIR)

LDFLAGS= -shared

SOURCES=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp,$(OBJS_DIR)/%.o,$(SOURCES))
SO=$(DEST_DIR)/$(NAME).so

all: mk_dir $(SOURCES) $(SO)

mk_dir:
	@mkdir -p $(OBJS_DIR)
	@mkdir -p $(DEST_DIR)
h:
	@echo OBJECTS=$(OBJECTS)
	@echo SOURCES=$(SOURCES)
	@echo OBJS_DIR=$(OBJS_DIR)
	@echo DEST_DIR=$(DEST_DIR)
	@echo SO=$(SO)
	@echo CFLAGS=$(CFLAGS)
	@echo NAME=$(NAME)
	
$(SO)	: $(OBJECTS) 
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

$(OBJS_DIR)/%.o : %.cpp $(INC_DIR)/Kernel.h
	$(CXX) $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJECTS)
	rm -rf $(SO)

new : clean all

