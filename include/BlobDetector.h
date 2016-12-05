//
// Created by Gutnar Leede on 06/12/2016.
//

#ifndef GGROBOT_BLOBDETECTOR_H
#define GGROBOT_BLOBDETECTOR_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <OpenCL/opencl.h>

namespace gg {
    class BlobDetector
    {
    public:
        BlobDetector();
    private:
        // Kernel code
        #define STRINGIFY(x) #x
        const char * mKernelSource =
        #include "BlobDetectorKernels.cl"
    };
}


#endif //GGROBOT_BLOBDETECTOR_H
