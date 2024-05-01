# MPI compiler and linker
MPICC = mpicc
MPICXX = mpicxx
MPIFC = mpifort
MPILINK = mpicc

# Flags for compiling C source files
CCFLAGS = -g -Wall

# Executable name
EXEC = stg_system

# Object files
OBJECTS = $(patsubst %.c,%.o,$(wildcard common/*.c) $(wildcard election/*.c) main.c)

.PHONY: all clean

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(MPILINK)  $(OBJECTS) -o $(EXEC)

%.o : %.c
	$(MPICC) $(CCFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(EXEC) $(wildcard ./*.o)
