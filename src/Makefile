BASE_DIR=..
OBJS_DIR=$(BASE_DIR)/bin/objs
DEST_DIR=$(BASE_DIR)/bin
INC_DIR=$(BASE_DIR)/includes

CC=g++
NAME=ydlemaster
INC_DIR=$(BASE_DIR)/includes
CFLAGS=-c -g -O2 -Wall -DRASPBERRY -std=gnu++0x -fpermissive -I$(INC_DIR)

LDFLAGS= -lconfig++ -ljsoncpp -lmicrohttpd -lwiringPi -lcurl -lpthread -lboost_signals  -L$(DEST_DIR) -lMisc
#LDFLAGS+=  -llua5.1 # not use for now

SOURCES=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp,$(OBJS_DIR)/%.o,$(SOURCES))
EXE=$(DEST_DIR)/$(NAME)

all: mk_dir $(SOURCES) $(EXE)

mk_dir:
	@mkdir -p $(OBJS_DIR)
h:
	@echo OBJECTS=$(OBJECTS)
	@echo SOURCES=$(SOURCES)
	@echo OBJS_DIR=$(OBJS_DIR)
	@echo EXE=$(EXE)
	
$(EXE): $(OBJECTS) 
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(OBJS_DIR)/%.o: %.cpp $(INC_DIR)/%.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJECTS)
	rm -rf $(EXE)

new : clean all

