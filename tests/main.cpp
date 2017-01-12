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

// Tresholder
Tresholder tresholder;

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
    }

    // Show pressed key
    else {
        std::cout << codepoint << std::endl;
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
    BlobDetector detector(640, 360);

    // Add video source onto the control panel
    controlPanel.addSource(video.getTextureID(), 0, 0, 640, 360);

    // Add blob detector view onto the control panel
    //controlPanel.addSource(detector, 640, 0, 640, 360);
    //controlPanel.addSource(detector.getTextureID(), 640, 0, 640, 360);

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