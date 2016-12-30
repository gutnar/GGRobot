//
// Created by Gutnar Leede on 05/12/2016.
//

#include "VideoCapture.h"


namespace gg {
    /// Generates an OpenGL texture ID and starts capturing
    VideoCapture::VideoCapture(int device)
    {
        glGenTextures(1, &mTextureID);
        mDevice = device;
        //mVideoCapture.open(mDevice);
        startCapture();
    }

    /*
    cv::Mat VideoCapture::getCurrentFrame()
    {
        cv::Mat image;
        mVideoCapture >> image;
        return image;
    }
     */

    void VideoCapture::startCapture()
    {
        // Open device
        mVideoCapture.open(mDevice);

        // Get initial frame
        //mVideoCapture >> mLatestFrame;

        // Continously get frames
        //mThread = std::thread(&VideoCapture::storeLatestFrame, this);
    }

    void VideoCapture::stopCapture()
    {
        mVideoCapture.release();
    }

    void VideoCapture::storeLatestFrame()
    {
        while (mVideoCapture.isOpened())
        {
            // Capture frame
            cv::Mat mat;
            mVideoCapture >> mat;

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
                         mat.cols,          // Image width
                         mat.rows,          // Image height
                         0,                 // Border width in pixels (can either be 1 or 0)
                         GL_BGR, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                         GL_UNSIGNED_BYTE,  // Image data type
                         mat.ptr());        // The actual image data itself
        }
    }

    VideoCapture::~VideoCapture()
    {
        //mThread.detach();
        stopCapture();
        glDeleteTextures(1, &mTextureID);
    }

    /*
    cv::Mat VideoCapture::getLatestFrame()
    {
        return mLatestFrame;
    }
     */

    GLuint VideoCapture::getTextureID() {
        return mTextureID;
    }

    void VideoCapture::capture() {
        // Capture frame
        cv::Mat mat;
        mVideoCapture >> mat;

        // Bind to our texture handle
        glBindTexture(GL_TEXTURE_2D, mTextureID);

        // Set texture interpolation methods for minification and magnification
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Set texture clamping method
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        // Set incoming texture format
        GLenum inputColourFormat = GL_BGR;

        if (mat.channels() == 1)
        {
            inputColourFormat = GL_LUMINANCE;
        }

        // Create the texture
        glTexImage2D(GL_TEXTURE_2D,     // Type of texture
                     0,                 // Pyramid level (for mip-mapping) - 0 is the top level
                     GL_RGB,            // Internal colour format to convert to
                     mat.cols,          // Image width
                     mat.rows,          // Image height
                     0,                 // Border width in pixels (can either be 1 or 0)
                     inputColourFormat, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                     GL_UNSIGNED_BYTE,  // Image data type
                     mat.ptr());        // The actual image data itself
    }
}