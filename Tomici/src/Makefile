CC = gcc
CFLAGS = -Wall -g

# Detect operating system
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# Add SDL2 flags based on platform
ifeq ($(UNAME_S),Darwin)
    ifeq ($(UNAME_M),arm64)
        # M1/M2 Mac
        CFLAGS += -I/opt/homebrew/include -I/opt/homebrew/include/SDL2
        LDFLAGS = -L/opt/homebrew/lib -lSDL2 -lSDL2_image
    else
        # Intel Mac
        CFLAGS += -I/usr/local/include -I/usr/local/include/SDL2
        LDFLAGS = -L/usr/local/lib -lSDL2 -lSDL2_image
    endif
else
    # Linux
    LDFLAGS = -lSDL2 -lSDL2_image
endif

SRC = main.c RenderWindow.c Piece.c Events.c util.c
OBJ = $(SRC:.c=.o)
OUT = program

# Default target
all: $(OUT)

# Link object files to create exe
$(OUT): $(OBJ)
	$(CC) $(OBJ) -o $(OUT) $(LDFLAGS)

# Compile source file to obj file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJ) $(OUT)
