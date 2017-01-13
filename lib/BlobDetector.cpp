#include "BlobDetector.h"

namespace gg {
    BlobDetector::BlobDetector(int width, int height)
    {
        // Generate OpenGL texture
        glGenTextures(1, &mTextureID);

        /// GET CENTROIDS FROM THE FILE
        // Open the file
        std::ifstream centroidsFile("config/colors.txt");

        centroidsFile >> mNumberOfCentroids;

        int x;
        int i = 0;
        mCentroids = new int[mNumberOfCentroids*9];

        while (centroidsFile >> x)
        {
            mCentroids[i] = x;
            ++i;
        }

        centroidsFile.close();

        // Image size
        mWidth = width;
        mHeight = height;
        mNumberOfPixels = mWidth * mHeight;

        /// OPENCL
        // Output array
        mColorMap = new int[mNumberOfPixels];

        // OpenCL compilation
        mCLImageProcessor.compile("BlobDetectorKernels");

        // Create kernel
        mCLImageProcessor.setKernel("mark_pixels");

        // Create OpenCL read-only buffer for image
        mBufferIn = mCLImageProcessor.createReadBuffer(sizeof(int)*mNumberOfPixels*3, 0);
        mCLImageProcessor.setArg(0, sizeof(cl_mem), &mBufferIn);

        // Create OpenCL read-only buffer for centroids
        mBufferCentroids = mCLImageProcessor.createReadBuffer(sizeof(int)*mNumberOfCentroids*9, 1);
        mCLImageProcessor.setBufferData(mBufferCentroids, &*mCentroids);
        mCLImageProcessor.setArg(1, sizeof(cl_mem), &mBufferCentroids);

        // Set OpenCL argument numberOfCentroids
        mCLImageProcessor.setArg(2, sizeof(cl_mem), &mNumberOfCentroids);

        // Create OpenCL write-only buffer
        mBufferOut = mCLImageProcessor.createWriteBuffer(sizeof(int)*mNumberOfPixels, 3);
        mCLImageProcessor.setArg(3, sizeof(cl_mem), &mBufferOut);
    }

    BlobDetector::~BlobDetector()
    {
        delete[] mColorMap;
        //clReleaseProgram(mCLProgram);
        //clReleaseContext(mCLContext);
    }

    void BlobDetector::processImage(const cv::Mat &mat)
    {
        // Processed image
        cv::Mat processedMat = mat.clone();

        // Write image data to input buffer
        mCLImageProcessor.setBufferData(mBufferIn, processedMat.ptr());

        // Run OpenCL code (Send data from host to device)
        const size_t workSize[1] = {(size_t) mNumberOfPixels};
        mCLImageProcessor.setBufferData(mBufferOut, &*mColorMap);
        mCLImageProcessor.run(1, workSize);

        /// MARK PIXELS
        for (int y = 0; y < mHeight; ++y)
        {
            for (int x = 0; x < mWidth; ++x)
            {
                int c = mColorMap[y * mWidth + x] - 1;

                if (c == -1)
                {
                    processedMat.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 0);
                } else {
                    //std::cout << c << " " << mCentroids[c*9 + 6] << " " << mCentroids[c*9 + 7] << " " << mCentroids[c*9 + 8] << "\n";

                    processedMat.at<cv::Vec3b>(y, x) = cv::Vec3b(
                            (uchar) mCentroids[c*9 + 6],
                            (uchar) mCentroids[c*9 + 7],
                            (uchar) mCentroids[c*9 + 8]
                    );
                }

                //processedMat.at<cv::Vec3b>(y, x) = cv::Vec3b(out[y*width+x], out[y*width+x], out[y*width+x]);
                //continue;

                /*
                switch (mColorMap[y * mWidth + x])
                {
                    // white
                    case 0:
                        processedMat.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 255, 255);
                        break;
                        // yellow
                    case 1:
                        processedMat.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 255, 255);
                        break;
                        // blue
                    case 2:
                        processedMat.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 0, 0);
                        break;
                        // black
                    case 3:
                        processedMat.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 0);
                        break;
                        // green
                    case 4:
                        processedMat.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 255, 0);
                        break;
                }
                 */
            }
        }

        // Clean up
        /*
        clReleaseMemObject(bufferIn);
        clReleaseMemObject(bufferOut);
        clReleaseKernel(kernel);
        clReleaseCommandQueue(commands);
         */

        // Find blobs
        findBlobs();

        /// TEXTURING
        // Bind to our texture handle
        glBindTexture(GL_TEXTURE_2D, mTextureID);

        // Set texture interpolation methods for minification and magnification
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Set texture clamping method
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        // Create the texture
        glTexImage2D(GL_TEXTURE_2D,     // Type of texture
                     0,                 // Pyramid level (for mip-mapping) - 0 is the top level
                     GL_RGB,            // Internal colour format to convert to
                     mWidth,          // Image width
                     mHeight,          // Image height
                     0,                 // Border width in pixels (can either be 1 or 0)
                     GL_BGR, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                     GL_UNSIGNED_BYTE,  // Image data type
                     processedMat.ptr());        // The actual image data itself

        //cv::imshow("Blob detector", processedMat);
    }

    GLuint BlobDetector::getTextureID()
    {
        return mTextureID;
    }

    void BlobDetector::findBlobs()
    {
        // Clear previously detected blobs
        mBlobs.clear();

        // Detect blobs of same color
        std::vector<BlobLine> previousLines;

        for (int y = 0; y < mHeight; ++y)
        {
            std::vector<BlobLine> currentLines;

            int lastColor = mColorMap[y * mWidth];
            int beginColorIndex = 0;

            for (int x = 1; x < mWidth + 1; ++x)
            {
                int i = y * mWidth + x;
                int currentColor = (x == mWidth) ? -1 : mColorMap[i];

                if (lastColor != currentColor)
                {
                    if ((lastColor == 1 || lastColor == 2 || lastColor == 5) && x - 1 - beginColorIndex > 1)
                    {//} || out[i] == 2) {
                        BlobLine line;
                        line.y = y;
                        line.xi = beginColorIndex;
                        line.xf = x - 1;
                        line.color = lastColor;
                        line.blobIndex = -1;

                        for (int l = 0; l < previousLines.size(); ++l)
                        {
                            // Skip if colors do not match
                            if (previousLines[l].color != line.color)
                            {
                                continue;
                            }

                            // Skip if touching is too small
                            int common =
                                    std::min(previousLines[l].xf, line.xf) - std::max(previousLines[l].xi, line.xi);

                            if (common < 2)
                            {
                                continue;
                            }

                            // Connect lines into blob
                            if (line.blobIndex == -1)
                            {
                                line.blobIndex = previousLines[l].blobIndex;
                                //cout << line.blobIndex << endl;
                            } else if (line.blobIndex != previousLines[l].blobIndex)
                            {
                                mBlobs[line.blobIndex].addBlob(mBlobs[previousLines[l].blobIndex]);
                                mBlobs[previousLines[l].blobIndex].mHidden = true;
                                previousLines[l].blobIndex = line.blobIndex;
                            }
                        }

                        // Create new blob?
                        if (line.blobIndex == -1)
                        {
                            Blob blob(lastColor);
                            line.blobIndex = (int) mBlobs.size();
                            mBlobs.push_back(blob);
                        }

                        // Add line to blob
                        mBlobs[line.blobIndex].addLine(line);

                        // Add to current row
                        currentLines.push_back(line);
                    }

                    beginColorIndex = x;
                    lastColor = currentColor;
                }
            }

            previousLines = currentLines;
        }

        /*
        for (int i = 0; i < mBlobs.size(); ++i) {
            cv::line(mat, cv::Point(mBlobs[i].mMinX, mBlobs[i].mMinY), cv::Point(mBlobs[i].mMaxX, mBlobs[i].mMinY),
                     cv::Scalar(0, 0, 0));
            cv::line(mat, cv::Point(mBlobs[i].mMinX, mBlobs[i].mMaxY), cv::Point(mBlobs[i].mMaxX, mBlobs[i].mMaxY),
                     cv::Scalar(0, 0, 0));
            cv::line(mat, cv::Point(mBlobs[i].mMinX, mBlobs[i].mMinY), cv::Point(mBlobs[i].mMinX, mBlobs[i].mMaxY),
                     cv::Scalar(0, 0, 0));
            cv::line(mat, cv::Point(mBlobs[i].mMaxX, mBlobs[i].mMinY), cv::Point(mBlobs[i].mMaxX, mBlobs[i].mMaxY),
                     cv::Scalar(0, 0, 0));
        }
         */
    }

    void BlobDetector::render()
    {
        /*
        // Bind to our texture handle
        glBindTexture(GL_TEXTURE_2D, mTextureID);

        // Set texture interpolation methods for minification and magnification
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Set texture clamping method
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        // Create the texture
        glTexImage2D(GL_TEXTURE_2D,     // Type of texture
                     0,                 // Pyramid level (for mip-mapping) - 0 is the top level
                     GL_RED,            // Internal colour format to convert to
                     mWidth,          // Image width
                     mHeight,          // Image height
                     0,                 // Border width in pixels (can either be 1 or 0)
                     GL_RED, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                     GL_UNSIGNED_BYTE,  // Image data type
                     mColorMap);        // The actual image data itself
                     */


    }
}