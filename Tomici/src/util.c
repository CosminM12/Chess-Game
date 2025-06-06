#include "util.h"

Vector2f createVector(float x, float y) {
    Vector2f temp;
    temp.x = x;
    temp.y = y;
    
    return temp;
}

const int squareSize = 100;
const int sidebar1_width = 300;
const int sidebar2_width = 300;
const int boardWidth = squareSize * 8;
const int screenWidth = boardWidth + sidebar1_width + sidebar2_width;
const int screenHeight = squareSize * 8;