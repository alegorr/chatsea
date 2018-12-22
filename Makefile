CC = g++
CFLAGS = -g -Wall -I/src -std=c++14
SRCDIR = src/
SRCS = $(wildcard $(SRCDIR)/*.cpp)
PROG = chatsea

LIBS = -lzmq -lpthread 

$(PROG):$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)

all: mv $(PROG) ../

clean: ; rm -f *.* $(PROG)
