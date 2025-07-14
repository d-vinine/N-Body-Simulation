SRCDIR := ./src/
BUILDDIR := ./build/
OBJDIR := $(BUILDDIR)objects/
LIBDIR := $(SRCDIR)libs/

CC := gcc
CFLAGS := -Wall -Werror -Wextra -O3 
LDFLAGS := -lm -fopenmp 
INCLUDES := -I$(LIBDIR)raylib/include/ -I$(SRCDIR)simulation
RAYLIB_STATIC := $(LIBDIR)raylib/libraylib.a

SIMBUILD := $(SRCDIR)simulation/build/
SIMOBJ := $(SIMBUILD)simulation.o

MAINOBJ := $(OBJDIR)main.o
EXEC := $(BUILDDIR)n-body-simulation

THREADNUM := 10

.PHONY: all run clean

all: $(EXEC)

$(EXEC): $(MAINOBJ) $(SIMOBJ)
	$(CC) -o $@ $^ $(RAYLIB_STATIC) $(LDFLAGS)

$(MAINOBJ): $(SRCDIR)main.c 
	@mkdir -p $(OBJDIR) 
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ 

$(SIMOBJ):
	$(MAKE) -C $(SIMBUILD) all

run: $(EXEC)
	@export OMP_NUM_THREADS=$(THREADNUM)
	$(EXEC)

clean: 
	rm -f $(MAINOBJ) $(EXEC)
	$(MAKE) -C $(SIMBUILD) clean

