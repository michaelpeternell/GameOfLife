//
//  Board_opencl.cpp
//  GameOfLife
//
//  Created by Michael Peternell on 06.12.2017.
//  Copyright © 2017 Michael Peternell. All rights reserved.
//

#include "Board.h"

#if USE_OPENCL

#include <OpenCL/opencl.h>
#include "gol_kernel.cl.h"

static cl_ndrange makeRange(int size)
{
    return {
        1, // The number of dimensions to use.
        {0, 0, 0}, // The offset in each dimension.
        {(size_t)size, 0, 0}, // The global range
        {0, 0, 0} // The local size of each workgroup => auto-detect
    };
}

void Board::runOpenCL(int numberOfGenerations)
{
    // OpenCL code based on example code from Apple:
    // https://developer.apple.com/library/content/documentation/Performance/Conceptual/OpenCL_MacProgGuide/ExampleHelloWorld/Example_HelloWorld.html#//apple_ref/doc/uid/TP40008312-CH112-SW2
    
    if(numberOfGenerations <= 0) {
        return;
    }
    
    if(m_rowCount < 5 || m_colCount < 5) {
        printf("Warning: Refusing to run OpenCL for a board that has less than 5 rows or less than 5 columns.\n"
               "This board has %d rows and %d columns. Silently running single-threaded instead.\n", m_rowCount, m_colCount);
        runSingleThreaded(numberOfGenerations);
        return;
    }
    
    int size = (int)m_cells.size();
    
    char deviceName[128];
    
    // First, try to obtain a dispatch queue that can send work to the
    // GPU in our system.
    dispatch_queue_t queue = gcl_create_dispatch_queue(CL_DEVICE_TYPE_GPU, NULL);
    
    // In the event that our system does NOT have an OpenCL-compatible GPU,
    // we can use the OpenCL CPU compute device instead.
    if (queue == NULL) {
        printf("OpenCL: Didn't find GPU device. Falling back to GPU\n");
        queue = gcl_create_dispatch_queue(CL_DEVICE_TYPE_CPU, NULL);
    }
    
    // This is not required, but let's print out the name of the device
    // we are using to do work.  We could use the same function,
    // clGetDeviceInfo, to obtain all manner of information about the device.
    cl_device_id gpu = gcl_get_device_id_with_dispatch_queue(queue);
    clGetDeviceInfo(gpu, CL_DEVICE_NAME, 128, deviceName, NULL);
    if(m_verbose) {
        printf("OpenCL: Created a dispatch queue using the '%s'\n", deviceName);
    }
    
    // Convert cell data to "alive/dead format":
    cl_uchar* test_in = (cl_uchar*)malloc(sizeof(cl_uchar) * size);
    for (int i = 0; i < size; i++) {
        test_in[i] = m_cells[i] & 1;
    }
    
    // Once the computation using CL is done, will have to read the results
    // back into our application's memory space.  Allocate some space for that.
    cl_uchar* test_out = (cl_uchar*)malloc(sizeof(cl_uchar) * size);
    
    void* gpuBuffer1  = gcl_malloc(sizeof(cl_uchar) * size, test_in,
                               CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR);
    void* gpuBuffer2 = gcl_malloc(sizeof(cl_uchar) * size, NULL, CL_MEM_READ_WRITE);
    
    int o_groupSize;
    if(m_colCount >= 50) {
        o_groupSize = 15;
        int borders = (m_colCount-2) % o_groupSize;
        for(int gSize = 16; gSize<25; gSize++) {
            int newBorders = (m_colCount-2) % gSize;
            if(newBorders <= borders) {
                o_groupSize = gSize;
            }
        }
    } else {
        o_groupSize = 3;
    }
    if(m_verbose) {
        printf("OpenCL: Chose kernel-groupSize=%d\n", o_groupSize);
    }
    
    dispatch_sync(queue, ^{
        int o_groupsPerCol = (m_colCount-2)/o_groupSize;
        int o_numberOfRows = m_rowCount-2;
        int o_workgroupSize = o_groupsPerCol * o_numberOfRows;
        
        // Top+bottom
        int o2_workgroupSize = m_colCount*2;
        
        // Left+right
        int o3_rightBorder = m_colCount - o_groupsPerCol*o_groupSize - 1;
        std::vector<int> missingColumns;
        missingColumns.push_back(0);
        for(int i=m_colCount-o3_rightBorder; i<m_colCount; i++) {
            missingColumns.push_back(i);
        }
        
        cl_ndrange o_workgroupSize_range = makeRange(o_workgroupSize);
        cl_ndrange o2_workgroupSize_range = makeRange(o2_workgroupSize);
        cl_ndrange rowCount_range = makeRange(m_rowCount);
        cl_ndrange rangeOfOne = makeRange(1);
        
        void (^generate)(void* gpuBuffer1, void* gpuBuffer2) = ^(void* gpuBuffer1, void* gpuBuffer2) {
            gol_next_group_kernel(&o_workgroupSize_range,(cl_uchar*)gpuBuffer1, (cl_uchar*)gpuBuffer2, m_rowCount, m_colCount, o_groupSize);
            gol_next_updown_kernel(&o2_workgroupSize_range,(cl_uchar*)gpuBuffer1, (cl_uchar*)gpuBuffer2, m_rowCount, m_colCount);
            for(int c: missingColumns) {
                gol_next_col_kernel(&rowCount_range,(cl_uchar*)gpuBuffer1, (cl_uchar*)gpuBuffer2, m_rowCount, m_colCount, c);
            }
            
            // We need a full memory barrier after each generation, because we implicitly swap buffers.
            // OpenCL is unhappy if we call a function without arguments, so we give it some arguments.
            gol_rw_barrier_kernel(&rangeOfOne, (cl_uchar*)gpuBuffer1, (cl_uchar*)gpuBuffer2);
        };
        
        for(int gIdx=0; gIdx<numberOfGenerations-1; gIdx+=2) {
            generate(gpuBuffer1, gpuBuffer2);
            generate(gpuBuffer2, gpuBuffer1);
        }
        
        void* resultGpuBuffer;
        
        if(numberOfGenerations & 1) {
            generate(gpuBuffer1, gpuBuffer2);
            resultGpuBuffer = gpuBuffer2;
        } else {
            resultGpuBuffer = gpuBuffer1;
        }
        
        // Getting data out of the device's memory space is also easy;
        // use gcl_memcpy.  In this case, gcl_memcpy takes the output
        // computed by the kernel and copies it over to the
        // application's memory space.
        
        gcl_memcpy(test_out, resultGpuBuffer, sizeof(cl_uchar) * size);
    });
    
    // Don't forget to free up the CL device's memory when you're done.
    gcl_free(gpuBuffer1);
    gcl_free(gpuBuffer2);
    
    for(int rowIdx=0; rowIdx<m_rowCount; rowIdx++) {
        for(int colIdx=0; colIdx<m_colCount; colIdx++) {
            int idx = getIndex(rowIdx, colIdx);
            cl_uchar val = test_out[idx];
            setCell(rowIdx, colIdx, (val != 0));
        }
    }
    
    // And the same goes for system memory, as usual.
    free(test_in);
    free(test_out);
    
    // Finally, release your queue just as you would any GCD queue.
    dispatch_release(queue);
}

#endif // USE_OPENCL
