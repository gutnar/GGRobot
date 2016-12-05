#include <opencv2/highgui.hpp>
#include <iostream>

#include "Timer.h"
#include "VideoCapture.h"
#include "ControlPanel.h"
#include "BlobDetector.h"

using namespace std;
using namespace cv;
using namespace gg;

int main()
{
    gg::VideoCapture video(0);
    Timer timer;

    // Blob detector
    BlobDetector detector;

    // Get reference to latest captured image
    Mat image = video.getLatestFrame();

    // Control panel
    namedWindow("test");

    int i = 0;

    while (true) {
        putText(image, to_string(timer.getFPS()), Point(20, 20), 1, 1, Scalar(0, 0, 255));
        imshow("test", image);

        if (++i % 3 == 0) {
            if (waitKey(1) > 0) {
                break;
            }
        }
    }

    return 0;
}