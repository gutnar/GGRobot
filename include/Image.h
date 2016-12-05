//
// Created by Gutnar Leede on 05/12/2016.
//

#ifndef GGROBOT_IMAGE_H
#define GGROBOT_IMAGE_H

#include <highgui.h>

namespace gg {
    class Image
    {
    public:
        /// @brief Create an image from an OpenCV material
        Image(cv::Mat &cvImage);
    private:

    };
}


#endif //GGROBOT_IMAGE_H
