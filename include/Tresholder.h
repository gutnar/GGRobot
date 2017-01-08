#ifndef GGROBOT_TRESHOLDER_H
#define GGROBOT_TRESHOLDER_H

#include <cv.h>
#include <array>
#include <vector>
#include "CL.h"

namespace gg {
    class Tresholder
    {
    public:
        Tresholder(const cv::Mat &mat);

    private:
        cv::Mat mMat;
        int mNumberOfPixels;
        size_t mWorkSize[1];

        void kMeans();

        // OpenCL instance
        CL mCLTreshold;

    };
}


#endif //GGROBOT_TRESHOLDER_H
