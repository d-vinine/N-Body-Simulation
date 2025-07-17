SRCDIR := ../
BUILDDIR := ./
OBJDIR := $(BUILDDIR)objects/
BIGOBJ := $(BUILDDIR)simulation.o

CC := gcc
CFLAGS := -Wall -Werror -Wextra -O3 -fopenmp
export CC

SRCS := $(wildcard $(SRCDIR)*.c)
OBJS := $(patsubst $(SRCDIR)%.c,$(OBJDIR)%.o,$(SRCS))

$(OBJDIR)%.o: $(SRCDIR)%.c
	$(CC) $(CFLAGS) -c $< -o $@ 

$(BIGOBJ): $(OBJS)
	$(CC) -r $^ -o $@


.PHONY: clean all

clean:
	rm $(BUILDDIR)*.o $(OBJDIR)*.o

all: $(BIGOBJ)
