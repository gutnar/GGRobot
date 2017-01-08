#include <highgui.h>
#include "Tresholder.h"

namespace gg {
    Tresholder::Tresholder(const cv::Mat &mat)
    {
        mMat = mat;
        mNumberOfPixels = mMat.rows * mMat.cols;
        kMeans();

        // OpenCL compilation
        mCLTreshold.compile("TresholdKernels");
    }

    void Tresholder::kMeans()
    {
        /// OPENCL
        /*
        // Create kernel
        mCLImageProcessor.setKernel("mark_pixels");

        // Create OpenCL read-only buffer
        CL::Buffer bufferIn = mCLImageProcessor.createReadBuffer(sizeof(int)*numberOfPixels*3, 0);
        mCLImageProcessor.setBufferData(bufferIn, in);
        mCLImageProcessor.setArg(0, sizeof(cl_mem), &bufferIn);

        // Output
        int out[numberOfPixels];

        // Create OpenCL write-only buffer
        CL::Buffer bufferOut = mCLImageProcessor.createWriteBuffer(sizeof(int)*numberOfPixels, 1);
        mCLImageProcessor.setBufferData(bufferOut, &out);
        mCLImageProcessor.setArg(1, sizeof(cl_mem), &bufferOut);

        // Run OpenCL code (Send data from host to device)
        mCLImageProcessor.run(2, dimensions);
        std::cout<<"Sa eksid!" << std::endl;
        */

        // Initializing centroids
        int numberOfCentroids = 10;
        /*
        std::vector<std::array<int, 3>> centroids;

        for (int i = 0; i < numberOfCentroids; ++i)
        {
            std::array<int, 3> centroid;
            centroid[0] = rand() % 255;
            centroid[1] = rand() % 255;
            centroid[2] = rand() % 180;
            centroids.push_back(centroid);
        }*/

        // Image size
        const int width = mMat.cols;
        const int height = mMat.rows;
        const int numberOfPixels = width*height;
        const size_t dimensions[2] = {(size_t) width, (size_t) height};

        /// OPENCL
        int err;

        // Input
        int pixels[numberOfPixels*3];

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                cv::Vec3b pixel = mMat.at<cv::Vec3b>(y, x);

                pixels[(y*width + x)*3 + 0] = pixel[0];
                pixels[(y*width + x)*3 + 1] = pixel[1];
                pixels[(y*width + x)*3 + 2] = pixel[2];
            }
        }

        // Create kernel
        mCLTreshold.setKernel("find_closest_centroid");

        // Create OpenCL read-only buffer for pixels
        CL::Buffer bufferIn = mCLTreshold.createReadBuffer(sizeof(int)*numberOfPixels*3, 0);
        mCLTreshold.setBufferData(bufferIn, pixels);
        mCLTreshold.setArg(0, sizeof(cl_mem), &bufferIn);


        for (int n = 0; n < 2; ++n)
        {
            // Clear clusters
            std::vector<std::vector<cv::Vec3b>> clusters;

            for (int i = 0; i < numberOfCentroids; ++i) {
                std::vector<cv::Vec3b> cluster;
                clusters.push_back(cluster);
            }
            /// OPENCL PREPARATIONS
            // Create clusters

            for (int y = 0; y < mMat.rows; ++y)
            {
                for (int x = 0; x < mMat.cols; ++x)
                {
                    cv::Vec3b pixel = mMat.at<cv::Vec3b>(y, x);

                    // Find closest centroid
                    float minDistance = -1;
                    int closestCentroidIndex = -1;

                    for (int i = 0; i < numberOfCentroids; ++i)
                    {
                        float distance = (float) (pow(pixel[0] - centroids[i][0], 2) +
                                                  pow(pixel[1] - centroids[i][1], 2) +
                                                  pow(pixel[2] - centroids[i][2], 2));

                        if (distance < minDistance || minDistance == -1)
                        {
                            minDistance = distance;
                            closestCentroidIndex = i;
                        }
                    }

                    clusters[closestCentroidIndex].push_back(pixel);
                }
            }

            // Calculate new centroids
            for (int i = 0; i < numberOfCentroids; ++i)
            {
                float H = 0;
                float S = 0;
                float V = 0;

                for (int j = 0; j < clusters[i].size(); ++j)
                {
                    H += clusters[i][j][0];
                    S += clusters[i][j][1];
                    V += clusters[i][j][2];
                }

                centroids[i][0] = (int) (H / clusters[i].size());
                centroids[i][1] = (int) (S / clusters[i].size());
                centroids[i][2] = (int) (V / clusters[i].size());
            }
        }

        //cv::Mat test = cv::Mat::zeros(mMat.rows, mMat.cols, CV_8U);
        cv::Mat test(mMat.rows, mMat.cols, CV_8UC3, cv::Scalar(0, 0, 0));

        for (int i = 0; i < numberOfCentroids; ++i) {
            if (!isnan(centroids[i][0]))
            {
                std::cout << centroids[i][0] << " " << centroids[i][1] << " " << centroids[i][2] << std::endl;

                for (int x = i*50; x < (i+1)*50; ++x) {
                    for (int y = 0; y < 50; ++y) {
                        test.at<cv::Vec3b>(y, x) = cv::Vec3b(centroids[i][0], centroids[i][1], centroids[i][2]);
                    }
                }
            }
        }

        cv::imshow("test", test);
    }
}