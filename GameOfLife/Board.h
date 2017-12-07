//
//  Board.h
//  GameOfLife
//
//  Created by Michael Peternell on 05.12.2017.
//  Copyright © 2017 Michael Peternell. All rights reserved.
//

#ifndef Board_h
#define Board_h

#include "gol-config.h"
#include <stdio.h>
#include <vector>
#include <string>


/// A "Game of Life" board. The board is not infinite, but it is wrap-around, i.e.
/// cells that fall out on the left side come in on the right side, etc.
/// @see https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life
class Board
{
public:
    /// Creates an empty (useless) board with zero size.
    Board() = default;
    /// Creates an empty board of a particular size
    Board(int rowCount, int colCount);
    
    inline int getRowCount() const { return m_rowCount; }
    inline int getColCount() const { return m_colCount; }
    
    /// @return true if the cell is alive, false otherwise.
    inline bool getCell(int row, int col) const { return m_cells[row*m_colCount + col] & 1; };
    void setCell(int row, int col, bool isAlive);
    
    /// Calculates the next generation. (Single-threaded, on the CPU.)
    void nextGeneration();
    void runSingleThreaded(int numberOfGenerations);

#if USE_OPENMP
    void runOpenMP(int numberOfGenerations, int numberOfThreads);
#endif
    
#if USE_OPENCL
    void runOpenCL(int numberOfGenerations);
#endif
    
    std::string toString() const;
    
    bool isVerbose() const { return m_verbose; };
    void setVerbose(bool v) { m_verbose = v; };
    
    friend bool operator==(const Board& b1, const Board& b2);
    
private:
    bool m_verbose = false;
    
    /// Runs on a slice of the data. Slicing is for OpenMP.
    void nextGeneration(char* oldCells, int rowBegin, int rowEnd);

    /// Sets cell to 'alive', assuming its dead
    void setCell_unsafe(int row, int col);
    /// Sets cell to 'dead', assuming its alive
    void clearCell_unsafe(int row, int col);
    
    inline int getIndex(int row, int col) const {
        return row * m_colCount + col;
    };
    
    int m_rowCount = 0;
    int m_colCount = 0;
    std::vector<char> m_cells;

    // About the memory layout:
    // Each element (char) of m_cells represents one cell on the board.
    // The lowest bit says if the cell is 'alive'.
    // Bits 1-4 is the neighbour-count. It says how many neighbours a cell has.
    // E.g. 000001001
    //      ........1 => cell is alive
    //      ....0100. => it has 4 neighbours
    // => the cell will die in the next generation because it is alive and has more than 3 neighbours.
};

bool operator==(const Board& b1, const Board& b2);
inline bool operator!=(const Board& b1, const Board& b2) {
    return !(b1 == b2);
}

#endif // Board_h
