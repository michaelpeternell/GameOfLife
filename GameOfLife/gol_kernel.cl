
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

kernel void gol_next_group(global uchar* inputData, global uchar* outputData, int rowCount, int colCount, int groupSize)
{
    size_t what = get_global_id(0);
    
    int groupsPerCol = (colCount-2)/groupSize;
    int row = what / groupsPerCol + 1;
    int col = (what - (row-1)*groupsPerCol) * groupSize + 1;
    
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

kernel void gol_next_col(global uchar* inputData, global uchar* outputData, int rowCount, int colCount, int col)
{
    size_t row = get_global_id(0);
    int idx = row*colCount + col;
    gol_do_slow(inputData, outputData, rowCount, colCount, idx);
}

kernel void gol_rw_barrier(global uchar* inputData, global uchar* outputData)
{
    barrier(CLK_GLOBAL_MEM_FENCE);
}
