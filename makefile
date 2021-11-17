/*
	
	makefile

*/

CC      = g++
CFLAGS  = -std=c++11 -g -Wall -Wshadow
DEPS = oss.cpp user_proc.cpp sysclock.cpp sysclock.h
OBJ = oss.o user_proc.o sysclock.o
EXEC = oss user_proc

all: $(EXEC)


$(EXEC): %: %.o
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ): %.O: %.cpp $(DEPS)
	$(CC)  $(CLAGS) -o $@ -c $<


.PHONY: clean

clean:
        rm -f $(EXEC) *.o logfile

