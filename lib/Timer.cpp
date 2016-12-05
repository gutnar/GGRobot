//
// Created by Gutnar Leede on 05/12/2016.
//

#include "Timer.h"

namespace gg {

    Timer::Timer()
    {
        mStartTime = getTimestamp();
    }

    /// Finds the number of milliseconds elapsed since the epoch (January 1, 1970)
    timestamp Timer::getTimestamp()
    {
        timeval tp;
        gettimeofday(&tp, NULL);
        return tp.tv_sec * 1000 + tp.tv_usec / 1000;
        //return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    /// Measures the frames per seconds based on the last 10 frames
    int Timer::getFPS()
    {
        // Calculate new FPS when 10 frames reached
        // FPS = (10 frames) per (time between 10 frames in milliseconds) times
        if (mTicks == 10) {
            mTicks = 0;
            timestamp currentTimestamp = getTimestamp();
            mCurrentFPS = (int) (10.0 / (currentTimestamp - mPreviousTimestamp) * 1000);
            mPreviousTimestamp = currentTimestamp;
        }

        // Increment number of ticks before returning to prevent division by zero on first frame
        mTicks++;

        return mCurrentFPS;
    }
}