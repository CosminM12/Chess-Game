#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "RenderWindow.h"
#include "Piece.h"

bool gameRunning = true;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

SDL_Color color_light = {240, 240, 240, 255};
SDL_Color color_dark = {119, 149, 86, 255};

const int screenWidth=1200, screenHeight=800;
const int squareSize=100;

Uint64 currentTick, lastTick;
double deltaTime;

bool init() {
    if(SDL_Init(SDL_INIT_VIDEO) > 0) {
        printf("SDL_Init has failed. Error: %s\n", SDL_GetError());
    }

    return true;
}


void getEvents(SDL_Event event, bool *gameRunning) {
    while(SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_QUIT:
                *gameRunning = false;
                break;
        }
    }
}


void printfBoard(unsigned char board[8][8]) {
    for(int i=0;i<8;i++) {
        for(int j=0;j<8;j++) {
            printf("%d ", (int)board[i][j]);
        }
        putchar('\n');
    }
    putchar('\n');
}

int main(int argc, char* argv[]) {
    SDL_Event event;
    SDL_Texture* pieceTextures[2][7];
    unsigned char board[8][8] = {0};

    currentTick = SDL_GetPerformanceCounter();
    bool SDLInit = init();
    bool windowCreation = createWindow("Chess Game", &window, &renderer, screenWidth, screenHeight);

    if(!windowCreation || !SDLInit) {
        return -1;
    }

    printf("Program started successfully\n");
    loadPieceTextures(pieceTextures, &renderer);
    SDL_Rect pieceTextureCoordinates = {0, 0, 60, 60};
    char input[] = "RNBQKBNR/PPPPPPPP/////pppppppp/rnbqkbnr";
    placePieces(board, input);

    while(gameRunning) {
        
        //find deltaTime
        lastTick = currentTick;
        currentTick = SDL_GetPerformanceCounter();
        deltaTime = (double)((currentTick - lastTick)*1000/(double)SDL_GetPerformanceFrequency());
        
        // printfBoard(board);
        getEvents(event, &gameRunning);

        
        clear(&renderer);
        drawBoard(renderer, squareSize, screenWidth, color_light, color_dark);
        for(int row=0;row<8;row++) {
            for(int col=0;col<8;col++) {
                renderPiece(pieceTextureCoordinates, 200, squareSize, row, col, getPieceTexture(pieceTextures, board[row][col]), &renderer);
            }
        } 
        display(&renderer);

    }

    cleanUp(window);
    printf("Program ended\n");
    return 0;
}