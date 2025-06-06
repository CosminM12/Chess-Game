#ifndef RENDERWINDOW_H
#define RENDERWINDOW_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include "GameState.h"

bool createWindow(const char *p_title, SDL_Window **window, SDL_Renderer **renderer, int screenWidth, int screenHeight);

void drawBoard(SDL_Renderer *renderer, int squareSize, int screenWidth, SDL_Color color1, SDL_Color color2,
          SDL_Color colorClicked, SDL_Color colorPossible, SDL_Color colorRisky, unsigned char board[8][8]);

SDL_Texture *loadTexture(const char *p_filePath, SDL_Renderer **renderer);

void render(SDL_Rect textureAtlas, int posx, int posy, SDL_Texture *tex, SDL_Renderer **renderer);

void renderPiece(SDL_Rect pieceAtlas, int boardOffset, int squareSize, int line, int col, SDL_Texture *tex,
                 SDL_Renderer **renderer);

bool initFont(const char *fontPath, int fontSize);

void renderText(SDL_Renderer *renderer, const char *text, SDL_Color color, int x, int y);

void renderCapturedPieces(SDL_Renderer *renderer, SDL_Texture* pieceTextures[2][7], GameState* state);

void destroyFont();

void display(SDL_Renderer **renderer);

void clear(SDL_Renderer **renderer);

void cleanUp(SDL_Window *window);

// Function to get the renderer for external use
SDL_Renderer* getMainRenderer();

unsigned char showPromotionMenu(SDL_Renderer* renderer, SDL_Texture* pieceTextures[2][7], int x, int y, unsigned char color, int screenWidth, int screenHeight);

#endif