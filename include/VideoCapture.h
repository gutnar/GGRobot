#ifndef GGROBOT_VIDEO_H
#define GGROBOT_VIDEO_H

#include <iostream>
#include <GLFW/glfw3.h>
#include <thread>
#include <highgui.h>
#include <cv.h>


namespace gg {
    class VideoCapture
    {
    public:
        /// @brief Default constructor
        VideoCapture();

        /// @brief Video constructor that opens the device
        /// @param device Video capture device number
        VideoCapture(int device);

        // Destructor, stops capturing
        ~VideoCapture();

        /// @brief Get the latest captured thread
        /// @return Image
        cv::Mat getLatestFrame();

        /// @brief Start capturing frames
        void open(int device);

        /// @brief Stop capturing frames
        void close();

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
        cv::Mat mLatestFrame;
    };
}


#endif //GGROBOT_VIDEO_H
