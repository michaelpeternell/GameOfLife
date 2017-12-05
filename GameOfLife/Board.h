//
//  Board.h
//  GameOfLife
//
//  Created by Michael Peternell on 05.12.2017.
//  Copyright © 2017 Michael Peternell. All rights reserved.
//

#ifndef Board_h
#define Board_h

#include <stdio.h>
#include <vector>
#include <string>

#define BOARD_ALIVE_BIT 0x01
#define BOARD_NEIGH_MASK 0x0E

class Board
{
public:
    Board() = default;
    Board(int rowCount, int colCount);
    
    inline int getRowCount() const { return m_rowCount; }
    inline int getColCount() const { return m_colCount; }
    
    inline bool getCell(int row, int col) const { return m_cells[row*m_colCount + col] & 1; };
    void setCell(int row, int col, bool val);
    
    /// Single-threaded: make next generation
    void nextGeneration();
    
    void runSingleThreaded(int numberOfGenerations);
    
    std::string toString() const;
    
    friend bool operator==(const Board& b1, const Board& b2);
    
private:
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
};

bool operator==(const Board& b1, const Board& b2);
inline bool operator!=(const Board& b1, const Board& b2) {
    return !(b1 == b2);
}

#endif // Board_h
