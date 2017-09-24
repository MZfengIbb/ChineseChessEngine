//
// Created by Ibb on 2017/9/19.
//

#include <memory.h>
#include <cstdlib>
#include <search.h>
#include <algorithm>
#include <time.h>

#include "engine.h"

// Step size
static const int8 __king_step[4] = { -16, -1, 1, 16 };
static const int8 __guard_step[4] = { -17, -15, 15, 17 };
static const int8 __knight_step[4][2] =
        {
                { -33, -31 },
                { -18, 14 },
                { -14, 18 },
                { 31, 33 } };
static const int8 __knight_check_step[4][2] =
        {
                { -33, -18 },
                { -31, -14 },
                { 14, 31 },
                { 18, 33 } };

// Mask code for chess in board
static const uint8 __inboard[256] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,
        0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,
        0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,
        0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,
        0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,
        0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,
        0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,
        0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,
        0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,
        0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

// Mask code for chess in the fort
static const uint8 __infort[256] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

// Initialize the board with chess id
static const uint8 __board_init[256] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0, 20, 19, 18, 17, 16, 17, 18, 19, 20,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0, 21,  0,  0,  0,  0,  0, 21,  0,  0,  0,  0,  0,
        0,  0,  0, 22,  0, 22,  0, 22,  0, 22,  0, 22,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0, 14,  0, 14,  0, 14,  0, 14,  0, 14,  0,  0,  0,  0,
        0,  0,  0,  0, 13,  0,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0, 12, 11, 10,  9,  8,  9, 10, 11, 12,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

// Legal move for KING=1, GUARD=2, BISHOP=3
static const int8 __legal_span[512] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  3,  0,  0,  0,  3,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  2,  1,  2,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  1,  0,  1,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  2,  1,  2,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  3,  0,  0,  0,  3,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0
};

// Matrix for KNIGHT is pined
static const int8 __knight_pin[512] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,-16,  0,-16,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0, -1,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0, -1,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0, 16,  0, 16,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0
};



// Value table determined by the chess location
static const uint8 __pos_value[7][256] = {
        { // KING
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0, 11, 15, 11,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
        },{ // GUARD
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0, 20,  0, 20,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0, 23,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0, 20,  0, 20,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
        },{ // BISHOP
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0, 20,  0,  0,  0, 20,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0, 18,  0,  0,  0, 23,  0,  0,  0, 18,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0, 20,  0,  0,  0, 20,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
        },{ // KNIGHT
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0, 90, 90, 90, 96, 90, 96, 90, 90, 90,  0,  0,  0,  0,
                0,  0,  0, 90, 96,103, 97, 94, 97,103, 96, 90,  0,  0,  0,  0,
                0,  0,  0, 92, 98, 99,103, 99,103, 99, 98, 92,  0,  0,  0,  0,
                0,  0,  0, 93,108,100,107,100,107,100,108, 93,  0,  0,  0,  0,
                0,  0,  0, 90,100, 99,103,104,103, 99,100, 90,  0,  0,  0,  0,
                0,  0,  0, 90, 98,101,102,103,102,101, 98, 90,  0,  0,  0,  0,
                0,  0,  0, 92, 94, 98, 95, 98, 95, 98, 94, 92,  0,  0,  0,  0,
                0,  0,  0, 93, 92, 94, 95, 92, 95, 94, 92, 93,  0,  0,  0,  0,
                0,  0,  0, 85, 90, 92, 93, 78, 93, 92, 90, 85,  0,  0,  0,  0,
                0,  0,  0, 88, 85, 90, 88, 90, 88, 90, 85, 88,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
        },{ // ROOK
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,206,208,207,213,214,213,207,208,206,  0,  0,  0,  0,
                0,  0,  0,206,212,209,216,233,216,209,212,206,  0,  0,  0,  0,
                0,  0,  0,206,208,207,214,216,214,207,208,206,  0,  0,  0,  0,
                0,  0,  0,206,213,213,216,216,216,213,213,206,  0,  0,  0,  0,
                0,  0,  0,208,211,211,214,215,214,211,211,208,  0,  0,  0,  0,
                0,  0,  0,208,212,212,214,215,214,212,212,208,  0,  0,  0,  0,
                0,  0,  0,204,209,204,212,214,212,204,209,204,  0,  0,  0,  0,
                0,  0,  0,198,208,204,212,212,212,204,208,198,  0,  0,  0,  0,
                0,  0,  0,200,208,206,212,200,212,206,208,200,  0,  0,  0,  0,
                0,  0,  0,194,206,204,212,200,212,204,206,194,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
        },{ // CANNON
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,100,100, 96, 91, 90, 91, 96,100,100,  0,  0,  0,  0,
                0,  0,  0, 98, 98, 96, 92, 89, 92, 96, 98, 98,  0,  0,  0,  0,
                0,  0,  0, 97, 97, 96, 91, 92, 91, 96, 97, 97,  0,  0,  0,  0,
                0,  0,  0, 96, 99, 99, 98,100, 98, 99, 99, 96,  0,  0,  0,  0,
                0,  0,  0, 96, 96, 96, 96,100, 96, 96, 96, 96,  0,  0,  0,  0,
                0,  0,  0, 95, 96, 99, 96,100, 96, 99, 96, 95,  0,  0,  0,  0,
                0,  0,  0, 96, 96, 96, 96, 96, 96, 96, 96, 96,  0,  0,  0,  0,
                0,  0,  0, 97, 96,100, 99,101, 99,100, 96, 97,  0,  0,  0,  0,
                0,  0,  0, 96, 97, 98, 98, 98, 98, 98, 97, 96,  0,  0,  0,  0,
                0,  0,  0, 96, 96, 97, 99, 99, 99, 97, 96, 96,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
        },{ // PAWN
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  9,  9,  9, 11, 13, 11,  9,  9,  9,  0,  0,  0,  0,
                0,  0,  0, 19, 24, 34, 42, 44, 42, 34, 24, 19,  0,  0,  0,  0,
                0,  0,  0, 19, 24, 32, 37, 37, 37, 32, 24, 19,  0,  0,  0,  0,
                0,  0,  0, 19, 23, 27, 29, 30, 29, 27, 23, 19,  0,  0,  0,  0,
                0,  0,  0, 14, 18, 20, 27, 29, 27, 20, 18, 14,  0,  0,  0,  0,
                0,  0,  0,  7,  0, 13,  0, 16,  0, 13,  0,  7,  0,  0,  0,  0,
                0,  0,  0,  7,  0,  7,  0, 15,  0,  7,  0,  7,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
        } };

// Get true if the index is in board
inline bool in_board(uint32 idx)
{
    return __inboard[idx];
}

// Get true if the index is in fort
inline bool in_fort(uint32 idx)
{
    return __infort[idx];
}

// Get true if KING's move is legal
inline bool king_span(uint32 from_idx, uint32 to_idx)
{
    return __legal_span[to_idx - from_idx + 256] == 1;
}

// Get true if GUARD's move is legal
inline bool guard_span(uint32 from_idx, uint32 to_idx)
{
    return __legal_span[to_idx - from_idx + 256] == 2;
}

// Get true if BISHOP's move is legal
inline bool bishop_span(uint32 from_idx, uint32 to_idx)
{
    return __legal_span[to_idx - from_idx + 256] == 3;
}

// Get the pined index of the BISHOP's move
inline uint32 bishop_pin(uint32 from_idx, uint32 to_idx)
{
    return (from_idx + to_idx) >> 1;
}

// Get the pined index of the KNIGHT's move
inline uint32 knight_pin(uint32 from_idx, uint32 to_idx)
{
    return from_idx + __knight_pin[to_idx - from_idx + 256];
}


static ZobristStruct __zobrist;

////////////////////////////////////////////////////////////////////////
/// Method implements for class

// Initialize the board
void Board::init(void)
{
    int idx, val;
    clear_board();
    for (idx = 0; idx < 256; idx++)
    {
        val = __board_init[idx];
        if (val != 0)
        {
            add_piece(idx, val);
        }
    }
    clear_moves();
    _records.clear();
}
inline void Board::change_side(void)
{
    _player = 1 - _player;
    _zobr.Xor(__zobrist.player);
}
inline void Board::add_piece(uint32 idx, uint8 piece)
{
    _board[idx] = piece;
    // Add value for Red while sub value for Black
    if (piece < 16)
    {
        _red_val += __pos_value[piece - 8][idx];
        _zobr.Xor(__zobrist.table[piece - 8][idx]);
    }
    else
    {
        _black_val += __pos_value[piece - 16][rotate(idx)];
        _zobr.Xor(__zobrist.table[piece - 9][idx]);
    }
}
inline void Board::del_piece(uint32 idx, uint8 piece)
{
    _board[idx] = 0;
    // Sub value for Red while add value for Black
    if (piece < 16)
    {
        _red_val -= __pos_value[piece - 8][idx];
        _zobr.Xor(__zobrist.table[piece - 8][idx]);
    }
    else
    {
        _black_val -= __pos_value[piece - 16][rotate(idx)];
        _zobr.Xor(__zobrist.table[piece - 9][idx]);
    }
}
inline int32 Board::evaluate(void) const
{
    return (_player == 0 ? _red_val - _black_val : _black_val - _red_val)
           + ADVANCED_VALUE;
}
inline uint8 Board::move_piece(uint32 mv)
{
    uint32 from_idx, to_idx;
    uint8 from_val, captured;
    from_idx = from(mv);
    to_idx = to(mv);
    captured = _board[to_idx];
    if (captured != 0)
        del_piece(to_idx, captured);
    from_val = _board[from_idx];
    del_piece(from_idx, from_val);
    add_piece(to_idx, from_val);
    return captured;
}
inline void Board::undo_move_piece(uint32 mv, uint8 captured)
{
    uint32 from_idx, to_idx;
    uint8 val;
    from_idx = from(mv);
    to_idx = to(mv);
    val = _board[to_idx];
    del_piece(to_idx, val);
    add_piece(from_idx, val);
    if (captured != 0)
        add_piece(to_idx, captured);
}
inline bool Board::make_move(uint32 mv)
{
    uint8 captured;
    uint32 key;
    key = _zobr.key;
    captured = move_piece(mv);
    if (checked())
    {
        undo_move_piece(mv, captured);
        return false;
    }
    change_side();
    _move_list[_move_count].set(mv, captured, checked(), key);
    _move_count++;
    _distance++;
    return true;
}
inline void Board::undo_make_move(void)
{
    _distance--;
    _move_count--;
    change_side();
    undo_move_piece(_move_list[_move_count].mv, _move_list[_move_count].captured);
}
bool Board::do_move(uint32 mv)
{
    bool ret = make_move(mv);
    if (ret)
    {
        _records.push_back(_move_list[_move_count - 1]);
    }
    return ret;
}
void Board::undo_move(void)
{
    if (_move_count > 1)
    {
        undo_make_move();
        _records.pop_back();
    }
    else if (_records.size() > 0)
    {
        MoveInfo& mvinfo = _records.back();
        change_side();
        undo_move_piece(mvinfo.mv, mvinfo.captured);
        _records.pop_back();
    }
}
inline void Board::make_null_move(void)
{
    uint32 key;
    key = _zobr.key;
    change_side();
    _move_list[_move_count].set(0, 0, false, key);
    _move_count++;
    _distance++;
}
inline void Board::undo_make_null_move(void)
{
    _distance--;
    _move_count--;
    change_side();
}
bool Board::checked(void) const
{
    uint32 from_idx, to_idx;
    uint8 to_val;
    int32 step;
    uint8 self_base = base(_player);
    uint8 mate_base = base(!_player);

    //for (from_idx = 0; from_idx < 256; from_idx++)
    for (from_idx = 0x33; from_idx <= 0xcb; from_idx++)
    {
        // Find KING
        if (_board[from_idx] != self_base + KING)
            continue;

        // 1. Checked by PAWN ?
        if (_board[forward(from_idx, _player)] == mate_base + PAWN)
            return true;
        for (step = -1; step <= 1; step += 2)
        {
            if (_board[from_idx + step] == mate_base + PAWN)
                return true;
        }

        // 2. Checked by KNIGHT ?
        for (uint32 i = 0; i < 4; i++)
        {
            if (_board[from_idx + __guard_step[i]] != 0)
                continue;
            for (uint32 j = 0; j < 2; j++)
            {
                to_val = _board[from_idx + __knight_check_step[i][j]];
                if (to_val == mate_base + KNIGHT)
                    return true;
            }
        }

        // 3. Checked by ROOK or KING or CANNON ?
        for (uint32 i = 0; i < 4; i++)
        {
            step = __king_step[i];
            to_idx = from_idx + step;
            while (in_board(to_idx))
            {
                to_val = _board[to_idx];
                if (to_val != 0)
                {
                    if (to_val == mate_base + ROOK || to_val == mate_base
                                                                + KING)
                        return true;
                    break;
                }
                to_idx += step;
            }
            to_idx += step;
            while (in_board(to_idx))
            {
                to_val = _board[to_idx];
                if (to_val != 0)
                {
                    if (to_val == mate_base + CANNON)
                        return true;
                    break;
                }
                to_idx += step;
            }
        }
        return false;
    }
    return false;
}
bool Board::is_mate(void)
{
    uint8 captured;
    uint32 mvs[MAX_GEN_MOVES];
    uint32 gen_count = gen_moves(mvs);
    for (uint32 i = 0; i < gen_count; i++)
    {
        captured = move_piece(mvs[i]);
        if (!checked())
        {
            undo_move_piece(mvs[i], captured);
            return false;
        }
        else
        {
            undo_move_piece(mvs[i], captured);
        }
    }
    return true;
}
bool Board::legal_move(uint32 mv) const
{
    uint32 from_idx, to_idx, pin_idx, step;
    uint8 self_base, from_val, to_val;
    // Judgement process：

    // 1. Player's chess is on the starting point
    from_idx = from(mv);
    from_val = _board[from_idx];
    self_base = base(_player);
    if ((from_val & self_base) == 0)
        return false;

    // 2. Player's chess can't on the target point
    to_idx = to(mv);
    to_val = _board[to_idx];
    if ((to_val & self_base) != 0)
        return false;

    // 3. Judge by the chess ID
    switch (from_val - self_base)
    {
        case KING:
            return in_fort(to_idx) && king_span(from_idx, to_idx);
        case GUARD:
            return in_fort(to_idx) && guard_span(from_idx, to_idx);
        case BISHOP:
            return same_side(from_idx, to_idx) && bishop_span(from_idx, to_idx)
                   && _board[bishop_pin(from_idx, to_idx)] == 0;
        case KNIGHT:
            pin_idx = knight_pin(from_idx, to_idx);
            return pin_idx != from_idx && _board[pin_idx] == 0;
        case ROOK:
        case CANNON:
            if (same_rank(from_idx, to_idx))
            {
                step = (to_idx < from_idx ? -1 : 1);
            }
            else if (same_file(from_idx, to_idx))
            {
                step = (to_idx < from_idx ? -16 : 16);
            }
            else
            {
                return false;
            }
            pin_idx = from_idx + step;
            while (pin_idx != to_idx && _board[pin_idx] == 0)
            {
                pin_idx += step;
            }
            if (pin_idx == to_idx)
            {
                return to_val == 0 || from_val - self_base == ROOK;
            }
            else if (to_val != 0 && from_val - self_base == CANNON)
            {
                pin_idx += step;
                while (pin_idx != to_idx && _board[pin_idx] == 0)
                {
                    pin_idx += step;
                }
                return pin_idx == to_idx;
            }
            else
            {
                return false;
            }
        case PAWN:
            if (away_side(to_idx, _player) && (to_idx == from_idx - 1 || to_idx == from_idx
                                                                                   + 1))
            {
                return true;
            }
            return to_idx == forward(from_idx, _player);
        default:
            return false;
    }
}
uint32 Board::gen_moves(uint32 * moves, bool capture)
{
    uint32 gen_count = 0;
    uint8 self_base = base(_player);
    uint8 mate_base = base(!_player);
    int32 from_idx, to_idx;
    uint8 from_val, to_val;
    int32 step;
    //for (from_idx = 0; from_idx < 256; ++from_idx)
    for (from_idx = 0x33; from_idx <= 0xcb; ++from_idx)
    {
        from_val = _board[from_idx];
        // Chess is owned by current player
        if ((from_val & self_base) == 0)
            continue;

        // Generate moves by chess ID
        switch (from_val - self_base)
        {
            case KING:
                for (uint32 i = 0; i < 4; i++)
                {
                    to_idx = from_idx + __king_step[i];
                    if (!in_fort(to_idx))
                        continue;
                    to_val = _board[to_idx];
                    if (capture ? (to_val & mate_base) != 0
                                : (to_val & self_base) == 0)
                        moves[gen_count++] = move(from_idx, to_idx);
                }
                break;
            case GUARD:
                for (uint32 i = 0; i < 4; i++)
                {
                    to_idx = from_idx + __guard_step[i];
                    if (!in_fort(to_idx))
                        continue;
                    to_val = _board[to_idx];
                    if (capture ? (to_val & mate_base) != 0
                                : (to_val & self_base) == 0)
                        moves[gen_count++] = move(from_idx, to_idx);
                }
                break;
            case BISHOP:
                for (uint32 i = 0; i < 4; i++)
                {
                    to_idx = from_idx + __guard_step[i];
                    if (!(in_board(to_idx) && home_side(to_idx, _player)
                          && _board[to_idx] == 0))
                        continue;

                    to_idx += __guard_step[i];
                    to_val = _board[to_idx];
                    if (capture ? (to_val & mate_base) != 0
                                : (to_val & self_base) == 0)
                        moves[gen_count++] = move(from_idx, to_idx);
                }
                break;
            case KNIGHT:
                for (uint32 i = 0; i < 4; i++)
                {
                    to_idx = from_idx + __king_step[i];
                    if (_board[to_idx] != 0)
                        continue;
                    for (uint32 j = 0; j < 2; j++)
                    {
                        to_idx = from_idx + __knight_step[i][j];
                        if (!in_board(to_idx))
                            continue;
                        to_val = _board[to_idx];
                        if (capture ? (to_val & mate_base) != 0 : (to_val
                                                                   & self_base) == 0)
                            moves[gen_count++] = move(from_idx, to_idx);
                    }
                }
                break;
            case ROOK:
                for (uint32 i = 0; i < 4; i++)
                {
                    step = __king_step[i];
                    to_idx = from_idx + step;
                    while (in_board(to_idx))
                    {
                        to_val = _board[to_idx];
                        if (to_val == 0)
                        {
                            if (!capture)
                                moves[gen_count++] = move(from_idx, to_idx);
                        }
                        else
                        {
                            if ((to_val & mate_base) != 0)
                                moves[gen_count++] = move(from_idx, to_idx);
                            break;
                        }
                        to_idx += step;
                    }
                }
                break;
            case CANNON:
                for (uint32 i = 0; i < 4; i++)
                {
                    step = __king_step[i];
                    to_idx = from_idx + step;
                    while (in_board(to_idx))
                    {
                        to_val = _board[to_idx];
                        if (to_val == 0)
                        {
                            if (!capture)
                                moves[gen_count++] = move(from_idx, to_idx);
                        }
                        else
                        {
                            break;
                        }
                        to_idx += step;
                    }
                    to_idx += step;
                    while (in_board(to_idx))
                    {
                        to_val = _board[to_idx];
                        if (to_val != 0)
                        {
                            if ((to_val & mate_base) != 0)
                                moves[gen_count++] = move(from_idx, to_idx);
                            break;
                        }
                        to_idx += step;
                    }
                }
                break;
            case PAWN:
                to_idx = forward(from_idx, _player);
                if (in_board(to_idx))
                {
                    to_val = _board[to_idx];
                    if (capture ? (to_val & mate_base) != 0 : (to_val
                                                               & self_base) == 0)
                        moves[gen_count++] = move(from_idx, to_idx);
                }
                if (away_side(from_idx, _player))
                {
                    for (step = -1; step <= 1; step += 2)
                    {
                        to_idx = from_idx + step;
                        if (in_board(to_idx))
                        {
                            to_val = _board[to_idx];
                            if (capture ? (to_val & mate_base) != 0 : (to_val
                                                                       & self_base) == 0)
                                moves[gen_count++] = move(from_idx, to_idx);
                        }
                    }
                }
                break;
        }
    }
    return gen_count;
}
int Board::rep_status(int32 recur) const
{
    bool self_side, perp_check, opp_perp_check;
    const MoveInfo *mvs;

    self_side = false;
    perp_check = opp_perp_check = true;
    mvs = _move_list + _move_count - 1;
    while (mvs->mv != 0 && mvs->captured == 0)
    {
        if (self_side)
        {
            perp_check = perp_check && mvs->check;
            if (mvs->key == _zobr.key)
            {
                recur--;
                if (recur == 0)
                {
                    return 1 + (perp_check ? 2 : 0) + (opp_perp_check ? 4 : 0);
                }
            }
        }
        else
        {
            opp_perp_check = opp_perp_check && mvs->check;
        }
        self_side = !self_side;
        mvs--;
    }
    return 0;
}


void Engine::load_book(void)
{
    std::ifstream infs;
    try
    {
        infs.open("book.dat", std::ios::in | std::ios::binary);
        infs.read((char *)_book_table, BOOK_SIZE * sizeof(BookItem));
        _book_size = infs.gcount() / sizeof(BookItem);
        infs.close();
    }
    catch (...)
    {
        _book_size = 0;
    }
}
static int compare_book(const void *lpbk1, const void *lpbk2)
{
    return ((BookItem *)lpbk1)->lock - ((BookItem *)lpbk2)->lock;
}
int32 Engine::search_book(void)
{
    int32 i, vl, moves;
    uint32 mv;
    uint32 mvs[MAX_GEN_MOVES];
    int32 vls[MAX_GEN_MOVES];
    bool is_mirror;
    BookItem item, *pitem;
    Board mirror_board;
    // Searching process

    // 1. return if there's no book
    if (_book_size == 0)
        return 0;

    // 2. search book for current board
    is_mirror = false;
    item.lock = _board._zobr.lock1;
    pitem = (BookItem *)bsearch(&item, _book_table,
                                _book_size, sizeof(BookItem), compare_book);
    // 3. search book for current mirror board
    if (pitem == NULL)
    {
        is_mirror = true;
        _board.mirror(mirror_board);
        item.lock = mirror_board._zobr.lock1;
        pitem = (BookItem *)bsearch(&item, _book_table,
                                    _book_size, sizeof(BookItem), compare_book);
    }
    // 4. return if find nothing
    if (pitem == NULL)
    {
        return 0;
    }
    // 5. return to the first book item if find successfully
    while (pitem >= _book_table && pitem->lock == item.lock)
    {
        pitem--;
    }
    pitem++;
    // 6. write the moves and values into "mvs" and "vls"
    vl = moves = 0;
    while (pitem < _book_table + _book_size && pitem->lock == item.lock)
    {
        mv = (is_mirror ? mirror_move(pitem->mv) : pitem->mv);
        if (_board.legal_move(mv))
        {
            mvs[moves] = mv;
            vls[moves] = pitem->vl;
            vl += vls[moves];
            moves++;
            if (moves == MAX_GEN_MOVES)
            {
                break;
            }
        }
        pitem++;
    }
    if (vl == 0)
    {
        return 0;
    }
    // 7. randomly choose a move
    vl = rand() % vl;
    for (i = 0; i < moves; i++)
    {
        vl -= vls[i];
        if (vl < 0)
        {
            break;
        }
    }
    return mvs[i];
}
int32 Engine::probe_hash(int32 alpha, int32 beta, int32 depth, uint32 &mv)
{
    bool mate; // Checkmate flag, can search deeper if it's true
    HashItem hsh;

    hsh = _hash_table[_board._zobr.key & (HASH_SIZE - 1)];
    if (hsh.lock0 != _board._zobr.lock0 || hsh.lock1 != _board._zobr.lock1)
    {
        mv = 0;
        return -MATE_VALUE;
    }
    mv = hsh.mv;
    mate = false;
    if (hsh.vl > WIN_VALUE)
    {
        if (hsh.vl < BAN_VALUE)
        {
            // End the search if it's unstable
            return -MATE_VALUE;
        }
        hsh.vl -= _board._distance;
        mate = true;
    }
    else if (hsh.vl < -WIN_VALUE)
    {
        if (hsh.vl > -BAN_VALUE)
        {
            // Same as above
            return -MATE_VALUE;
        }
        hsh.vl += _board._distance;
        mate = true;
    }
    if (hsh.depth >= depth || mate)
    {
        if (hsh.flag == HASH_BETA)
        {
            return (hsh.vl >= beta ? hsh.vl : -MATE_VALUE);
        }
        else if (hsh.flag == HASH_ALPHA)
        {
            return (hsh.vl <= alpha ? hsh.vl : -MATE_VALUE);
        }
        return hsh.vl;
    }
    return -MATE_VALUE;
}
void Engine::record_hash(int32 flag, int32 vl, int32 depth, uint32 mv)
{
    HashItem hsh;
    hsh = _hash_table[_board._zobr.key & (HASH_SIZE - 1)];
    if (hsh.depth > depth)
    {
        return;
    }
    hsh.flag = flag;
    hsh.depth = depth;
    if (vl > WIN_VALUE)
    {
        if (mv == 0 && vl <= BAN_VALUE)
        {
            return; // End the search if it's unstable
        }
        hsh.vl = vl + _board._distance;
    }
    else if (vl < -WIN_VALUE)
    {
        if (mv == 0 && vl >= -BAN_VALUE)
        {
            return; // Same as above
        }
        hsh.vl = vl - _board._distance;
    }
    else
    {
        hsh.vl = vl;
    }
    hsh.mv = mv;
    hsh.lock0 = _board._zobr.lock0;
    hsh.lock1 = _board._zobr.lock1;
    _hash_table[_board._zobr.key & (HASH_SIZE - 1)] = hsh;
}
// MVV/LVA for every chess
extern const uint8 __mvv_lva[24] =
        { 0, 0, 0, 0, 0, 0, 0, 0, 5, 1, 1, 3, 4, 3, 2, 0, 5, 1, 1, 3, 4, 3, 2, 0 };
// Sort by comparing with MVV/LVA
class CompareMvvLva
{
public:
    inline CompareMvvLva(Board& board);
    inline bool operator()(uint32 mv1, uint32 mv2);
private:
    Board& _board;
};

CompareMvvLva::CompareMvvLva(Board& board)
        : _board(board)
{
}
inline bool CompareMvvLva::operator()(uint32 mv1, uint32 mv2)
{
    return _board.mvv_lva(mv1) > _board.mvv_lva(mv2);
}
// Sort by comparing with history tale
class CompareHistory
{
public:
    inline CompareHistory(int32* history_table);
    inline bool operator()(uint32 mv1, uint32 mv2);
private:
    int32* _history_table;
};
inline CompareHistory::CompareHistory(int32* history_table)
        : _history_table(history_table)
{
}
inline bool CompareHistory::operator()(uint32 mv1, uint32 mv2)
{
    return _history_table[mv1] > _history_table[mv2];
}

// Sort the moves
#define PHASE_HASH			0
#define PHASE_KILLER_1		1
#define PHASE_KILLER_2		2
#define PHASE_GEN_MOVES		3
#define PHASE_REST			4

// Sort struct
struct SortStruct
{
    inline SortStruct(Engine& engine);
    uint32 mv_hash, mv_killer1, mv_killer2; // hash table moves and killer moves
    int32 phase, index, moves; // current phase, move index , move counts
    uint32 mvs[MAX_GEN_MOVES]; // all generated moves

    inline void init(uint32 mv_hash_);
    // Get next move
    int next(void);
    Engine& _engine;
};

inline SortStruct::SortStruct(Engine& engine) : _engine(engine)
{
}

inline void SortStruct::init(uint32 mv_hash_)
{ // Initialize the hash moves and two kind of killer moves
    mv_hash = mv_hash_;
    mv_killer1 = _engine._killers[_engine._board._distance][0];
    mv_killer2 = _engine._killers[_engine._board._distance][1];
    phase = PHASE_HASH;
}

// Get the next move
int SortStruct::next(void)
{
    uint32 mv;
    CompareHistory comp_his(_engine._history_table);
    switch (phase)
    {
        // "phase" means heuristic move phase

        // 0. hash table heuristic move
        case PHASE_HASH:
            phase = PHASE_KILLER_1;
            if (mv_hash != 0)
            {
                return mv_hash;
            }
            // Tips：No "break" here, we can do the next "case"

            // 1. first killer heuristic move
        case PHASE_KILLER_1:
            phase = PHASE_KILLER_2;
            if (mv_killer1 != mv_hash && mv_killer1 != 0
                && _engine._board.legal_move(mv_killer1))
            {
                return mv_killer1;
            }

            // 2. second killer heuristic move
        case PHASE_KILLER_2:
            phase = PHASE_GEN_MOVES;
            if (mv_killer2 != mv_hash && mv_killer2 != 0
                && _engine._board.legal_move(mv_killer2))
            {
                return mv_killer2;
            }

            // 3. generate all moves
        case PHASE_GEN_MOVES:
            phase = PHASE_REST;
            moves = _engine._board.gen_moves(mvs);
            std::sort(mvs, mvs + moves, comp_his);
            index = 0;

            // 4. do history heuristic move for remained moves
        case PHASE_REST:
            while (index < moves)
            {
                mv = mvs[index];
                index++;
                if (mv != mv_hash && mv != mv_killer1 && mv != mv_killer2)
                {
                    return mv;
                }
            }

            // 5. return 0 as no moves remained。
        default:
            return 0;
    }
}





Engine::Engine()
{
    load_book();
    init();
}
int32 Engine::search_quiesc(int alpha, int beta)
{
    int32 i, moves;
    int32 vl, vl_best;
    uint32 mvs[MAX_GEN_MOVES];
    CompareHistory comp_his(_history_table);
    CompareMvvLva comp_mvvlva(_board);

    // Quiescence search process

    // 1. Check if recurs
    vl = _board.rep_status();
    if (vl != 0)
    {
        return _board.rep_value(vl);
    }

    // 2. Return status evaluate if reaches the limited depth
    if (_board._distance == LIMIT_DEPTH)
    {
        return _board.evaluate();
    }

    // 3. Initialize the MATE_VALUE
    vl_best = -MATE_VALUE;

    if (_board.in_check())
    {
        // 4. Generate all moves if KING is checked
        moves = _board.gen_moves(mvs);
        std::sort(mvs, mvs + moves, comp_his);
    }
    else
    {

        // 5. Evaluate the status if not checked
        vl = _board.evaluate();
        if (vl > vl_best)
        {
            vl_best = vl;
            if (vl >= beta)
            {
                return vl;
            }
            if (vl > alpha)
            {
                alpha = vl;
            }
        }

        // 6. Generate the capture moves
        moves = _board.gen_moves(mvs, true);
        std::sort(mvs, mvs + moves, comp_mvvlva);
    }

    // 7. Recurse these moves
    for (i = 0; i < moves; i++)
    {
        if (_board.make_move(mvs[i]))
        {
            vl = -search_quiesc(-beta, -alpha);
            _board.undo_make_move();

            // 8. Alpha-Beta judgement and cut
            if (vl > vl_best)
            { // get best value
                vl_best = vl; // "vl_best"to be returned, whith may beyond Alpha-Beta border
                if (vl >= beta)
                { // Find a Beta move
                    return vl; // Beta cut
                }
                if (vl > alpha)
                { // Find a PV move
                    alpha = vl; // Narrow the Alpha-Beta border
                }
            }
        }
    }
    // 9. return the best move if all moves are searched
    return vl_best == -MATE_VALUE ? _board._distance - MATE_VALUE : vl_best;
}

int32 Engine::search_full(int32 alpha, int32 beta, int32 depth, bool no_null)
{
    int32 hash_flag, vl, vl_best;
    uint32 mv, mv_best, mv_hash;
    int32 new_depth;
    SortStruct sort_data(*this);
    // An Alpha-Beta complete searching process

    // 1. Quiescence search process until depth > 0
    if (depth <= 0)
    {
        return search_quiesc(alpha, beta);
    }

    // 1-1. Check if recurs
    vl = _board.rep_status();
    if (vl != 0)
    {
        return _board.rep_value(vl);
    }

    // 1-2. Return status evaluate if reaches the limited depth
    if (_board._distance == LIMIT_DEPTH)
    {
        return _board.evaluate();
    }

    // 1-3. Try hash table moves
    vl = probe_hash(alpha, beta, depth, mv_hash);
    if (vl > -MATE_VALUE)
    {
        return vl;
    }

    // 1-4. Try NULL moves cut
    if (!no_null && !_board.in_check() && _board.null_okay())
    {
        _board.make_null_move();
        vl = -search_full(-beta, 1 - beta, depth - NULL_DEPTH - 1, true);
        _board.undo_make_null_move();
        if (vl >= beta)
        {
            return vl;
        }
    }

    // 2. Initialize best move and value
    hash_flag = HASH_ALPHA;
    vl_best = -MATE_VALUE;
    mv_best = 0;

    // 3. Initialize move sort struct
    sort_data.init(mv_hash);

    // 4. Sort and recurse the moves
    while ((mv = sort_data.next()) != 0)
    {
        if (_board.make_move(mv))
        {
            // Search deeper when KING is checked
            new_depth = _board.in_check() ? depth : depth - 1;
            // PVS
            if (vl_best == -MATE_VALUE)
            {
                vl = -search_full(-beta, -alpha, new_depth);
            }
            else
            {
                vl = -search_full(-alpha - 1, -alpha, new_depth);
                if (vl > alpha && vl < beta)
                {
                    vl = -search_full(-beta, -alpha, new_depth);
                }
            }
            _board.undo_make_move();

            // 5. Alpha-Beta size judgment and truncation
            if (vl > vl_best)
            { // Find best value
                vl_best = vl; // "vl_best" is the best value that may beyond Alpha-Beta border
                if (vl >= beta)
                { // Find a beta move
                    hash_flag = HASH_BETA;
                    mv_best = mv; // Save the BETA move to history table
                    break; // Beta cut
                }
                if (vl > alpha)
                { // Find a PV movement
                    hash_flag = HASH_PV;
                    mv_best = mv; // Save the PV move to history table
                    alpha = vl; // narrow the Alpha-Beta border
                }
            }
        }
    }

    // 5. Save the best move to history table and return the best value
    if (vl_best == -MATE_VALUE)
    {
        // Evaluate the move by step counts if it's killer move
        return _board._distance - MATE_VALUE;
    }
    // Record to hash table
    record_hash(hash_flag, vl_best, depth, mv_best);
    if (mv_best != 0)
    {
        // Save to the history move if it's not Alpha move
        set_best_move(mv_best, depth);
    }
    return vl_best;
}
int32 Engine::search_root(int depth)
{
    int32 vl, vl_best, new_depth;
    uint32 mv;
    SortStruct sort_data(*this);

    vl_best = -MATE_VALUE;
    sort_data.init(_mv_result);
    while ((mv = sort_data.next()) != 0)
    {
        if (_board.make_move(mv))
        {
            new_depth = _board.in_check() ? depth : depth - 1;
            if (vl_best == -MATE_VALUE)
            {
                vl = -search_full(-MATE_VALUE, MATE_VALUE, new_depth, true);
            }
            else
            {
                vl = -search_full(-vl_best - 1, -vl_best, new_depth);
                if (vl > vl_best)
                {
                    vl = -search_full(-MATE_VALUE, -vl_best, new_depth, true);
                }
            }
            _board.undo_make_move();
            if (vl > vl_best)
            {
                vl_best = vl;
                _mv_result = mv;
                if (vl_best > -WIN_VALUE && vl_best < WIN_VALUE)
                {
                    vl_best += (rand() & RANDOM_MASK) - (rand() & RANDOM_MASK);
                }
            }
        }
    }
    record_hash(HASH_PV, vl_best, depth, _mv_result);
    set_best_move(_mv_result, depth);
    return vl_best;
}
void Engine::search_main(float sec)
{
    int32 i, t, vl, moves;
    uint32 mvs[MAX_GEN_MOVES];

    // Initialize
    memset(_history_table, 0, 65536 * sizeof(int32)); // Clear history table
    memset(_killers, 0, LIMIT_DEPTH * 2 * sizeof(uint32)); // Clear killer move table
    memset(_hash_table, 0, HASH_SIZE * sizeof(HashItem)); // Clear hash table
    t = clock(); // Initialize the timer
    _board._distance = 0; // Initialize the distance

    // Search for the book
    _mv_result = search_book();
    if (_mv_result != 0)
    {
        _board.make_move(_mv_result);
        if (_board.rep_status(3) == 0)
        {
            _board.undo_make_move();
            return;
        }
        _board.undo_make_move();
    }

    // Check if it's the only move
    vl = 0;
    moves = _board.gen_moves(mvs);
    for (i = 0; i < moves; i++)
    {
        if (_board.make_move(mvs[i]))
        {
            _board.undo_make_move();
            _mv_result = mvs[i];
            vl++;
        }
    }
    if (vl == 1)
    {
        return;
    }

    // Iterations deepen the search process
    for (i = 1; i <= LIMIT_DEPTH; i++)
    {
        vl = search_root(i);
        // End the search if it's a killer move
        if (vl > WIN_VALUE || vl < -WIN_VALUE)
        {
            break;
        }
        // End searching step if exceed the interval
        if (clock() - t > CLOCKS_PER_SEC * sec)
        {
            break;
        }
    }
}


