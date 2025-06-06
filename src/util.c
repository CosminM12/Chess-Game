#include "util.h"

Vector2f createVector(int x, int y) { // Note: Original was float, I will keep current int signature for consistency.
    Vector2f temp;
    temp.x = x;
    temp.y = y;

    return temp;
}

// Define global constants for screen dimensions
const int squareSize = 100;
const int boardWidth = 8 * squareSize; // 8 squares * 100px = 800px
const int sidebar1_width = 300; // First sidebar width
const int sidebar2_width = 300; // Second sidebar width
const int screenWidth = boardWidth + sidebar1_width + sidebar2_width; // 800 + 200 + 200 = 1200px
const int screenHeight = 8 * squareSize; // 8 squares * 100px = 800px