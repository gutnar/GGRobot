//
// Created by Gutnar Leede on 05/12/2016.
//

#ifndef GGROBOT_CONTROLPANEL_H
#define GGROBOT_CONTROLPANEL_H

#include <cstdio>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace gg {
    class ControlPanel
    {
    public:
        GLFWwindow* window;
        ControlPanel();
    };
}


#endif //GGROBOT_CONTROLPANEL_H
