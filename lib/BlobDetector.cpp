#include "BlobDetector.h"

namespace gg {
    BlobDetector::BlobDetector()
    {
        // Generate OpenGL texture
        glGenTextures(1, &mTextureID);

        // OpenCL compilation
        mCLImageProcessor.compile("BlobDetectorKernels");
    }

    BlobDetector::~BlobDetector()
    {
        //clReleaseProgram(mCLProgram);
        //clReleaseContext(mCLContext);
    }

    void BlobDetector::processImage(const cv::Mat &mat)
    {
        // Processed image
        cv::Mat processedMat = mat.clone();

        // Resize image
        //cv::resize(mat, processedMat, cv::Size(640, 360), 0, 0, CV_INTER_LINEAR);

        // Image size
        const int width = processedMat.cols;
        const int height = processedMat.rows;
        const int numberOfPixels = width*height;
        const size_t dimensions[2] = {(size_t) width, (size_t) height};

        /// OPENCL
        int err;

        // Input
        int in[numberOfPixels*3];

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                cv::Vec3b pixel = processedMat.at<cv::Vec3b>(y, x);

                in[(y*width + x)*3 + 0] = pixel[0];
                in[(y*width + x)*3 + 1] = pixel[1];
                in[(y*width + x)*3 + 2] = pixel[2];
            }
        }

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

        /// MARK PIXELS
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                //processedMat.at<cv::Vec3b>(y, x) = cv::Vec3b(out[y*width+x], out[y*width+x], out[y*width+x]);
                //continue;
                switch (out[y*width + x]) {
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
        findBlobs(processedMat, out);

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
                     width,          // Image width
                     height,          // Image height
                     0,                 // Border width in pixels (can either be 1 or 0)
                     GL_BGR, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                     GL_UNSIGNED_BYTE,  // Image data type
                     processedMat.ptr());        // The actual image data itself
    }

    GLuint BlobDetector::getTextureID() {
        return mTextureID;
    }

    void BlobDetector::findBlobs(cv::Mat &mat, int* out) {
        int height = mat.rows;
        int width = mat.cols;

        // Clear previously detected blobs
        mBlobs.clear();

        // Detect blobs of same color
        std::vector<BlobLine> previousLines;

        for (int y = 0; y < height; ++y) {
            std::vector<BlobLine> currentLines;

            int lastColor = out[y*width];
            int beginColorIndex = 0;

            for (int x = 1; x < width + 1; ++x) {
                int i = y*width + x;
                int currentColor = (x == width) ? -1 : out[i];

                if (lastColor != currentColor) {
                    if ((lastColor == 1 || lastColor == 2 || lastColor == 5) && x - 1 - beginColorIndex > 1) {//} || out[i] == 2) {
                        BlobLine line;
                        line.y = y;
                        line.xi = beginColorIndex;
                        line.xf = x - 1;
                        line.color = lastColor;
                        line.blobIndex = -1;

                        for (int l = 0; l < previousLines.size(); ++l) {
                            // Skip if colors do not match
                            if (previousLines[l].color != line.color) {
                                continue;
                            }

                            // Skip if touching is too small
                            int common = std::min(previousLines[l].xf, line.xf) - std::max(previousLines[l].xi, line.xi);

                            if (common < 2) {
                                continue;
                            }

                            // Connect lines into blob
                            if (line.blobIndex == -1) {
                                line.blobIndex = previousLines[l].blobIndex;
                                //cout << line.blobIndex << endl;
                            } else if (line.blobIndex != previousLines[l].blobIndex) {
                                mBlobs[line.blobIndex].addBlob(mBlobs[previousLines[l].blobIndex]);
                                mBlobs[previousLines[l].blobIndex].mHidden = true;
                                previousLines[l].blobIndex = line.blobIndex;
                            }
                        }

                        // Create new blob?
                        if (line.blobIndex == -1) {
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
    }
}