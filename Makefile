# makefile (use -pg for profiling)

IDIR = include
CC = g++
DEBUG = -g
PROFILE =
CFLAGS = -Wall -c $(DEBUG) -I$(IDIR) $(PROFILE)

SDIR = src
ODIR = bin
LIBS = -lm -lpthread

# header files => .cpp files
_DEPS =
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

# object files
_OBJ =
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CC) $(CFLAGS) -o $@ $<

all: dir $(ODIR)/main

dir:
	mkdir -p $(ODIR)

$(ODIR)/main: $(OBJ)
	$(CC) -I$(IDIR) -o $@ $^ $(SDIR)/main.cpp $(PROFILE) $(LIBS)

clean:
	rm -rf $(ODIR) *~ $(INCDIR)/*~

distclean: clean

rebuild: distclean all

.PHONY: clean
