# makefile (use -pg for profiling)

IDIR = include
CC = g++
DEBUG = -O2
PROFILE =
CFLAGS = -Wall -D_FILE_OFFSET_BITS="64" -c -std=c++0x $(DEBUG) -I$(IDIR) $(PROFILE)

SDIR = src
ODIR = bin
TDIR = test
LIBS = -lm -lpthread -std=c++0x

# header files => .cpp files
_DEPS = slidingmc.h common.h scont.h mc.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

# object files
_OBJ1 = slidingmc.o scont.o mc.o
OBJ1 = $(patsubst %,$(ODIR)/%,$(_OBJ1))

_TOBJ = slidingmc_test.o scont_test.o
TOBJ = $(patsubst %,$(ODIR)/%,$(_TOBJ))

$(ODIR)/%_test.o: $(TDIR)/%_test.cpp $(DEPS)
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CC) $(CFLAGS) -o $@ $<

all: dir $(ODIR)/query2 $(ODIR)/query2m $(ODIR)/house $(ODIR)/broker_q1 $(ODIR)/broker_q2

dir:
	mkdir -p $(ODIR)

$(ODIR)/query2: $(OBJ1) $(ODIR)/query2.o
	$(CC) -I$(IDIR) -o $@ $^ $(PROFILE) $(LIBS)

$(ODIR)/query2m: $(OBJ1) $(ODIR)/query2m.o
	$(CC) -I$(IDIR) -o $@ $^ $(PROFILE) $(LIBS)

$(ODIR)/house: $(OBJ1) $(ODIR)/house.o $(IDIR)/common_q1.h
	$(CC) -I$(IDIR) -o $@ $^ $(PROFILE) $(LIBS)

$(ODIR)/broker_q1: $(ODIR)/broker_q1.o
	$(CC) -I$(IDIR) -o $@ $^ $(PROFILE) $(LIBS)

$(ODIR)/broker_q2: $(OBJ2)$(ODIR)/broker_q2.o
	$(CC) -I$(IDIR) -o $@ $^ $(PROFILE) $(LIBS)

clean:
	rm -f priv/*.out
	rm -rf $(ODIR) *~ $(INCDIR)/*~

test: clean dir $(TOBJ)
	@$(foreach test,$(TOBJ), $(CC) -o $(patsubst %_test.o,%,$(test)) $(test) -lcppunit && ./$(patsubst %_test.o,%,$(test));)

distclean: clean

rebuild: distclean all

.PHONY: clean
