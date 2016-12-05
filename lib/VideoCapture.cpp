//
// Created by Gutnar Leede on 05/12/2016.
//

#include <iostream>
#include <unistd.h>

#include "VideoCapture.h"


namespace gg {
    VideoCapture::VideoCapture(int device) {
        mDevice = device;
        startCapture();
    }

    cv::Mat VideoCapture::getCurrentFrame()
    {
        cv::Mat image;
        mVideoCapture >> image;
        return image;
    }

    void VideoCapture::startCapture() {
        // Open device
        mVideoCapture.open(mDevice);

        // Get initial frame
        mVideoCapture >> mLatestFrame;

        // Continously get frames
        mThread = std::thread(&VideoCapture::storeLatestFrame, this);
    }

    void VideoCapture::stopCapture() {
        mVideoCapture.release();
    }

    void VideoCapture::storeLatestFrame()
    {
        while (true) {
            // Kill thread when device is released
            if (!mVideoCapture.isOpened()) {
                break;
            }

            mVideoCapture >> mLatestFrame;
        }
    }

    VideoCapture::~VideoCapture()
    {
        mThread.detach();
        stopCapture();
    }

    cv::Mat VideoCapture::getLatestFrame()
    {
        return mLatestFrame;
    }
}