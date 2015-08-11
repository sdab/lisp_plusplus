CC      = g++
CFLAGS  = -std=c++11 -g
LDFLAGS = 

all: lisp++ clean

lisp++: lisp++.o
	$(CC) -o $@ $^ $(LDFLAGS)

lisp++.o: lisp++.cpp
	$(CC) -c $(CFLAGS) $<

.PHONY: clean cleanest

clean:
	rm *.o

cleanest: clean
	rm lisp++
