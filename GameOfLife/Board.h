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


class Board
{
public:
    Board() = default;
    Board(int rowCount, int colCount);
    
    inline int getRowCount() const { return m_rowCount; }
    inline int getColCount() const { return m_colCount; }
    
    inline bool getCell(int row, int col) const { return m_cells[row*m_colCount + col] & 1; };
    void setCell(int row, int col, bool val);
    
    void nextGeneration();
    void runSingleThreaded(int numberOfGenerations);

#if USE_OPENMP
	void runOpenMP(int numberOfGenerations);
#endif
    
    std::string toString() const;
    
    friend bool operator==(const Board& b1, const Board& b2);
    
private:
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
