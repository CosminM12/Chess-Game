CC = gcc
CFLAGS = -Wall -g
SRC = main.c RenderWindow.c Piece.c Events.c util.c engine.c GameState.c app_globals.c
OBJ = $(SRC:.c=.o)
OUT = program

#Default target
all: $(OUT)

#Link object file to create exe
$(OUT): $(OBJ)
	$(CC) $(OBJ) -o $(OUT) -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

#Compile source file in obj file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

#Clean
clean:
	rm -f $(OBJ) $(OUT)