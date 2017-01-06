#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Timer.h"
#include "VideoCapture.h"
#include "ControlPanel.h"
#include "BlobDetector.h"

using namespace gg;

int main()
{
    // Control panel
    ControlPanel controlPanel(1280, 720);

    // Video capture
    VideoCapture video(0);

    // Timer
    Timer timer;

    // Blob detector
    BlobDetector detector;

    // Add video source onto the control panel
    controlPanel.addSource(video.getTextureID(), 0, 0, 640, 360);
    controlPanel.addSource(detector.getTextureID(), 640, 0, 640, 360);

    // Loop until the user closes the window
    while (controlPanel.isOpen())
    {
        // Get frame from video device
        video.capture();

        // Find blobs
        //detector.processImage(video.getTextureID());
        detector.processImage(video.getLatestFrame());

        // Redraw control panel
        controlPanel.redraw();
    }

    return 0;
}