//
//  main.cpp
//  GameOfLife
//
//  Created by Michael Peternell on 5.12.2017.
//  Copyright © 2017 Michael Peternell. All rights reserved.
//

#include "Board.h"
#include <stdlib.h>
#include <chrono>

using namespace std::chrono;

static void die(const std::string& str) {
    puts((str+"\n").c_str());
    abort();
}

Board loadBoard(const std::string& path) {
    FILE *f = fopen(path.c_str(), "r");
    if(!f) {
        die("fopen read");
    }
    int rowCount=-1, colCount=-1;
    fscanf(f, "%d,%d\n", &colCount, &rowCount);
    if(rowCount < 1 || colCount < 1) {
        die("file read rowCount colCount");
    }
    char* buf = (char*)malloc(colCount+5);
    if(buf == nullptr) {
        die("malloc");
    }
    Board board = Board(rowCount, colCount);
    for(int rowIdx=0; rowIdx<rowCount; rowIdx++) {
        char* result = fgets(buf, colCount+3, f);
        if(result == nullptr) {
            die("fgets");
        }
        for(int colIdx=0; colIdx<colCount; colIdx++) {
            if(buf[colIdx] == 'x' || buf[colIdx] == 'X') {
                board.setCell(rowIdx, colIdx, true);
            }
        }
    }
    return board;
}

int main(int argc, const char * argv[])
{
    Board b_in = loadBoard("/Users/michael/Dropbox/FH/GameEngineering/Semester1/EPR/GameOfLife/step1000_in_250generations/random10000_in.gol");
    Board b_verify = loadBoard("/Users/michael/Dropbox/FH/GameEngineering/Semester1/EPR/GameOfLife/step1000_out_250generations/random10000_out.gol");
    
    high_resolution_clock::time_point beginCalc = high_resolution_clock::now();
    for(int i=0; i<250; i++) {
        b_in.nextGeneration();
    }
    high_resolution_clock::time_point endCalc = high_resolution_clock::now();
    duration<double> calcTime = duration_cast<duration<double>>(endCalc - beginCalc);
    printf("Time it took: %.3fs\n", calcTime.count());
    if(b_in == b_verify) {
        printf("Passt.\n");
    } else {
        printf("Ohjee\n");
    }
    return 0;
}
