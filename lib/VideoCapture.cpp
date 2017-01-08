#include "VideoCapture.h"


namespace gg {
    /// Default constructor
    VideoCapture::VideoCapture()
    {
    }

    /// Generates an OpenGL texture ID and starts capturing
    VideoCapture::VideoCapture(int device)
    {
        open(device);
    }

    void VideoCapture::open(int device)
    {
        if (!mTextureID) {
            glGenTextures(1, &mTextureID);
        }

        mDevice = device;
        mVideoCapture.open(mDevice);

    }

    void VideoCapture::close()
    {
        mVideoCapture.release();
    }

    VideoCapture::~VideoCapture()
    {
        close();
        glDeleteTextures(1, &mTextureID);
    }

    cv::Mat VideoCapture::getLatestFrame()
    {
        return mLatestFrame;
    }

    GLuint VideoCapture::getTextureID() {
        return mTextureID;
    }

    void VideoCapture::capture() {
        // Capture frame
        mVideoCapture >> mLatestFrame;

        cv::resize(mLatestFrame, mLatestFrame, cv::Size(640, 360), 0, 0, CV_INTER_LINEAR);

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
                     mLatestFrame.cols,          // Image width
                     mLatestFrame.rows,          // Image height
                     0,                 // Border width in pixels (can either be 1 or 0)
                     GL_BGR, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                     GL_UNSIGNED_BYTE,  // Image data type
                     mLatestFrame.ptr());        // The actual image data itself
    }
}