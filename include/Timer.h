//
// Created by Gutnar Leede on 05/12/2016.
//

#ifndef GGROBOT_TIMER_H
#define GGROBOT_TIMER_H

#include <GLFW/glfw3.h>


namespace gg {
    class Timer
    {
    public:
        /// @brief Default constructor
        Timer();

        /// @brief Get number of milliseconds since initialization
        /// @return Number of milliseconds since initialization
        double getTime();

        /// @brief Get the current FPS
        /// @return The current FPS
        int getFPS();
    private:
        // Time when this timer was created
        double mStartTime;

        // Number of ticks since the FPS was last reported
        int mTicks;

        // Current measured average FPS
        int mCurrentFPS;

        // Time when the last FPS value was reported
        double mPreviousTime;
    };
}


#endif //GGROBOT_TIMER_H
