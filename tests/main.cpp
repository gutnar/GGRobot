#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Tresholder.h>

#include "Timer.h"
#include "VideoCapture.h"
#include "ControlPanel.h"
#include "BlobDetector.h"

using namespace gg;

// Control panel
ControlPanel controlPanel(1280, 360);

// Video capture
VideoCapture video;

// Tresholder
Tresholder tresholder;

// Blob detector
BlobDetector detector(640, 360);

// Whether to draw blobs on camera feed
bool drawBlobs = true;

// Handle keyboard events
void onKeyPress(GLFWwindow* window, unsigned int codepoint)
{
    // Activate tresholding (T)
    if (codepoint == 116)
    {
        tresholder.processImage(video.getLatestFrame());
    }

    // Save tresholded colors (S)
    else if (codepoint == 115) {
        tresholder.save();
        detector.reloadConfiguration();
    }

    // Toggle showing blobs (B)
    else if (codepoint == 98) {
        drawBlobs = !drawBlobs;
    }

    // Quit (Q)
    else if (codepoint == 113) {
        controlPanel.close();
    }

    // Show pressed key
    else {
        std::cout << codepoint << std::endl;
    }
}

// Main
int main()
{
    // Open video device
    video.open(0);

    // Add video source onto the control panel
    controlPanel.addSource(video.getTextureID(), 0, 0, 640, 360);

    // Add blob detector view onto the control panel
    //controlPanel.addSource(detector, 640, 0, 640, 360);
    controlPanel.addSource(detector.getTextureID(), 640, 0, 640, 360);

    // Keyboard input
    controlPanel.setCharCallback(onKeyPress);

    // Timer
    Timer timer;
    int frames = 0;
    double lastTime = timer.getTime();

    // Loop until the user closes the window
    while (controlPanel.isOpen())
    {
        // Get frame from video device
        video.capture();
        cv::Mat frame = video.getLatestFrame();

        // Find blobs
        detector.processImage(video.getLatestFrame());

        // Draw blobs
        if (drawBlobs)
        {
            std::vector<gg::Blob> blobs = detector.getBlobs();

            for (int i = 0; i < blobs.size(); ++i)
            {
                cv::line(frame, cv::Point(blobs[i].mMinX, blobs[i].mMinY), cv::Point(blobs[i].mMaxX, blobs[i].mMinY),
                         cv::Scalar(255, 255, 0));
                cv::line(frame, cv::Point(blobs[i].mMinX, blobs[i].mMaxY), cv::Point(blobs[i].mMaxX, blobs[i].mMaxY),
                         cv::Scalar(255, 255, 0));
                cv::line(frame, cv::Point(blobs[i].mMinX, blobs[i].mMinY), cv::Point(blobs[i].mMinX, blobs[i].mMaxY),
                         cv::Scalar(255, 255, 0));
                cv::line(frame, cv::Point(blobs[i].mMaxX, blobs[i].mMinY), cv::Point(blobs[i].mMaxX, blobs[i].mMaxY),
                         cv::Scalar(255, 255, 0));
            }

            video.generateTexture();
        }

        // Print FPS
        if (++frames == 50) {
            double newTime = timer.getTime();
            std::cout << 1000*(newTime - lastTime)/50 << std::endl;
            lastTime = newTime;
            frames = 0;
        }

        // Redraw control panel
        controlPanel.redraw();
    }

    return 0;
}