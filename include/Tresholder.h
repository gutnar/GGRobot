#ifndef GGROBOT_TRESHOLDER_H
#define GGROBOT_TRESHOLDER_H

#include <cv.h>
#include <array>
#include <vector>
#include "CL.h"
#include <iostream>
#include <fstream>

namespace gg {
    class Tresholder
    {
    public:
        /// @brief Default constructor
        Tresholder();

        /// @brief Process image
        /// @param mat OpenCV material
        void processImage(const cv::Mat &mat);

        /// @brief Save selected centroids
        void save();

        // Mouse event handlers
        void onMouse(int event, int x, int y);
        static void mouseEventHandler(int event, int x, int y, int flags, void* userdata);

    private:
        // Image data
        static cv::Mat mMat;
        int mNumberOfPixels;

        // K-means algorithm
        void kMeans();

        // OpenCL instance
        size_t mWorkSize[1];
        CL mCLTreshold;

        // Number of centroids (different colors)
        static int mNumberOfCentroids;

        // Cluster map (OpenCL output)
        static int* mClusterMap;

        // Centroids (detected color ranges)
        static float* mCentroids;

        // Selected centroids (centroids to save)
        static std::vector<int> mSelectedCentroids;

        // User interface
        static cv::Mat mTest;
        static int mSampleSize;
    };
}


#endif //GGROBOT_TRESHOLDER_H
