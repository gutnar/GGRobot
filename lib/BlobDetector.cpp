#include "BlobDetector.h"

namespace gg {
    /// Test
    /// This is a test
    BlobDetector::BlobDetector(int width, int height)
    {
        // Generate OpenGL texture
        glGenTextures(1, &mTextureID);

        /// GET CENTROIDS FROM THE FILE
        reloadConfiguration();

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
        mCLImageProcessor.setKernel("closest_centroids");

        // Create OpenCL read-only buffer for image
        mBufferIn = mCLImageProcessor.createReadBuffer(sizeof(int) * mNumberOfPixels * 3, 0);
        mCLImageProcessor.setArg(0, sizeof(cl_mem), &mBufferIn);

        // Create OpenCL read-only buffer for centroids
        mBufferCentroids = mCLImageProcessor.createReadBuffer(sizeof(int) * mNumberOfCentroids * 9, 1);
        mCLImageProcessor.setBufferData(mBufferCentroids, &*mCentroids);
        mCLImageProcessor.setArg(1, sizeof(cl_mem), &mBufferCentroids);

        // Set OpenCL argument numberOfCentroids
        mCLImageProcessor.setArg(2, sizeof(cl_mem), &mNumberOfCentroids);

        // Create OpenCL write-only buffer
        mBufferOut = mCLImageProcessor.createWriteBuffer(sizeof(int) * mNumberOfPixels, 3);
        mCLImageProcessor.setBufferData(mBufferOut, &*mColorMap);
        mCLImageProcessor.setArg(3, sizeof(cl_mem), &mBufferOut);
    }

    void BlobDetector::reloadConfiguration()
    {
        // Open the file
        std::ifstream centroidsFile("config/colors.txt");

        centroidsFile >> mNumberOfCentroids;

        int x;
        int i = 0;
        mCentroids = new int[mNumberOfCentroids * 9];

        while (centroidsFile >> x)
        {
            mCentroids[i] = x;
            ++i;
        }

        centroidsFile.close();

        if (mBufferCentroids)
        {
            mCLImageProcessor.setBufferData(mBufferCentroids, &*mCentroids);
        }
    }

    BlobDetector::~BlobDetector()
    {
        mCLImageProcessor.releaseKernel();
        delete[] mColorMap;
        //clReleaseProgram(mCLProgram);
        //clReleaseContext(mCLContext);
    }

    cv::Mat BlobDetector::processImage(const cv::Mat &mat)
    {
        // Processed image
        cv::Mat processedMat = mat.clone();

        // Write image data to input buffer
        mCLImageProcessor.setBufferData(mBufferIn, processedMat.ptr());

        // Run OpenCL code (Send data from host to device)
        const size_t workSize[1] = {(size_t) mNumberOfPixels};
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
                    processedMat.at<cv::Vec3b>(y, x) = cv::Vec3b(
                            (uchar) mCentroids[c*9 + 6],
                            (uchar) mCentroids[c*9 + 7],
                            (uchar) mCentroids[c*9 + 8]
                    );
                }
            }
        }

        // Find blobs
        findBlobs();

        // Clean up
        /*
        clReleaseMemObject(bufferIn);
        clReleaseMemObject(mBufferOut);
        clReleaseKernel(kernel);
        clReleaseCommandQueue(commands);
         */

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

        return processedMat;
    }

    GLuint BlobDetector::getTextureID()
    {
        return mTextureID;
    }

    void BlobDetector::findBlobs()
    {
        // Detect blobs of same color
        std::vector<Blob> blobs;
        std::vector<BlobLine> previousLines;
        const int allowedOtherColors = 0;

        for (int y = 0; y < mHeight; ++y) {
            std::vector<BlobLine> currentLines;

            int lastColor = mColorMap[y*mWidth];
            int beginColorIndex = 0;
            int numberOfOtherColor = 0;

            // Go over right edge to mark line ends
            for (int x = 1; x < mWidth + 1 + allowedOtherColors; ++x) {
                int i = y*mWidth + x;
                int currentColor = (x >= mWidth) ? -1 : mColorMap[i];

                if (lastColor != currentColor) {
                    ++numberOfOtherColor;

                    if (numberOfOtherColor < allowedOtherColors) {
                        continue;
                    }

                    if (lastColor != 0 && x - 1 - allowedOtherColors - beginColorIndex > 1) {
                        BlobLine line;
                        line.y = y;
                        line.xi = beginColorIndex;
                        line.xf = x - 1 - allowedOtherColors;
                        line.color = lastColor;
                        line.blobIndex = -1;

                        for (int l = 0; l < previousLines.size(); ++l) {
                            // Skip if colors do not match
                            if (previousLines[l].color != line.color) {
                                continue;
                            }

                            // Skip if touching is too small
                            int common = std::min(previousLines[l].xf, line.xf) - std::max(previousLines[l].xi, line.xi);

                            if (common < 1) {
                                continue;
                            }

                            // Connect lines into blob
                            if (line.blobIndex == -1) {
                                line.blobIndex = previousLines[l].blobIndex;
                                //cout << line.blobIndex << endl;
                            } else if (line.blobIndex != previousLines[l].blobIndex) {
                                if (blobs[line.blobIndex].mSurface > blobs[previousLines[l].blobIndex].mSurface) {
                                    blobs[line.blobIndex].addBlob(blobs[previousLines[l].blobIndex]);
                                    blobs[previousLines[l].blobIndex].mHidden = true;
                                    previousLines[l].blobIndex = line.blobIndex;
                                } else {
                                    blobs[previousLines[l].blobIndex].addBlob(blobs[line.blobIndex]);
                                    blobs[line.blobIndex].mHidden = true;
                                    line.blobIndex = previousLines[l].blobIndex;
                                };
                            }
                        }

                        // Create new blob?
                        if (line.blobIndex == -1) {
                            Blob blob(lastColor);
                            line.blobIndex = (int) blobs.size();
                            blobs.push_back(blob);
                        }

                        // Add line to blob
                        blobs[line.blobIndex].addLine(line);

                        // Add to current row
                        currentLines.push_back(line);
                    }

                    //x -= allowedOtherColors;
                    beginColorIndex = x;
                    lastColor = mColorMap[y*mWidth + x]; //currentColor;

                    if (x >= mWidth) {
                        break;
                    }
                }
            }

            previousLines = currentLines;
        }

        // Discard non-blobs
        mBlobs.clear();

        for (int i = 0; i < blobs.size(); ++i) {
            if (blobs[i].getHeight() > 0) {
                mBlobs.push_back(blobs[i]);
            }
        }
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

    std::vector<Blob> BlobDetector::getBlobs() {
        return mBlobs;
    }
}
