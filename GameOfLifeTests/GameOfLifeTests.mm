//
//  GameOfLifeTests.m
//  GameOfLifeTests
//
//  Created by Michael Peternell on 05.12.2017.
//  Copyright © 2017 Michael Peternell. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "Board.h"

@interface GameOfLifeTests : XCTestCase

@end



@implementation GameOfLifeTests

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testEmptyBoard {
    Board b1;
    Board b2;
    XCTAssert(b1 == b2);
    XCTAssert(!(b1 != b2));
    
    b1 = Board(5,3);
    b2 = Board(5,3);
    XCTAssert(b1 == b2);
    
    b2 = Board(5,5);
    XCTAssert(b1 != b2);
}

- (void)testSimpleGenerations {
    const Board empty6x6 = Board(6,6);
    Board b1, b2;
    
    b1 = empty6x6;
    
    b1.setCell(0, 0, true);
    XCTAssert(b1 != empty6x6);
    b1.nextGeneration();
    XCTAssert(b1 == empty6x6);
    
    // Square pattern shouldn't change
    //
    //  xx
    //  xx
    //
    
    b1.setCell(1, 1, true);
    b1.setCell(1, 2, true);
    b1.setCell(2, 1, true);
    b1.setCell(2, 2, true);
    b2 = b1;
    XCTAssert(b1 == b2);
    b1.nextGeneration();
    XCTAssert(b1 == b2);
    XCTAssert(!(b1 != b2));
    
    // Two elements below each other should die
    //
    //  x
    //  x
    //
    
    b1 = empty6x6;
    b1.setCell(1, 1, true);
    b1.setCell(2, 1, true);
    b1.nextGeneration();
    XCTAssert(b1 == empty6x6, "b1 is %s", b1.toString().c_str());
    
    // Test new element comes alive to form square pattern
    //
    //  xx
    //  x
    //
    
    b1 = empty6x6;
    b1.setCell(1, 1, true);
    b1.setCell(1, 2, true);
    b1.setCell(2, 1, true);
    b2 = b1;
    b2.setCell(2, 2, true);
    
    XCTAssert(b1 != b2);
    b1.nextGeneration();
    XCTAssert(b1 == b2);
    b1.nextGeneration();
    XCTAssert(b1 == b2);
    b1.nextGeneration();
    XCTAssert(!(b1 != b2));
    
    // Test crowded element dies
    //
    //  xxx
    //  xx
    //
    //    =>
    //   x
    //  x x
    //  x x
    //
    
    b1 = empty6x6;
    b1.setCell(1, 1, true);
    b1.setCell(1, 2, true);
    b1.setCell(1, 3, true);
    b1.setCell(2, 1, true);
    b1.setCell(2, 2, true);
    b2 = empty6x6;
    b2.setCell(0, 2, true);
    b2.setCell(1, 1, true);
    b2.setCell(1, 3, true);
    b2.setCell(2, 1, true);
    b2.setCell(2, 3, true);
    b1.nextGeneration();
    XCTAssert(b1 == b2);
    
    // Test fully overcrowded crowd dies immediately
    
    b1 = empty6x6;
    for(int rowIdx=0; rowIdx<b1.getRowCount(); rowIdx++) {
        for(int colIdx=0; colIdx<b1.getColCount(); colIdx++) {
            b1.setCell(rowIdx, colIdx, true);
        }
    }
    b1.nextGeneration();
    XCTAssert(b1 == empty6x6);
}

- (void)testWrapAround
{
    const Board empty6x6 = Board(6,6);
    Board b1, b2;
    
    b1 = empty6x6;
    b1.setCell(0, 0, true);
    b1.setCell(1, 0, true);
    b1.setCell(0, 5, true);
    b1.setCell(1, 5, true);
    b2 = b1;
    b1.nextGeneration();
    XCTAssert(b1 == b2);
    b1.nextGeneration();
    XCTAssert(b1 == b2);
}

#if USE_OPENCL
- (void)testOpenCL
{
    const Board empty6x6 = Board(6,6);
    Board b1, b2;
    
    b1 = empty6x6;
    b1.setCell(0, 0, true);
    b1.setCell(1, 0, true);
    b1.setCell(0, 5, true);
    b1.setCell(1, 5, true);
    b2 = b1;
    
    b1.runOpenCL(1);
    if(b1 != b2) {
        printf("b1 = %s\n", b1.toString().c_str());
    }
    XCTAssert(b1 == b2);
    b1.runOpenCL(2);
    XCTAssert(b1 == b2);
    b1.runOpenCL(20);
    XCTAssert(b1 == b2);
    
    b1.setCell(0, 0, false);
    b1.runOpenCL(1);
    if(b1 != b2) {
        printf("b1 = %s\n", b1.toString().c_str());
    }
    XCTAssert(b1 == b2);
}

static Board makeRandomBoard(int rows, int cols) {
    Board b = Board(rows, cols);
    for(int rIdx = 0; rIdx < rows; rIdx++) {
        for(int cIdx = 0; cIdx < cols; cIdx++) {
            if(arc4random() % 3 == 1) {
                b.setCell(rIdx, cIdx, true);
            }
        }
    }
    return b;
}

static Board diffBoards(const Board& b1, const Board& b2) {
    Board diff = Board(b1.getRowCount(), b1.getColCount());
    assert(b1.getRowCount() == b2.getRowCount());
    assert(b1.getColCount() == b2.getColCount());
    for(int rowIdx = 0; rowIdx < b1.getRowCount(); rowIdx++) {
        for(int colIdx = 0; colIdx < b1.getColCount(); colIdx++) {
            bool val1 = b1.getCell(rowIdx, colIdx);
            bool val2 = b2.getCell(rowIdx, colIdx);
            if(val1 != val2) {
                diff.setCell(rowIdx, colIdx, true);
            }
        }
    }
    return diff;
}

- (void)testOpenCL_moreComplicated
{
    const Board randomBoard = makeRandomBoard(20, 20);
    Board b1 = randomBoard;
    Board b2 = randomBoard;
    
    b1.runSingleThreaded(1);
    b2.runOpenCL(1);
    if(b1 != b2) {
        Board diff = diffBoards(b1, b2);
        printf("b1 and b2 differ. Difference: %s\n", diff.toString().c_str());
    }
    XCTAssert(b1 == b2);
    
    b1 = randomBoard;
    b2 = randomBoard;
    b1.runSingleThreaded(4);
    b2.runOpenCL(4);
    if(b1 != b2) {
        Board diff = diffBoards(b1, b2);
        printf("b1 and b2 differ (4-gen). Difference: %s\n", diff.toString().c_str());
    }
    XCTAssert(b1 == b2);
}

- (void)testOpenCL_biggerBoard
{
    const Board randomBoard = makeRandomBoard(200, 200);
    Board b1 = randomBoard;
    Board b2 = randomBoard;
    
    b1.runSingleThreaded(1);
    b2.runOpenCL(1);
    if(b1 != b2) {
        Board diff = diffBoards(b1, b2);
        printf("b1 and b2 differ. Difference: %s\n", diff.toString().c_str());
    }
    XCTAssert(b1 == b2);
    
    b1 = randomBoard;
    b2 = randomBoard;
    b1.runSingleThreaded(51);
    b2.runOpenCL(51);
    if(b1 != b2) {
        Board diff = diffBoards(b1, b2);
        printf("b1 and b2 differ (51-gen). Difference: %s\n", diff.toString().c_str());
    }
    XCTAssert(b1 == b2);
}
#endif

@end
