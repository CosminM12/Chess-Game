#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

#include "RenderWindow.h"

bool createWindow(const char* p_title, SDL_Window** window,SDL_Renderer** renderer, int screenWidth, int screenHeight) {
    //Create new window
    SDL_DisplayMode screenSize;
    SDL_GetCurrentDisplayMode(0, &screenSize);

    *window = SDL_CreateWindow(p_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);

    if(*window == NULL) {
        printf("Window failed to init. Error: %s\n", SDL_GetError());
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED && SDL_RENDERER_PRESENTVSYNC);

    return true;
}

void drawBoard(SDL_Renderer* renderer, int squareSize, int screenWidth, SDL_Color color1, SDL_Color color2, SDL_Color colorClicked, SDL_Color colorPossible, unsigned char board[8][8]) {
    int boardOffset = (screenWidth - squareSize*8) / 2;
    for(int row = 0; row < 8; row++) {
        for(int col = 0; col < 8; col++) {

            // printf("0x%X ", board[row][col]);
            SDL_Color currentColor = ((row + col) % 2 == 0) ? color1 : color2;
            int isClicked = board[row][col] & (0x1 << 5);
            int isPossible = board[row][col] & 0x40;

            if(isClicked) {
                currentColor = colorClicked;
            }
            else if(isPossible) {
                currentColor = colorPossible;
            }

            SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);

            SDL_Rect square = {boardOffset + col * squareSize, row * squareSize, squareSize, squareSize};
            SDL_RenderFillRect(renderer, &square);

        }
        // putchar('\n');
    }
    // putchar('\n');

    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_Rect boardBorder = {boardOffset-1, -1, squareSize*8+2, squareSize*8+2};
    SDL_RenderDrawRect(renderer, &boardBorder);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
}

SDL_Texture* loadTexture(const char* p_filePath, SDL_Renderer** renderer) {
    SDL_Texture* texture = NULL;
    texture = IMG_LoadTexture(*renderer, p_filePath);
    
    if(texture == NULL) {
        printf("Texture failed to load. Error: %s\n", SDL_GetError());
    }

    return texture;
}


void render(SDL_Rect textureAtlas, int posx, int posy, SDL_Texture* tex, SDL_Renderer** renderer) {
    SDL_Rect src = textureAtlas;

    SDL_Rect dst;
    dst.x = posx;
    dst.y = posy;
    dst.w = 100;
    dst.h = 100;

    SDL_RenderCopy(*renderer, tex, &src, &dst);

}

void renderPiece(SDL_Rect pieceAtlas, int boardOffset, int squareSize, int line, int col, SDL_Texture* tex, SDL_Renderer** renderer) {
    SDL_Rect src = pieceAtlas;

    SDL_Rect dst;
    dst.x = boardOffset+col*squareSize;
    dst.y = line*squareSize;
    dst.w = squareSize;
    dst.h = squareSize;

    SDL_RenderCopy(*renderer, tex, &src, &dst);
}


void display(SDL_Renderer** renderer) {
    SDL_RenderPresent(*renderer);
}


void clear(SDL_Renderer** renderer) {
    SDL_RenderClear(*renderer);
}

void cleanUp(SDL_Window* window) {
    TTF_Quit();
    SDL_DestroyWindow(window);
}