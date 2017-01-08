#include <highgui.h>
#include "Tresholder.h"

namespace gg {
    Tresholder::Tresholder(const cv::Mat &mat)
    {
        mMat = mat;
        mNumberOfPixels = mMat.rows * mMat.cols;
        mWorkSize[0] = (size_t) mNumberOfPixels;

        // OpenCL compilation
        mCLTreshold.compile("TresholdKernels");

        kMeans();
    }

    void Tresholder::kMeans()
    {
        // Image size
        const int width = mMat.cols;
        const int height = mMat.rows;

        // Initializing centroids
        const int numberOfCentroids = 10;
        float centroids[numberOfCentroids * 3];

        for (int i = 0; i < numberOfCentroids * 3; ++i)
        {
            centroids[i] = rand() % 255;
        }

        // Input
        int pixels[mNumberOfPixels * 3];

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                cv::Vec3b pixel = mMat.at<cv::Vec3b>(y, x);

                pixels[(y * width + x) * 3 + 0] = pixel[0];
                pixels[(y * width + x) * 3 + 1] = pixel[1];
                pixels[(y * width + x) * 3 + 2] = pixel[2];
            }
        }

        // Output
        int clusterMap[mNumberOfPixels];

        // Iterations
        for (int n = 0; n < 10; ++n)
        {
            // Create kernel
            mCLTreshold.setKernel("findClosestCentroid");

            // Create OpenCL read-only buffer for pixels
            CL::Buffer bufferPixels = mCLTreshold.createReadBuffer(sizeof(int) * mNumberOfPixels * 3, 0);
            mCLTreshold.setBufferData(bufferPixels, pixels);
            mCLTreshold.setArg(0, sizeof(cl_mem), &bufferPixels);

            // Create OpenCL read-only buffer for centroids
            CL::Buffer bufferCentroids = mCLTreshold.createReadBuffer(sizeof(float) * numberOfCentroids * 3, 1);
            mCLTreshold.setBufferData(bufferCentroids, centroids);
            mCLTreshold.setArg(1, sizeof(cl_mem), &bufferCentroids);

            // Send number of centroids to GPU
            mCLTreshold.setArg(2, sizeof(int), &numberOfCentroids);

            // Create OpenCL write-only buffer
            CL::Buffer bufferCluster = mCLTreshold.createWriteBuffer(sizeof(int) * mNumberOfPixels, 3);
            mCLTreshold.setBufferData(bufferCluster, &clusterMap);
            mCLTreshold.setArg(3, sizeof(cl_mem), &bufferCluster);

            // Run OpenCL code
            mCLTreshold.run(1, mWorkSize);

            // Reset centroids
            float centroidMembers[numberOfCentroids];

            for (int i = 0; i < numberOfCentroids; ++i)
            {
                centroids[i * 3 + 0] = 0;
                centroids[i * 3 + 1] = 0;
                centroids[i * 3 + 2] = 0;
                centroidMembers[i] = 0;
            }

            // Calculate new centroids
            for (int x = 0; x < width; ++x)
            {
                for (int y = 0; y < height; ++y)
                {
                    int c = clusterMap[y * width + x];
                    cv::Vec3b pixel = mMat.at<cv::Vec3b>(y, x);
                    centroids[c * 3] += pixel[0];
                    centroids[c * 3 + 1] += pixel[1];
                    centroids[c * 3 + 2] += pixel[2];
                    centroidMembers[c]++;
                }
            }

            for (int i = 0; i < numberOfCentroids; ++i)
            {
                if (centroidMembers[i] >= 1)
                {
                    centroids[i * 3] /= centroidMembers[i];
                    centroids[i * 3 + 1] /= centroidMembers[i];
                    centroids[i * 3 + 2] /= centroidMembers[i];
                } else
                {
                    centroids[i * 3] = 0;
                    centroids[i * 3 + 1] = 0;
                    centroids[i * 3 + 2] = 0;
                }
            }
        }

        // Test
        cv::Mat test(mMat.rows, mMat.cols, CV_8UC3, cv::Scalar(0, 0, 0));

        for (int i = 0; i < numberOfCentroids; ++i)
        {
            for (int x = i * 50; x < (i + 1) * 50; ++x)
            {
                for (int y = 0; y < 50; ++y)
                {
                    test.at<cv::Vec3b>(y, x) = cv::Vec3b((uchar) centroids[i * 3], (uchar) centroids[i * 3 + 1],
                                                         (uchar) centroids[i * 3 + 2]);
                }
            }
        }

        cv::imshow("test", test);
    }
}