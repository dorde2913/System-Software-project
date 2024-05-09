IDIR=inc

_ASSEMBLER_DEPS=Assembler.hpp Line.hpp Parser.hpp
ASSEMBLER_DEPS = $(patsubst %,$(IDIR)/%,$(_ASSEMBLER_DEPS))


SRCDIR=src

_ASSEMBLER_SRC=Assembler.cpp Assembler_main.cpp Line.cpp Parser.cpp
ASSEMBLER_SRC = $(patsubst %,$(SRCDIR)/%,$(_ASSEMBLER_SRC))


TARGET_ASSEMBLER = assembler



CC = g++
CFLAGS = -I$(IDIR)

all: assembler

assembler:
	$(CC) -o $(TARGET_ASSEMBLER) $(ASSEMBLER_SRC) $(CFLAGS)

clean:
	rm -f assembler

