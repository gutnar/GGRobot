//
// Created by Gutnar Leede on 05/12/2016.
//

#ifndef GGROBOT_VIDEO_H
#define GGROBOT_VIDEO_H

#include <iostream>
#include <GLFW/glfw3.h>
#include <thread>
#include <highgui.h>

namespace gg {
    class VideoCapture
    {
    public:
        /// @brief Video constructor
        /// @param device Video capture device number
        VideoCapture(int device);

        // Destructor, stops capturing
        ~VideoCapture();

        /// @brief Get an image from the device
        /// @return An image as the captured frame
        //cv::Mat getCurrentFrame();

        /// @brief Get the latest captured thread
        /// @return Image
        //cv::Mat getLatestFrame();

        /// @brief Start capturing frames
        void startCapture();

        /// @brief Stop capturing frames
        void stopCapture();

        /// @brief Get OpenGL texture ID
        GLuint getTextureID();

        /// @brief Capture a frame into texture
        void capture();
    private:
        // Device number
        int mDevice;

        // OpenGL texture ID
        GLuint mTextureID;

        // OpenCV video capture instance
        cv::VideoCapture mVideoCapture;

        // Latest captured frame
        //cv::Mat mLatestFrame;

        // Thread that updates latest frame continously
        std::thread mThread;

        // Repeatedly fetch and store frames
        void storeLatestFrame();
    };
}


#endif //GGROBOT_VIDEO_H
