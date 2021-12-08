# Shawn brown
# makefile


CC      = g++
CFLAGS  = -std=c++11 -g -Wall -Wshadow
DEPS = sysclock.h user_proc.cpp  oss.cpp sysclock.cpp bitvector.cpp bitvector.h deadlock.h oss.h
OBJ = oss.o user_proc.o sysclock.o
EXEC = oss user_proc

all: $(EXEC)


$(EXEC): %: %.o
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ): %.o: %.cpp $(DEPS)
	$(CC)  $(CLAGS) -o $@ -c $<


.PHONY: clean

clean:
	rm -f $(EXEC) *.o logfile

