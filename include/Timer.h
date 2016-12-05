//
// Created by Gutnar Leede on 05/12/2016.
//

#ifndef GGROBOT_TIMER_H
#define GGROBOT_TIMER_H

#include <sys/time.h>


namespace gg {
    typedef long long int timestamp;

    class Timer
    {
    public:
        /// @brief Default constructor
        Timer();

        /// @brief Get number of milliseconds since the epoch
        /// @return Number of milliseconds since the epoch
        timestamp getTimestamp();

        /// @brief Get the current FPS
        /// @return The current FPS
        int getFPS();
    private:
        // Timestamp when this timer was created
        timestamp mStartTime;

        // Number of ticks since the FPS was last reported
        int mTicks;

        // Current measured average FPS
        int mCurrentFPS;

        // Timestamp when the last FPS value was reported
        timestamp mPreviousTimestamp;
    };
}


#endif //GGROBOT_TIMER_H
