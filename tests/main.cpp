#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Tresholder.h>

#include "Timer.h"
#include "VideoCapture.h"
#include "ControlPanel.h"
#include "BlobDetector.h"

using namespace gg;

// Video capture
VideoCapture video;

// Handle keyboard events
void onKeyPress(GLFWwindow* window, unsigned int codepoint)
{
    // Activate tresholding (T key)
    if (codepoint == 116) {
        Tresholder tresholder(video.getLatestFrame());
    } else {
        cv::Mat mat = video.getLatestFrame();

        cv::Vec3b pixel = mat.at<cv::Vec3b>(0, 0);
        std::cout << (int) pixel[0] << " " << (int) pixel[1] << " " << (int) pixel[2] << std::endl;

        pixel = mat.at<cv::Vec3b>(0, 1);
        std::cout << (int) pixel[0] << " " << (int) pixel[1] << " " << (int) pixel[2] << std::endl;

        pixel = mat.at<cv::Vec3b>(1, 0);
        std::cout << (int) pixel[0] << " " << (int) pixel[1] << " " << (int) pixel[2] << std::endl;

        for (int i = 1280*3; i < 1280*3+16; ++i)
        {
            std::cout << (int) *(mat.ptr() + i) << std::endl;
        }
    }
}

// Main
int main()
{
    // Control panel
    ControlPanel controlPanel(1280, 360);

    // Open video device
    video.open(0);

    // Blob detector
    BlobDetector detector;

    // Add video source onto the control panel
    controlPanel.addSource(video.getTextureID(), 0, 0, 640, 360);
    controlPanel.addSource(detector.getTextureID(), 640, 0, 640, 360);

    // Keyboard input
    controlPanel.setCharCallback(onKeyPress);

    // Loop until the user closes the window
    while (controlPanel.isOpen())
    {
        // Get frame from video device
        video.capture();

        // Find blobs
        detector.processImage(video.getLatestFrame());

        // Redraw control panel
        controlPanel.redraw();
    }

    return 0;
}