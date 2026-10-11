// ADDED FOR THE PUBLIC VERSION.
// The course provided a template header named "main.h" (arena interface, levels, game modes).
// It is not published here. This file declares only the two items the engine uses from it,
// so the team's source files build unchanged.
#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

// Size of a small board, and of the grid of small boards.
constexpr int TAILLE = 3;

// A move on the 9x9 grid: row and column from 0 to 8 ({-1, -1} when there is no move).
struct GameMove {
    int row;
    int col;
};

#endif  // MAIN_H_INCLUDED
