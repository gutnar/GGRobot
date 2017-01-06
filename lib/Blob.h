//
// Created by Gutnar Leede on 06/01/2017.
//

#ifndef GGROBOT_BLOB_H
#define GGROBOT_BLOB_H

#include <vector>
#include <opencv2/highgui.hpp>

namespace gg {
    struct BlobLine
    {
        int y;
        int xi;
        int xf;
        int color;
        int blobIndex;
    };

    class Blob
    {
    public:
        Blob(int color);

        void addLine(BlobLine &line);

        void addBlob(Blob &blob);

        int mColor;

        int mMinY = 2048;
        int mMaxY = 0;
        int mMinX = 2048;
        int mMaxX = 0;

        int mSumX = 0;
        int mSumY = 0;

        int mSurface = 0;

        bool mHidden = false;

        std::vector<BlobLine> mLines;

        int getWidth();

        int getHeight();

        int getSurface();

        cv::Point getCenter();

        cv::Point getMassCenter();
    };
}

#endif //GGROBOT_BLOB_H
