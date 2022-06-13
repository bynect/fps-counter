CC=gcc
CFLAGS=-O2 -g3 -Wall -Werror
LIBS=-lSDL2 -lSDL2_image

SRC=main.c
OBJ=main.o
EXE=fps

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(LIBS) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJ) $(EXE)
