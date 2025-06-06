#ifndef MAIN_H
#define MAIN_H

#include <SDL2/SDL.h>

// Function to get the renderer for external modules
SDL_Renderer* getMainRenderer();

// Function to show pawn promotion menu
unsigned char showPromotionMenu(SDL_Renderer* renderer, SDL_Texture* pieceTextures[2][7], int x, int y, unsigned char color);

#endif 