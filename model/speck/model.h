#pragma once

#include <stddef.h>

#include "../model.h"

// floating-point
const vertex_position speck_position[] = {
  {-0.5, -0.5, 0.5},
  {-0.5, 0.5, 0.5},
  {-0.5, -0.5, -0.5},
  {-0.5, 0.5, -0.5},
  {0.5, -0.5, 0.5},
  {0.5, 0.5, 0.5},
  {0.5, -0.5, -0.5},
  {0.5, 0.5, -0.5},
  {-0.572933, -0.572933, -0.29665},
  {-0.609568, -0.609568, 0.0},
  {-0.572933, -0.572933, 0.29665},
  {-0.572933, -0.29665, 0.572933},
  {-0.609568, 0.0, 0.609568},
  {-0.572933, 0.29665, 0.572933},
  {-0.572933, 0.572933, 0.29665},
  {-0.609568, 0.609568, 0.0},
  {-0.572933, 0.572933, -0.29665},
  {-0.572933, 0.29665, -0.572933},
  {-0.609568, 0.0, -0.609568},
  {-0.572933, -0.29665, -0.572933},
  {0.29665, -0.572933, -0.572933},
  {0.0, -0.609568, -0.609568},
  {-0.29665, -0.572933, -0.572933},
  {-0.29665, 0.572933, -0.572933},
  {0.0, 0.609568, -0.609568},
  {0.29665, 0.572933, -0.572933},
  {0.572933, 0.29665, -0.572933},
  {0.609568, 0.0, -0.609568},
  {0.572933, -0.29665, -0.572933},
  {0.572933, -0.572933, 0.29665},
  {0.609568, -0.609568, 0.0},
  {0.572933, -0.572933, -0.29665},
  {0.572933, 0.572933, -0.29665},
  {0.609568, 0.609568, 0.0},
  {0.572933, 0.572933, 0.29665},
  {0.572933, 0.29665, 0.572933},
  {0.609568, 0.0, 0.609568},
  {0.572933, -0.29665, 0.572933},
  {-0.29665, -0.572933, 0.572933},
  {0.0, -0.609568, 0.609568},
  {0.29665, -0.572933, 0.572933},
  {0.29665, 0.572933, 0.572933},
  {0.0, 0.609568, 0.609568},
  {-0.29665, 0.572933, 0.572933},
  {-0.72899, -0.316157, 0.316157},
  {-0.781829, 0.0, 0.33314},
  {-0.72899, 0.316157, 0.316157},
  {-0.781829, -0.33314, 0.0},
  {-0.839506, 0.0, 0.0},
  {-0.781829, 0.33314, 0.0},
  {-0.72899, -0.316157, -0.316157},
  {-0.781829, 0.0, -0.33314},
  {-0.72899, 0.316157, -0.316157},
  {-0.316157, -0.316157, -0.72899},
  {-0.33314, 0.0, -0.781829},
  {-0.316157, 0.316157, -0.72899},
  {0.0, -0.33314, -0.781829},
  {0.0, 0.0, -0.839506},
  {0.0, 0.33314, -0.781829},
  {0.316157, -0.316157, -0.72899},
  {0.33314, 0.0, -0.781829},
  {0.316157, 0.316157, -0.72899},
  {0.72899, -0.316157, -0.316157},
  {0.781829, 0.0, -0.33314},
  {0.72899, 0.316157, -0.316157},
  {0.781829, -0.33314, 0.0},
  {0.839506, 0.0, 0.0},
  {0.781829, 0.33314, 0.0},
  {0.72899, -0.316157, 0.316157},
  {0.781829, 0.0, 0.33314},
  {0.72899, 0.316157, 0.316157},
  {0.316157, -0.316157, 0.72899},
  {0.33314, 0.0, 0.781829},
  {0.316157, 0.316157, 0.72899},
  {0.0, -0.33314, 0.781829},
  {0.0, 0.0, 0.839506},
  {0.0, 0.33314, 0.781829},
  {-0.316157, -0.316157, 0.72899},
  {-0.33314, 0.0, 0.781829},
  {-0.316157, 0.316157, 0.72899},
  {-0.316157, -0.72899, -0.316157},
  {0.0, -0.781829, -0.33314},
  {0.316157, -0.72899, -0.316157},
  {-0.33314, -0.781829, 0.0},
  {0.0, -0.839506, 0.0},
  {0.33314, -0.781829, 0.0},
  {-0.316157, -0.72899, 0.316157},
  {0.0, -0.781829, 0.33314},
  {0.316157, -0.72899, 0.316157},
  {0.316157, 0.72899, -0.316157},
  {0.0, 0.781829, -0.33314},
  {-0.316157, 0.72899, -0.316157},
  {0.33314, 0.781829, 0.0},
  {0.0, 0.839506, 0.0},
  {-0.33314, 0.781829, 0.0},
  {0.316157, 0.72899, 0.316157},
  {0.0, 0.781829, 0.33314},
  {-0.316157, 0.72899, 0.316157},
};

// floating-point
const vertex_texture speck_texture[] = {
  {0.25, 0.25},
  {0.5, 0.25},
  {0.5, 0.5},
  {0.25, 0.5},
  {0.75, 0.25},
  {0.75, 0.5},
  {0.5, 0.75},
  {0.25, 0.75},
  {0.75, 0.75},
  {0.0, 0.0},
  {0.25, 0.0},
  {0.0, 0.25},
  {0.5, 0.0},
  {0.75, 0.0},
  {1.0, 0.0},
  {1.0, 0.25},
  {1.0, 0.5},
  {1.0, 0.75},
  {1.0, 1.0},
  {0.75, 1.0},
  {0.5, 1.0},
  {0.25, 1.0},
  {0.0, 0.75},
  {0.0, 1.0},
  {0.0, 0.5},
};

// floating-point
const vertex_normal speck_normal[] = {
  {-0.57735, -0.57735, 0.57735},
  {-0.57735, 0.57735, 0.57735},
  {-0.57735, -0.57735, -0.57735},
  {-0.57735, 0.57735, -0.57735},
  {0.57735, -0.57735, 0.57735},
  {0.57735, 0.57735, 0.57735},
  {0.57735, -0.57735, -0.57735},
  {0.57735, 0.57735, -0.57735},
  {-0.67094, -0.67094, -0.315719},
  {-0.707107, -0.707107, 0.0},
  {-0.67094, -0.67094, 0.315719},
  {-0.67094, -0.315719, 0.67094},
  {-0.707107, 0.0, 0.707107},
  {-0.67094, 0.315719, 0.67094},
  {-0.67094, 0.67094, 0.315719},
  {-0.707107, 0.707107, 0.0},
  {-0.67094, 0.67094, -0.315719},
  {-0.67094, 0.315719, -0.67094},
  {-0.707107, 0.0, -0.707107},
  {-0.67094, -0.315719, -0.67094},
  {0.315719, -0.67094, -0.67094},
  {0.0, -0.707107, -0.707107},
  {-0.315719, -0.67094, -0.67094},
  {-0.315719, 0.67094, -0.67094},
  {0.0, 0.707107, -0.707107},
  {0.315719, 0.67094, -0.67094},
  {0.67094, 0.315719, -0.67094},
  {0.707107, 0.0, -0.707107},
  {0.67094, -0.315719, -0.67094},
  {0.67094, -0.67094, 0.315719},
  {0.707107, -0.707107, 0.0},
  {0.67094, -0.67094, -0.315719},
  {0.67094, 0.67094, -0.315719},
  {0.707107, 0.707107, 0.0},
  {0.67094, 0.67094, 0.315719},
  {0.67094, 0.315719, 0.67094},
  {0.707107, 0.0, 0.707107},
  {0.67094, -0.315719, 0.67094},
  {-0.315719, -0.67094, 0.67094},
  {0.0, -0.707107, 0.707107},
  {0.315719, -0.67094, 0.67094},
  {0.315719, 0.67094, 0.67094},
  {0.0, 0.707107, 0.707107},
  {-0.315719, 0.67094, 0.67094},
  {-0.879729, -0.336211, 0.336211},
  {-0.934488, 0.0, 0.355995},
  {-0.879729, 0.336211, 0.336211},
  {-0.934488, -0.355995, 0.0},
  {-1.0, 0.0, 0.0},
  {-0.934488, 0.355995, 0.0},
  {-0.879729, -0.336211, -0.336211},
  {-0.934488, 0.0, -0.355995},
  {-0.879729, 0.336211, -0.336211},
  {-0.336211, -0.336211, -0.879729},
  {-0.355995, 0.0, -0.934488},
  {-0.336211, 0.336211, -0.879729},
  {0.0, -0.355995, -0.934488},
  {0.0, 0.0, -1.0},
  {0.0, 0.355995, -0.934488},
  {0.336211, -0.336211, -0.879729},
  {0.355995, 0.0, -0.934488},
  {0.336211, 0.336211, -0.879729},
  {0.879729, -0.336211, -0.336211},
  {0.934488, 0.0, -0.355995},
  {0.879729, 0.336211, -0.336211},
  {0.934488, -0.355995, 0.0},
  {1.0, 0.0, 0.0},
  {0.934488, 0.355995, 0.0},
  {0.879729, -0.336211, 0.336211},
  {0.934488, 0.0, 0.355995},
  {0.879729, 0.336211, 0.336211},
  {0.336211, -0.336211, 0.879729},
  {0.355995, 0.0, 0.934488},
  {0.336211, 0.336211, 0.879729},
  {0.0, -0.355995, 0.934488},
  {0.0, 0.0, 1.0},
  {0.0, 0.355995, 0.934488},
  {-0.336211, -0.336211, 0.879729},
  {-0.355995, 0.0, 0.934488},
  {-0.336211, 0.336211, 0.879729},
  {-0.336211, -0.879729, -0.336211},
  {0.0, -0.934488, -0.355995},
  {0.336211, -0.879729, -0.336211},
  {-0.355995, -0.934488, 0.0},
  {0.0, -1.0, 0.0},
  {0.355995, -0.934488, 0.0},
  {-0.336211, -0.879729, 0.336211},
  {0.0, -0.934488, 0.355995},
  {0.336211, -0.879729, 0.336211},
  {0.336211, 0.879729, -0.336211},
  {0.0, 0.934488, -0.355995},
  {-0.336211, 0.879729, -0.336211},
  {0.355995, 0.934488, 0.0},
  {0.0, 1.0, 0.0},
  {-0.355995, 0.934488, 0.0},
  {0.336211, 0.879729, 0.336211},
  {0.0, 0.934488, 0.355995},
  {-0.336211, 0.879729, 0.336211},
};

union quadrilateral speck_Cube_quadrilateral[] = {
  { .v = {
    {44, 0, 44},
    {45, 1, 45},
    {48, 2, 48},
    {47, 3, 47},
  }},
  { .v = {
    {45, 1, 45},
    {46, 4, 46},
    {49, 5, 49},
    {48, 2, 48},
  }},
  { .v = {
    {47, 3, 47},
    {48, 2, 48},
    {51, 6, 51},
    {50, 7, 50},
  }},
  { .v = {
    {48, 2, 48},
    {49, 5, 49},
    {52, 8, 52},
    {51, 6, 51},
  }},
  { .v = {
    {0, 9, 0},
    {11, 10, 11},
    {44, 0, 44},
    {10, 11, 10},
  }},
  { .v = {
    {11, 10, 11},
    {12, 12, 12},
    {45, 1, 45},
    {44, 0, 44},
  }},
  { .v = {
    {12, 12, 12},
    {13, 13, 13},
    {46, 4, 46},
    {45, 1, 45},
  }},
  { .v = {
    {13, 13, 13},
    {1, 14, 1},
    {14, 15, 14},
    {46, 4, 46},
  }},
  { .v = {
    {46, 4, 46},
    {14, 15, 14},
    {15, 16, 15},
    {49, 5, 49},
  }},
  { .v = {
    {49, 5, 49},
    {15, 16, 15},
    {16, 17, 16},
    {52, 8, 52},
  }},
  { .v = {
    {52, 8, 52},
    {16, 17, 16},
    {3, 18, 3},
    {17, 19, 17},
  }},
  { .v = {
    {51, 6, 51},
    {52, 8, 52},
    {17, 19, 17},
    {18, 20, 18},
  }},
  { .v = {
    {50, 7, 50},
    {51, 6, 51},
    {18, 20, 18},
    {19, 21, 19},
  }},
  { .v = {
    {8, 22, 8},
    {50, 7, 50},
    {19, 21, 19},
    {2, 23, 2},
  }},
  { .v = {
    {9, 24, 9},
    {47, 3, 47},
    {50, 7, 50},
    {8, 22, 8},
  }},
  { .v = {
    {10, 11, 10},
    {44, 0, 44},
    {47, 3, 47},
    {9, 24, 9},
  }},
  { .v = {
    {53, 0, 53},
    {54, 1, 54},
    {57, 2, 57},
    {56, 3, 56},
  }},
  { .v = {
    {54, 1, 54},
    {55, 4, 55},
    {58, 5, 58},
    {57, 2, 57},
  }},
  { .v = {
    {56, 3, 56},
    {57, 2, 57},
    {60, 6, 60},
    {59, 7, 59},
  }},
  { .v = {
    {57, 2, 57},
    {58, 5, 58},
    {61, 8, 61},
    {60, 6, 60},
  }},
  { .v = {
    {2, 9, 2},
    {19, 10, 19},
    {53, 0, 53},
    {22, 11, 22},
  }},
  { .v = {
    {19, 10, 19},
    {18, 12, 18},
    {54, 1, 54},
    {53, 0, 53},
  }},
  { .v = {
    {18, 12, 18},
    {17, 13, 17},
    {55, 4, 55},
    {54, 1, 54},
  }},
  { .v = {
    {17, 13, 17},
    {3, 14, 3},
    {23, 15, 23},
    {55, 4, 55},
  }},
  { .v = {
    {55, 4, 55},
    {23, 15, 23},
    {24, 16, 24},
    {58, 5, 58},
  }},
  { .v = {
    {58, 5, 58},
    {24, 16, 24},
    {25, 17, 25},
    {61, 8, 61},
  }},
  { .v = {
    {61, 8, 61},
    {25, 17, 25},
    {7, 18, 7},
    {26, 19, 26},
  }},
  { .v = {
    {60, 6, 60},
    {61, 8, 61},
    {26, 19, 26},
    {27, 20, 27},
  }},
  { .v = {
    {59, 7, 59},
    {60, 6, 60},
    {27, 20, 27},
    {28, 21, 28},
  }},
  { .v = {
    {20, 22, 20},
    {59, 7, 59},
    {28, 21, 28},
    {6, 23, 6},
  }},
  { .v = {
    {21, 24, 21},
    {56, 3, 56},
    {59, 7, 59},
    {20, 22, 20},
  }},
  { .v = {
    {22, 11, 22},
    {53, 0, 53},
    {56, 3, 56},
    {21, 24, 21},
  }},
  { .v = {
    {62, 0, 62},
    {63, 1, 63},
    {66, 2, 66},
    {65, 3, 65},
  }},
  { .v = {
    {63, 1, 63},
    {64, 4, 64},
    {67, 5, 67},
    {66, 2, 66},
  }},
  { .v = {
    {65, 3, 65},
    {66, 2, 66},
    {69, 6, 69},
    {68, 7, 68},
  }},
  { .v = {
    {66, 2, 66},
    {67, 5, 67},
    {70, 8, 70},
    {69, 6, 69},
  }},
  { .v = {
    {6, 9, 6},
    {28, 10, 28},
    {62, 0, 62},
    {31, 11, 31},
  }},
  { .v = {
    {28, 10, 28},
    {27, 12, 27},
    {63, 1, 63},
    {62, 0, 62},
  }},
  { .v = {
    {27, 12, 27},
    {26, 13, 26},
    {64, 4, 64},
    {63, 1, 63},
  }},
  { .v = {
    {26, 13, 26},
    {7, 14, 7},
    {32, 15, 32},
    {64, 4, 64},
  }},
  { .v = {
    {64, 4, 64},
    {32, 15, 32},
    {33, 16, 33},
    {67, 5, 67},
  }},
  { .v = {
    {67, 5, 67},
    {33, 16, 33},
    {34, 17, 34},
    {70, 8, 70},
  }},
  { .v = {
    {70, 8, 70},
    {34, 17, 34},
    {5, 18, 5},
    {35, 19, 35},
  }},
  { .v = {
    {69, 6, 69},
    {70, 8, 70},
    {35, 19, 35},
    {36, 20, 36},
  }},
  { .v = {
    {68, 7, 68},
    {69, 6, 69},
    {36, 20, 36},
    {37, 21, 37},
  }},
  { .v = {
    {29, 22, 29},
    {68, 7, 68},
    {37, 21, 37},
    {4, 23, 4},
  }},
  { .v = {
    {30, 24, 30},
    {65, 3, 65},
    {68, 7, 68},
    {29, 22, 29},
  }},
  { .v = {
    {31, 11, 31},
    {62, 0, 62},
    {65, 3, 65},
    {30, 24, 30},
  }},
  { .v = {
    {71, 0, 71},
    {72, 1, 72},
    {75, 2, 75},
    {74, 3, 74},
  }},
  { .v = {
    {72, 1, 72},
    {73, 4, 73},
    {76, 5, 76},
    {75, 2, 75},
  }},
  { .v = {
    {74, 3, 74},
    {75, 2, 75},
    {78, 6, 78},
    {77, 7, 77},
  }},
  { .v = {
    {75, 2, 75},
    {76, 5, 76},
    {79, 8, 79},
    {78, 6, 78},
  }},
  { .v = {
    {4, 9, 4},
    {37, 10, 37},
    {71, 0, 71},
    {40, 11, 40},
  }},
  { .v = {
    {37, 10, 37},
    {36, 12, 36},
    {72, 1, 72},
    {71, 0, 71},
  }},
  { .v = {
    {36, 12, 36},
    {35, 13, 35},
    {73, 4, 73},
    {72, 1, 72},
  }},
  { .v = {
    {35, 13, 35},
    {5, 14, 5},
    {41, 15, 41},
    {73, 4, 73},
  }},
  { .v = {
    {73, 4, 73},
    {41, 15, 41},
    {42, 16, 42},
    {76, 5, 76},
  }},
  { .v = {
    {76, 5, 76},
    {42, 16, 42},
    {43, 17, 43},
    {79, 8, 79},
  }},
  { .v = {
    {79, 8, 79},
    {43, 17, 43},
    {1, 18, 1},
    {13, 19, 13},
  }},
  { .v = {
    {78, 6, 78},
    {79, 8, 79},
    {13, 19, 13},
    {12, 20, 12},
  }},
  { .v = {
    {77, 7, 77},
    {78, 6, 78},
    {12, 20, 12},
    {11, 21, 11},
  }},
  { .v = {
    {38, 22, 38},
    {77, 7, 77},
    {11, 21, 11},
    {0, 23, 0},
  }},
  { .v = {
    {39, 24, 39},
    {74, 3, 74},
    {77, 7, 77},
    {38, 22, 38},
  }},
  { .v = {
    {40, 11, 40},
    {71, 0, 71},
    {74, 3, 74},
    {39, 24, 39},
  }},
  { .v = {
    {80, 0, 80},
    {81, 1, 81},
    {84, 2, 84},
    {83, 3, 83},
  }},
  { .v = {
    {81, 1, 81},
    {82, 4, 82},
    {85, 5, 85},
    {84, 2, 84},
  }},
  { .v = {
    {83, 3, 83},
    {84, 2, 84},
    {87, 6, 87},
    {86, 7, 86},
  }},
  { .v = {
    {84, 2, 84},
    {85, 5, 85},
    {88, 8, 88},
    {87, 6, 87},
  }},
  { .v = {
    {2, 9, 2},
    {22, 10, 22},
    {80, 0, 80},
    {8, 11, 8},
  }},
  { .v = {
    {22, 10, 22},
    {21, 12, 21},
    {81, 1, 81},
    {80, 0, 80},
  }},
  { .v = {
    {21, 12, 21},
    {20, 13, 20},
    {82, 4, 82},
    {81, 1, 81},
  }},
  { .v = {
    {20, 13, 20},
    {6, 14, 6},
    {31, 15, 31},
    {82, 4, 82},
  }},
  { .v = {
    {82, 4, 82},
    {31, 15, 31},
    {30, 16, 30},
    {85, 5, 85},
  }},
  { .v = {
    {85, 5, 85},
    {30, 16, 30},
    {29, 17, 29},
    {88, 8, 88},
  }},
  { .v = {
    {88, 8, 88},
    {29, 17, 29},
    {4, 18, 4},
    {40, 19, 40},
  }},
  { .v = {
    {87, 6, 87},
    {88, 8, 88},
    {40, 19, 40},
    {39, 20, 39},
  }},
  { .v = {
    {86, 7, 86},
    {87, 6, 87},
    {39, 20, 39},
    {38, 21, 38},
  }},
  { .v = {
    {10, 22, 10},
    {86, 7, 86},
    {38, 21, 38},
    {0, 23, 0},
  }},
  { .v = {
    {9, 24, 9},
    {83, 3, 83},
    {86, 7, 86},
    {10, 22, 10},
  }},
  { .v = {
    {8, 11, 8},
    {80, 0, 80},
    {83, 3, 83},
    {9, 24, 9},
  }},
  { .v = {
    {89, 0, 89},
    {90, 1, 90},
    {93, 2, 93},
    {92, 3, 92},
  }},
  { .v = {
    {90, 1, 90},
    {91, 4, 91},
    {94, 5, 94},
    {93, 2, 93},
  }},
  { .v = {
    {92, 3, 92},
    {93, 2, 93},
    {96, 6, 96},
    {95, 7, 95},
  }},
  { .v = {
    {93, 2, 93},
    {94, 5, 94},
    {97, 8, 97},
    {96, 6, 96},
  }},
  { .v = {
    {7, 9, 7},
    {25, 10, 25},
    {89, 0, 89},
    {32, 11, 32},
  }},
  { .v = {
    {25, 10, 25},
    {24, 12, 24},
    {90, 1, 90},
    {89, 0, 89},
  }},
  { .v = {
    {24, 12, 24},
    {23, 13, 23},
    {91, 4, 91},
    {90, 1, 90},
  }},
  { .v = {
    {23, 13, 23},
    {3, 14, 3},
    {16, 15, 16},
    {91, 4, 91},
  }},
  { .v = {
    {91, 4, 91},
    {16, 15, 16},
    {15, 16, 15},
    {94, 5, 94},
  }},
  { .v = {
    {94, 5, 94},
    {15, 16, 15},
    {14, 17, 14},
    {97, 8, 97},
  }},
  { .v = {
    {97, 8, 97},
    {14, 17, 14},
    {1, 18, 1},
    {43, 19, 43},
  }},
  { .v = {
    {96, 6, 96},
    {97, 8, 97},
    {43, 19, 43},
    {42, 20, 42},
  }},
  { .v = {
    {95, 7, 95},
    {96, 6, 96},
    {42, 20, 42},
    {41, 21, 41},
  }},
  { .v = {
    {34, 22, 34},
    {95, 7, 95},
    {41, 21, 41},
    {5, 23, 5},
  }},
  { .v = {
    {33, 24, 33},
    {92, 3, 92},
    {95, 7, 95},
    {34, 22, 34},
  }},
  { .v = {
    {32, 11, 32},
    {89, 0, 89},
    {92, 3, 92},
    {33, 24, 33},
  }},
};

const struct object speck_Cube = {
  .triangle = NULL,
  .quadrilateral = &speck_Cube_quadrilateral[0],
  .triangle_count = 0,
  .quadrilateral_count = 96,
  .material = speck_matSpeck,
};

const struct object speck_Cube_white = {
  .triangle = NULL,
  .quadrilateral = &speck_Cube_quadrilateral[0],
  .triangle_count = 0,
  .quadrilateral_count = 96,
  .material = speck_white,
};

const struct object * speck_object_list[] = {
  &speck_Cube,
};

const struct model speck_model = {
  .position = &speck_position[0],
  .texture = &speck_texture[0],
  .normal = &speck_normal[0],
  .object = &speck_object_list[0],
  .object_count = 1,
};
