
// For OpenCL purposes, a Game Of Life board is represented as a contiguous
// array of booleans. (Each boolean represented by an unsigned char (8 bit))
// Each row in the board is a contiguous memory area of size colCount. The cell
// with row = Y and column = X has the index = Y*colCount + X.
// Each boolean value is true if the cell is alive and false otherwise.

// Each function on this file operates on "inputData" and "outputData".
// The functions only read from "inputData" and never write to it.
// The functions only write to "outputData" and never read to it.
// There can be no race condition during the calculation of a generation.

// The input board is always represented with the arguments "inputData",
// "rowCount" and "colCount". The output board (where results are saved to)
// is represented by "outputData", "rowCount" and "colCount".

// Each function in this file (except gol_rw_barrier) reads from the input board
// and writes to the output board. For kernel methods, the index to use is always
// derived from the global ID ( get_global_id(0) ). OpenCL-wise, a board is
// only 1-dimensional.

/// Calculates the alive state for the cell given by 'idx'.
/// This function is comparatively slow and is only used for edge cases.
void gol_do_slow(global uchar* inputData, global uchar* outputData, int rowCount, int colCount, size_t idx)
{
    int row = idx / colCount;
    int col = idx - (row * colCount);
    
    int rAbove, rBelow, cLeft, cRight;
    if (row == 0) {
        rAbove = (rowCount*colCount) - colCount;
    }
    else {
        rAbove = -colCount;
    }
    if (row == (rowCount - 1)) {
        rBelow = -(rowCount*colCount) + colCount;
    }
    else {
        rBelow = +colCount;
    }
    if (col == 0) {
        cLeft = colCount - 1;
    }
    else {
        cLeft = -1;
    }
    if (col == (colCount - 1)) {
        cRight = -colCount + 1;
    }
    else {
        cRight = +1;
    }
    
    uchar neighbours = inputData[idx+rAbove+cLeft] + inputData[idx+rAbove] + inputData[idx+rAbove+cRight]
    + inputData[idx+cLeft] + inputData[idx+cRight]
    + inputData[idx+rBelow+cLeft] + inputData[idx+rBelow] + inputData[idx+rBelow+cRight];
    uchar wasAlive = inputData[idx];
    if(wasAlive != 0) {
        outputData[idx] = (neighbours == 2 || neighbours == 3) ? 1 : 0;
    } else {
        outputData[idx] = (neighbours == 3) ? 1 : 0;
    }
}

/// Calculates the alive state of a group of cells.
/// Most cell alive states are calculated with this function. It cannot calculate
/// the alive states of cells at the borders of the board because it doesn't
/// implement coordinate wraparound (for performance reasons.)
/// @param groupSize Number of cells to compute with one function call.
///        The cells are next to each other on the same row.
kernel void gol_next_group(global uchar* inputData, global uchar* outputData, int rowCount, int colCount, int groupSize)
{
    size_t what = get_global_id(0);
    
    int groupsPerCol = (colCount-2)/groupSize;
    int row = what / groupsPerCol + 1;
    int col = (what - (row-1)*groupsPerCol) * groupSize + 1;
    
    // Variables a-i describe a 3*3 kernel of cell values.
    
    // a b c
    // d e f
    // g h i
    
    int aIdx = (row-1)*colCount + (col-1);
    int bIdx = aIdx+1;
    int cIdx = aIdx+2;
    int dIdx = row*colCount + (col-1);
    int eIdx = dIdx+1;
    int fIdx = dIdx+2;
    int gIdx = (row+1)*colCount + (col-1);
    int hIdx = gIdx+1;
    int iIdx = gIdx+2;
    
    uchar a = inputData[aIdx];
    uchar b = inputData[bIdx];
    uchar c = inputData[cIdx];
    uchar d = inputData[dIdx];
    uchar e = inputData[eIdx];
    uchar f = inputData[fIdx];
    uchar g = inputData[gIdx];
    uchar h = inputData[hIdx];
    uchar i = inputData[iIdx];
    
    int orange = 1;
    for(;;) {
        int neighbours = a+b+c+d+f+g+h+i;
        outputData[eIdx] = neighbours == 3 || (neighbours == 2 && e);
        
        if(orange == groupSize) {
            break;
        }
        
        orange++;
        
        // Move the 3*3 kernel one step to the right
        
        cIdx++;
        eIdx++;
        fIdx++;
        iIdx++;
        
        a = b;
        b = c;
        c = inputData[cIdx];
        d = e;
        e = f;
        f = inputData[fIdx];
        g = h;
        h = i;
        i = inputData[iIdx];
    }
}

/// Calculates alive state for the first and last row in a board
kernel void gol_next_updown(global uchar* inputData, global uchar* outputData, int rowCount, int colCount)
{
    size_t what = get_global_id(0);
    int row, col;
    if(what < colCount) {
        gol_do_slow(inputData, outputData, rowCount, colCount, what);
    } else {
        gol_do_slow(inputData, outputData, rowCount, colCount, what + colCount*(rowCount-2));
    }
}

/// Calculates alive state for a particular col
kernel void gol_next_col(global uchar* inputData, global uchar* outputData, int rowCount, int colCount, int col)
{
    size_t row = get_global_id(0);
    int idx = row*colCount + col;
    gol_do_slow(inputData, outputData, rowCount, colCount, idx);
}

/// Issue a full memory barrier. Necessary since these kernels are used with
/// some kind of double buffering.
/// OpenCL doesn't like kernel functions without arguments, so we will declare
/// some dummy unused arguments.
kernel void gol_rw_barrier(global uchar* inputData, global uchar* outputData)
{
    barrier(CLK_GLOBAL_MEM_FENCE);
}
