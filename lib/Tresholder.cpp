#include <highgui.h>
#include "Tresholder.h"

namespace gg {
    cv::Mat Tresholder::mMat;
    cv::Mat Tresholder::mTest;
    int Tresholder::mNumberOfCentroids = 20;
    int Tresholder::mSampleSize;
    int *Tresholder::mClusterMap;
    float *Tresholder::mCentroids;
    std::vector<int> Tresholder::mSelectedCentroids;

    Tresholder::Tresholder()
    {
        // OpenCL compilation
        mCLTreshold.compile("TresholdKernels");
    }

    void Tresholder::processImage(const cv::Mat &mat)
    {
        // Image data
        mMat = mat;
        mNumberOfPixels = mMat.rows * mMat.cols;

        // Clear selected centroids
        mSelectedCentroids.clear();

        // Run K-means algorithm
        mWorkSize[0] = (size_t) mNumberOfPixels;
        kMeans();
    }

    void Tresholder::kMeans()
    {
        // Image size
        const int width = mMat.cols;
        const int height = mMat.rows;

        // Initializing centroids
        const int numberOfCentroids = mNumberOfCentroids;
        mCentroids = new float[numberOfCentroids * 3];

        for (int i = 0; i < numberOfCentroids * 3; ++i)
        {
            mCentroids[i] = rand() % 255;
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
        mClusterMap = new int[mNumberOfPixels];

        // Iterations
        for (int n = 0; n < 20; ++n)
        {
            // Create kernel
            mCLTreshold.setKernel("findClosestCentroid");

            // Create OpenCL read-only buffer for pixels
            CL::Buffer bufferPixels = mCLTreshold.createReadBuffer(sizeof(int) * mNumberOfPixels * 3, 0);
            mCLTreshold.setBufferData(bufferPixels, pixels);
            mCLTreshold.setArg(0, sizeof(cl_mem), &bufferPixels);

            // Create OpenCL read-only buffer for centroids
            CL::Buffer bufferCentroids = mCLTreshold.createReadBuffer(sizeof(float) * numberOfCentroids * 3, 1);
            mCLTreshold.setBufferData(bufferCentroids, mCentroids);
            mCLTreshold.setArg(1, sizeof(cl_mem), &bufferCentroids);

            // Send number of centroids to GPU
            mCLTreshold.setArg(2, sizeof(int), &numberOfCentroids);

            // Create OpenCL write-only buffer
            CL::Buffer bufferCluster = mCLTreshold.createWriteBuffer(sizeof(int) * mNumberOfPixels, 3);
            mCLTreshold.setBufferData(bufferCluster, &*mClusterMap);
            mCLTreshold.setArg(3, sizeof(cl_mem), &bufferCluster);

            // Run OpenCL code
            mCLTreshold.run(1, mWorkSize);

            // Reset centroids
            float centroidMembers[numberOfCentroids];

            for (int i = 0; i < numberOfCentroids; ++i)
            {
                mCentroids[i * 3 + 0] = 0;
                mCentroids[i * 3 + 1] = 0;
                mCentroids[i * 3 + 2] = 0;
                centroidMembers[i] = 0;
            }

            // Calculate new centroids
            for (int x = 0; x < width; ++x)
            {
                for (int y = 0; y < height; ++y)
                {
                    int c = mClusterMap[y * width + x];
                    cv::Vec3b pixel = mMat.at<cv::Vec3b>(y, x);
                    mCentroids[c * 3] += pixel[0];
                    mCentroids[c * 3 + 1] += pixel[1];
                    mCentroids[c * 3 + 2] += pixel[2];
                    centroidMembers[c]++;
                }
            }

            for (int i = 0; i < numberOfCentroids; ++i)
            {
                if (centroidMembers[i] >= 1)
                {
                    mCentroids[i * 3 + 0] /= centroidMembers[i];
                    mCentroids[i * 3 + 1] /= centroidMembers[i];
                    mCentroids[i * 3 + 2] /= centroidMembers[i];
                } else
                {
                    mCentroids[i * 3 + 0] = 0;
                    mCentroids[i * 3 + 1] = 0;
                    mCentroids[i * 3 + 2] = 0;
                }
            }
        }

        // Sample size
        mSampleSize = mMat.cols / numberOfCentroids;

        // Test
        cv::Mat test(mMat.rows + mSampleSize, mMat.cols, CV_8UC3, cv::Scalar(0, 0, 0));

        for (int x = 0; x < width; ++x)
        {
            for (int y = 0; y < height; ++y)
            {
                int c = mClusterMap[y * width + x];
                test.at<cv::Vec3b>(y, x) = cv::Vec3b((uchar) mCentroids[c * 3], (uchar) mCentroids[c * 3 + 1],
                                                     (uchar) mCentroids[c * 3 + 2]);
            }
        }

        for (int i = 0; i < numberOfCentroids; ++i)
        {
            for (int x = i * mSampleSize; x < (i + 1) * mSampleSize; ++x)
            {
                for (int y = mMat.rows + 1; y < test.rows; ++y)
                {
                    test.at<cv::Vec3b>(y, x) = cv::Vec3b((uchar) mCentroids[i * 3], (uchar) mCentroids[i * 3 + 1],
                                                         (uchar) mCentroids[i * 3 + 2]);
                }
            }
        }

        mTest = test;

        // User interface
        cv::imshow("Treshold", test);
        cv::setMouseCallback("Treshold", mouseEventHandler, this);
    }

    void Tresholder::onMouse(int event, int x, int y)
    {
        if (event == cv::EVENT_LBUTTONDOWN && y > mMat.rows)
        {
            // Toggle selection
            int centroidIndex = x / mSampleSize;
            bool selected = false;

            for (int i = 0; i < mSelectedCentroids.size(); ++i)
            {
                if (mSelectedCentroids[i] == centroidIndex)
                {
                    selected = true;
                    mSelectedCentroids.erase(mSelectedCentroids.begin() + i);
                    break;
                }
            }

            if (!selected)
            {
                mSelectedCentroids.push_back(centroidIndex);
            }

            // Update test image
            for (int x = 0; x < mMat.cols; ++x)
            {
                for (int y = 0; y < mMat.rows; ++y)
                {
                    int c = mClusterMap[y * mMat.cols + x];
                    bool selected = mSelectedCentroids.empty();

                    for (int i = 0; i < mSelectedCentroids.size(); ++i)
                    {
                        if (mSelectedCentroids[i] == c)
                        {
                            selected = true;
                            break;
                        }
                    }

                    if (selected)
                    {
                        mTest.at<cv::Vec3b>(y, x) = cv::Vec3b((uchar) mCentroids[c * 3], (uchar) mCentroids[c * 3 + 1],
                                                              (uchar) mCentroids[c * 3 + 2]);
                    } else
                    {
                        mTest.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 0);
                    }
                }
            }

            // Draw samples
            for (int i = 0; i < mNumberOfCentroids; ++i)
            {
                for (int x = i * mSampleSize; x < (i + 1) * mSampleSize; ++x)
                {
                    for (int y = mMat.rows + 1; y < mTest.rows; ++y)
                    {
                        mTest.at<cv::Vec3b>(y, x) = cv::Vec3b((uchar) mCentroids[i * 3], (uchar) mCentroids[i * 3 + 1],
                                                              (uchar) mCentroids[i * 3 + 2]);
                    }
                }
            }

            // Mark selected centroids
            for (int i = 0; i < mSelectedCentroids.size(); ++i)
            {
                cv::putText(mTest, std::to_string(i + 1),
                            cv::Point(mSelectedCentroids[i] * mSampleSize + mSampleSize / 2,
                                      mMat.rows + mSampleSize / 2), 1, 1,
                            cv::Scalar(255, 255, 255));
            }

            // Render
            cv::imshow("Treshold", mTest);
        }
    }

    void Tresholder::mouseEventHandler(int event, int x, int y, int flags, void *userdata)
    {
        Tresholder *tresholder = reinterpret_cast<Tresholder *>(userdata);
        tresholder->onMouse(event, x, y);
    }

    void Tresholder::save()
    {
        // Close window
        cv::destroyWindow("Treshold");

        // Get the cluster color ranges
        int min[mNumberOfCentroids][3];
        int max[mNumberOfCentroids][3];

        for (int i = 0; i < mNumberOfCentroids; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                min[i][j] = 255;
                max[i][j] = 0;
            }
        }

        for (int x = 0; x < mMat.cols; ++x)
        {
            for (int y = 0; y < mMat.rows; ++y)
            {
                int c = mClusterMap[y * mMat.cols + x];
                cv::Vec3b pixel = mMat.at<cv::Vec3b>(y, x);

                for (int i = 0; i < 3; ++i)
                {
                    if (pixel[i] < min[c][i])
                    {
                        min[c][i] = pixel[i];
                    }

                    if (pixel[i] > max[c][i])
                    {
                        max[c][i] = pixel[i];
                    }
                }
            }
        }

        // Write to file
        std::ofstream file;
        file.open("config/colors.txt");

        // Write number of colors
        file << mSelectedCentroids.size() << "\n";

        for (int i = 0; i < mSelectedCentroids.size(); ++i)
        {
            int c = mSelectedCentroids[i];

            std::cout << c << std::endl;

            file << min[c][0] << " " << min[c][1] << " " << min[c][2]
                 << " " << max[c][0] << " "
                 << max[c][1] << " "
                 << max[c][2] << " " << (int) mCentroids[c * 3] << " " << (int) mCentroids[c * 3 + 1] << " "
                 << (int) mCentroids[c * 3 + 2] << "\n";
        }

        file.close();
    }
}