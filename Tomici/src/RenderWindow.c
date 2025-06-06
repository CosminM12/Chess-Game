#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <SDL2/SDL_ttf.h>

#include "RenderWindow.h"

TTF_Font *globalFont = NULL;

bool
createWindow(const char *p_title, SDL_Window **window, SDL_Renderer **renderer, int screenWidth, int screenHeight) {
    //Create new window
    SDL_DisplayMode screenSize;
    SDL_GetCurrentDisplayMode(0, &screenSize);

    *window = SDL_CreateWindow(p_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight,
                               SDL_WINDOW_SHOWN);

    if (*window == NULL) {
        printf("Window failed to init. Error: %s\n", SDL_GetError());
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED && SDL_RENDERER_PRESENTVSYNC);

    return true;
}

void drawBoard(SDL_Renderer *renderer,
               int squareSize,
               int boardOffset,
               int screenWidth,
               SDL_Color color1,
               SDL_Color color2,
               SDL_Color colorClicked,
               SDL_Color colorPossible,
               unsigned char board[8][8]) {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {

            // printf("0x%X ", board[row][col]);
            SDL_Color currentColor = ((row + col) % 2 == 0) ? color1 : color2;
            int isClicked = board[row][col] & (0x1 << 5);
            int isPossible = board[row][col] & 0x40;

            if (isClicked) {
                currentColor = colorClicked;
            } else if (isPossible) {
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
    SDL_Rect boardBorder = {boardOffset - 1, -1, squareSize * 8 + 2, squareSize * 8 + 2};
    SDL_RenderDrawRect(renderer, &boardBorder);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
}

bool initFont(const char *fontPath, int fontSize) {
    if (TTF_Init() == -1) {
        printf("TTF_Init failed: %s\n", TTF_GetError());
        return false;
    }

    globalFont = TTF_OpenFont(fontPath, fontSize);
    if (!globalFont) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return false;
    }

    return true;
}

void renderCapturedPieces(SDL_Renderer *renderer, SDL_Texture* pieceTextures[2][7], GameState* state) {
    // Constants for rendering captured pieces
    const int capturedPieceSize = 40; // Smaller size for captured pieces
    const int padding = 5;
    const int piecesPerRow = 6; // Number of pieces to display per row in sidebar

    SDL_Rect srcRect = {0, 0, 60, 60}; // Assuming piece textures are 60x60 within a larger atlas

    // Render White's Captured Pieces (Black's pieces)
    // This goes into the 'Captured by Black' box, which is currently a blue box at boardWidth, 100
    int startX_blackCaptured = boardWidth + padding;
    int startY_blackCaptured = 140 + padding; // Below the timer box

    for (int i = 0; i < state->numWhiteCapturedPieces; i++) {
        unsigned char pieceType = state->whiteCapturedPieces[i];
        // Captured pieces by White are Black's pieces (COLOR_MASK represents black if set)
        unsigned char pieceByte = pieceType | COLOR_MASK; // Combine type with black color mask

        SDL_Texture *tex = getPieceTexture(pieceTextures, pieceByte);
        if (tex) {
            int currentX = startX_blackCaptured + (i % piecesPerRow) * (capturedPieceSize + padding);
            int currentY = startY_blackCaptured + (i / piecesPerRow) * (capturedPieceSize + padding);

            SDL_Rect destRect = {currentX, currentY, capturedPieceSize, capturedPieceSize};
            SDL_RenderCopy(renderer, tex, &srcRect, &destRect);
        }
    }

    // Render Black's Captured Pieces (White's pieces)
    // This goes into the 'Captured by White' box, which is currently a purple box at boardWidth + sidebar1_width, 250
    int startX_whiteCaptured = boardWidth + padding;
    int startY_whiteCaptured = 390 + padding; // Offset for white's captured box

    for (int i = 0; i < state->numBlackCapturedPieces; i++) {
        unsigned char pieceType = state->blackCapturedPieces[i];
        // Captured pieces by Black are White's pieces (0 for white color)
        unsigned char pieceByte = pieceType; // Combine type with white color (0 means white)

        SDL_Texture *tex = getPieceTexture(pieceTextures, pieceByte);
        if (tex) {
            int currentX = startX_whiteCaptured + (i % piecesPerRow) * (capturedPieceSize + padding);
            int currentY = startY_whiteCaptured + (i / piecesPerRow) * (capturedPieceSize + padding);

            SDL_Rect destRect = {currentX, currentY, capturedPieceSize, capturedPieceSize};
            SDL_RenderCopy(renderer, tex, &srcRect, &destRect);
        }
    }
}

void renderText(SDL_Renderer *renderer, const char *text, SDL_Color color, int x, int y) {

    if (!globalFont) {
        printf("Font not initialized!\n");
        return;
    }

    SDL_Surface *surface = TTF_RenderText_Blended(globalFont, text, color);
    if (!surface) {
        printf("Text render error: %s\n", TTF_GetError());
        return;
    }

//    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect dstRect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void destroyFont() {
    if (globalFont) {
        TTF_CloseFont(globalFont);
        globalFont = NULL;
    }
    TTF_Quit();
}

SDL_Texture *loadTexture(const char *p_filePath, SDL_Renderer **renderer) {
    SDL_Texture *texture = NULL;
    texture = IMG_LoadTexture(*renderer, p_filePath);

    if (texture == NULL) {
        printf("Texture failed to load. Error: %s\n", SDL_GetError());
    }

    return texture;
}


void render(SDL_Rect textureAtlas, int posx, int posy, SDL_Texture *tex, SDL_Renderer **renderer) {
    SDL_Rect src = textureAtlas;

    SDL_Rect dst;
    dst.x = posx;
    dst.y = posy;
    dst.w = 100;
    dst.h = 100;

    SDL_RenderCopy(*renderer, tex, &src, &dst);

}

void renderPiece(SDL_Rect pieceAtlas, int boardOffset, int squareSize, int line, int col, SDL_Texture *tex,
                 SDL_Renderer **renderer) {
    SDL_Rect src = pieceAtlas;

    SDL_Rect dst;
    dst.x = boardOffset + col * squareSize;
    dst.y = line * squareSize;
    dst.w = squareSize;
    dst.h = squareSize;

    SDL_RenderCopy(*renderer, tex, &src, &dst);
}


void display(SDL_Renderer **renderer) {
    SDL_RenderPresent(*renderer);
}


void clear(SDL_Renderer **renderer) {
    SDL_RenderClear(*renderer);
}

void cleanUp(SDL_Window *window) {
    SDL_DestroyWindow(window);
}