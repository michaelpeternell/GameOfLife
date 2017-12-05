//
//  Board.cpp
//  GameOfLife
//
//  Created by Michael Peternell on 05.12.2017.
//  Copyright © 2017 Michael Peternell. All rights reserved.
//

#include "Board.h"

Board::Board(int rowCount, int colCount) {
    m_rowCount = rowCount;
    m_colCount = colCount;
    m_cells.resize(rowCount*colCount, 0);
}

void Board::setCell(int row, int col, bool val) {
    val = val ? true : false;
    bool currentVal = getCell(row, col);
    if(val != currentVal) {
        if(val) {
            setCell_unsafe(row, col);
        } else {
            clearCell_unsafe(row, col);
        }
    }
}

void Board::setCell_unsafe(int row, int col) {
    int rowMinus1 = row == 0 ? m_rowCount-1 : row - 1;
    int rowPlus1 = row == (m_rowCount-1) ? 0 : row + 1;
    int colMinus1 = col == 0 ? m_colCount-1 : col - 1;
    int colPlus1 = col == (m_colCount-1) ? 0 : col + 1;
    
    m_cells[getIndex(rowMinus1, colMinus1)] += 2;
    m_cells[getIndex(rowMinus1, col)] += 2;
    m_cells[getIndex(rowMinus1, colPlus1)] += 2;
    m_cells[getIndex(row, colMinus1)] += 2;
    m_cells[getIndex(row, col)] |= 1;
    m_cells[getIndex(row, colPlus1)] += 2;
    m_cells[getIndex(rowPlus1, colMinus1)] += 2;
    m_cells[getIndex(rowPlus1, col)] += 2;
    m_cells[getIndex(rowPlus1, colPlus1)] += 2;
}

void Board::clearCell_unsafe(int row, int col) {
    int rowMinus1 = row == 0 ? m_rowCount-1 : row - 1;
    int rowPlus1 = row == (m_rowCount-1) ? 0 : row + 1;
    int colMinus1 = col == 0 ? m_colCount-1 : col - 1;
    int colPlus1 = col == (m_colCount-1) ? 0 : col + 1;
    
    m_cells[getIndex(rowMinus1, colMinus1)] -= 2;
    m_cells[getIndex(rowMinus1, col)] -= 2;
    m_cells[getIndex(rowMinus1, colPlus1)] -= 2;
    m_cells[getIndex(row, colMinus1)] -= 2;
    m_cells[getIndex(row, col)] &= 0xFE;
    m_cells[getIndex(row, colPlus1)] -= 2;
    m_cells[getIndex(rowPlus1, colMinus1)] -= 2;
    m_cells[getIndex(rowPlus1, col)] -= 2;
    m_cells[getIndex(rowPlus1, colPlus1)] -= 2;
}

void Board::nextGeneration()
{
    std::vector<char> oldCells = m_cells;
    int idx = 0;
    for(int row=0; row<m_rowCount; row++) {
        for(int col=0; col<m_colCount; col++, idx++) {
            char val = oldCells[idx];
            switch(val) {
                case 0: // not set, no neighbours
                case 2: // not set, 1 neighbour
                case 4: // not set, 2 neighbours
                case 5: // set, 2 neighbours
                case 7: // set, 3 neighbours
                case 8: // not set, 4 neighbours
                case 10: // not set, 5 neighbours
                case 12: // not set, 6 neighbours
                case 14: // not set, 7 neighbours
                case 16: // not set, 8 neighbours
                    break;
                    
                case 6: // not set, 3 neighbours
                    setCell_unsafe(row, col);
                    break;
                
                case 1: // set, no neighbours
                case 3: // set, 1 neighbour
                case 9: // set, 4 neighbours
                case 11: // set, 5 neighbours
                case 13: // set, 6 neighbours
                case 15: // set, 7 neighbours
                case 17: // set, 8 neighbours
                    clearCell_unsafe(row, col);
                    break;
                    
                default:
                    //printf("Oh noes\n");
                    break;
            }
        }
    }
}

void Board::runSingleThreaded(int numberOfGenerations) {
    for(int i=0; i<numberOfGenerations; i++) {
        nextGeneration();
    }
}

std::string Board::toString() const {
    std::string s = std::to_string(m_colCount) + "," + std::to_string(m_rowCount) + "\n";
    for(int rowIdx = 0; rowIdx < m_rowCount; rowIdx++) {
        std::string line;
        for(int colIdx = 0; colIdx < m_colCount; colIdx++) {
            if(getCell(rowIdx,colIdx)) {
                line += "x";
            } else {
                line += ".";
            }
        }
        s += line + "\n";
    }
    return s;
}

bool operator==(const Board& b1, const Board& b2) {
    return b1.getRowCount() == b2.getRowCount() && b1.m_cells == b2.m_cells;
}
