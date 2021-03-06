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
    char* ptr = &m_cells[row*m_colCount + col];

    int rAboveOffset, rBelowOffset, cLeftOffset, cRightOffset;
    if (row == 0) {
        rAboveOffset = (int)m_cells.size() - m_colCount;
    }
    else {
        rAboveOffset = -m_colCount;
    }
    if (row == (m_rowCount - 1)) {
        rBelowOffset = -(int)m_cells.size() + m_colCount;
    }
    else {
        rBelowOffset = +m_colCount;
    }
    if (col == 0) {
        cLeftOffset = m_colCount - 1;
    }
    else {
        cLeftOffset = -1;
    }
    if (col == (m_colCount - 1)) {
        cRightOffset = -m_colCount + 1;
    }
    else {
        cRightOffset = +1;
    }

    ptr[rAboveOffset + cLeftOffset] += 2;
    ptr[rAboveOffset] += 2;
    ptr[rAboveOffset + cRightOffset] += 2;
    ptr[cLeftOffset] += 2;
    ptr[0] |= 1;
    ptr[cRightOffset] += 2;
    ptr[rBelowOffset + cLeftOffset] += 2;
    ptr[rBelowOffset] += 2;
    ptr[rBelowOffset + cRightOffset] += 2;
}

void Board::clearCell_unsafe(int row, int col) {
    char* ptr = &m_cells[row*m_colCount + col];

    int rAboveOffset, rBelowOffset, cLeftOffset, cRightOffset;
    if (row == 0) {
        rAboveOffset = (int)m_cells.size() - m_colCount;
    }
    else {
        rAboveOffset = -m_colCount;
    }
    if (row == (m_rowCount - 1)) {
        rBelowOffset = -(int)m_cells.size() + m_colCount;
    }
    else {
        rBelowOffset = +m_colCount;
    }
    if (col == 0) {
        cLeftOffset = m_colCount - 1;
    }
    else {
        cLeftOffset = -1;
    }
    if (col == (m_colCount - 1)) {
        cRightOffset = -m_colCount + 1;
    }
    else {
        cRightOffset = +1;
    }

    ptr[rAboveOffset + cLeftOffset] -= 2;
    ptr[rAboveOffset] -= 2;
    ptr[rAboveOffset + cRightOffset] -= 2;
    ptr[cLeftOffset] -= 2;
    ptr[0] &= 0xFE;
    ptr[cRightOffset] -= 2;
    ptr[rBelowOffset + cLeftOffset] -= 2;
    ptr[rBelowOffset] -= 2;
    ptr[rBelowOffset + cRightOffset] -= 2;
}

void Board::nextGeneration()
{
    std::vector<char> oldCells = m_cells;
    nextGeneration(&oldCells[0], 0, m_rowCount);
}

void Board::nextGeneration(char* oldCells, int rowBegin, int rowEnd)
{
    int idx = rowBegin * m_colCount;
    for (int row = rowBegin; row<rowEnd; row++) {
        for (int col = 0; col<m_colCount; col++, idx++) {
            char val = oldCells[idx];
            switch (val) {
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
            }
        }
    }
}

void Board::runSingleThreaded(int numberOfGenerations) {
    std::vector<char> oldCells = m_cells;
    size_t size = oldCells.size();
    for(int i=0; i<numberOfGenerations; i++) {
        if (i != 0) {
            memcpy(&oldCells[0], &m_cells[0], size);
        }
        nextGeneration(&oldCells[0], 0, m_rowCount);
    }
}

#if USE_OPENMP
void Board::runOpenMP(int numberOfGenerations, int numberOfThreads)
{
    if (numberOfGenerations <= 0) {
        return;
    }

    if (m_rowCount < 10) {
        printf("Warning: Refusing to run OpenMP for a board that has less than 10 rows.\n"
            "This board has %d rows and %d columns. Silently running single-threaded instead.\n", m_rowCount, m_colCount);
        runSingleThreaded(numberOfGenerations);
        return;
    }

    if (m_rowCount < (numberOfThreads*3 + 2)) {
        int newNumberOfThreads = m_rowCount / 4;
        printf("Warning: Refusing to run OpenMP with %d threads when having just %d rows. Will use %d threads instead.\n", numberOfThreads, m_rowCount, newNumberOfThreads);
        numberOfThreads = newNumberOfThreads;
    }

    std::vector<int> slicesBegin;
    std::vector<int> slicesEnd;
    std::vector<int> edgeRows;

    for (int i = 0; i < numberOfThreads; i++) {
        slicesBegin.push_back(m_rowCount * i / numberOfThreads + 2);
    }
    for (int i = 0; i < numberOfThreads -1; i++) {
        slicesEnd.push_back(slicesBegin[i + 1] - 2);
    }
    slicesEnd.push_back(m_rowCount);
    for (int i = 0; i < numberOfThreads; i++) {
        int row = slicesBegin[i];
        edgeRows.push_back(row - 2);
        edgeRows.push_back(row - 1);
    }

    std::vector<char> oldCells = m_cells;
    size_t size = oldCells.size();
    for (int gIdx = 0; gIdx < numberOfGenerations; gIdx++) {
        if (gIdx != 0) {
            memcpy(&oldCells[0], &m_cells[0], size);
        }

        #pragma omp parallel for
        for (int i = 0; i < numberOfThreads; i++) {
            nextGeneration(&oldCells[0], slicesBegin[i], slicesEnd[i]);
        }

        for (auto row : edgeRows) {
            nextGeneration(&oldCells[0], row, row + 1);
        }
    }
}
#endif

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
