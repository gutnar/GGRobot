#ifndef GGROBOT_TRESHOLDER_H
#define GGROBOT_TRESHOLDER_H

#include <cv.h>
#include <array>
#include <vector>

namespace gg {
    class Tresholder
    {
    public:
        Tresholder(const cv::Mat &mat);

    private:
        cv::Mat mMat;
        int mNumberOfPixels;

        void kMeans();

    };
}


#endif //GGROBOT_TRESHOLDER_H
