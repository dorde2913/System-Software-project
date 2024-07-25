IDIR=inc

_ASSEMBLER_DEPS=Assembler.hpp Line.hpp Parser.hpp
ASSEMBLER_DEPS = $(patsubst %,$(IDIR)/%,$(_ASSEMBLER_DEPS))


SRCDIR=src

_ASSEMBLER_SRC=Assembler.cpp Assembler_main.cpp Line.cpp Parser.cpp
ASSEMBLER_SRC = $(patsubst %,$(SRCDIR)/%,$(_ASSEMBLER_SRC))


TARGET_ASSEMBLER = assembler

_LINKER_DEPS = Linker.hpp
LINKER_DEPS = $(patsubst %,$(IDIR)/%,$(_LINKER_DEPS))

_LINKER_SRC = Linker_main.cpp Linker.cpp
LINKER_SRC = $(patsubst %,$(SRCDIR)/%,$(_LINKER_SRC))

TARGET_LINKER = linker

_EMULATOR_DEPS = Emulator.hpp
EMULATOR_DEPS = $(patsubst %,$(IDIR)/%,$(_EMULATOR_DEPS))

_EMULATOR_SRC = Emulator_main.cpp Emulator.cpp
EMULATOR_SRC = $(patsubst %,$(SRCDIR)/%,$(_EMULATOR_SRC))

TARGET_EMULATOR = emulator

CC = g++
CFLAGS = -I$(IDIR)

all: assembler linker emulator

assembler:
	$(CC) -o $(TARGET_ASSEMBLER) $(ASSEMBLER_SRC) $(CFLAGS)
linker:
	$(CC) -o $(TARGET_LINKER) $(LINKER_SRC) $(CFLAGS)
emulator:
	$(CC) -o $(TARGET_EMULATOR) $(EMULATOR_SRC) $(CFLAGS)

clean:
	rm -f assembler
	rm -f linker
	rm -f emulator

cleanlinker:
	rm -f linker

cleanassembler:
	rm -f assembler

cleanemulator:
	rm -f emulator

