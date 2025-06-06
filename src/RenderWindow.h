#ifndef RENDERWINDOW_H
#define RENDERWINDOW_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include "GameState.h"

bool createWindow(const char* title, SDL_Window** window, SDL_Renderer** renderer, int width, int height);

void clear(SDL_Renderer** renderer);

void display(SDL_Renderer** renderer);

SDL_Texture* loadTexture(const char* filePath, SDL_Renderer** renderer);

void renderPiece(SDL_Rect textureRect, int boardOffset, int squareSize, int row, int col, SDL_Texture* texture, SDL_Renderer** renderer);

unsigned char showPromotionMenu(SDL_Renderer* renderer, SDL_Texture* pieceTextures[2][7], int x, int y, unsigned char color, int screenWidth, int screenHeight);

void drawBoard(SDL_Renderer* renderer, int squareSize, int boardOffset, int screenWidth, SDL_Color color_light, SDL_Color color_dark, SDL_Color color_clicked, SDL_Color color_possible, SDL_Color color_risky, unsigned char board[8][8]);

// Render captured pieces in the sidebar
void renderCapturedPieces(SDL_Renderer* renderer, GameState* state);

// Function to get the renderer for external use
SDL_Renderer* getRenderer();

void cleanUp(SDL_Window* window);

// Font handling functions
bool initFont(const char* fontPath, int fontSize);
void renderText(SDL_Renderer* renderer, const char* text, SDL_Color color, int x, int y);
void destroyFont();

#endif