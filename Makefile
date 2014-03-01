
DIRS := ${sort ${dir ${wildcard */ }}}
DIRS := misc plugins src

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
	
new : clean all

