#ifndef GGROBOT_CONTROLPANEL_H
#define GGROBOT_CONTROLPANEL_H

#include <cstdio>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Renderable.h"

namespace gg {
    // Callback function type
    typedef void (*callbackType)(GLFWwindow* window, unsigned int codepoint);

    class ControlPanel
    {
    public:
        /// @brief Create a control panel with the default size 640x480
        ControlPanel();

        /// @brief Close the control panel
        ~ControlPanel();

        /// @brief Create a control panel with a certain size
        /// @param windowWidth Window width
        /// @param windowHeight Window height
        ControlPanel(int windowWidth, int windowHeight);

        /// @brief Create a control panel with a certain size and title
        /// @param windowWidth Window width
        /// @param windowHeight Window height
        /// @param windowTitle Window title
        ControlPanel(int windowWidth, int windowHeight, const char* windowTitle);

        /// @brief Add a texture source onto the control panel
        /// @param textureID OpenGL texture ID
        /// @param leftOffset Offset from left side of the window in pixels
        /// @param topOffset Offset from top side of the window in pixels
        /// @param width Image width in pixels on control panel
        /// @param height Image height in pixels on control panel
        void addSource(GLuint textureID, int leftOffset, int topOffset, int width, int height);

        /// @brief Add a renderable object onto the control panel
        /// @param renderable Renderable object
        /// @param leftOffset Offset from left side of the window in pixels
        /// @param topOffset Offset from top side of the window in pixels
        /// @param width Image width in pixels on control panel
        /// @param height Image height in pixels on control panel
        void addSource(Renderable &renderable, int leftOffset, int topOffset, int width, int height);

        /// @brief Chek if the window open
        /// @return True if the window is open or false
        bool isOpen();

        /// @brief Redraw control panel content
        void redraw();

        /// @brief Add keyboard input event handler
        void setCharCallback(callbackType callback);
    private:
        // GLFW window
        GLFWwindow* mWindow;
        int mWindowWidth;
        int mWindowHeight;

        // Shaders
        const GLchar* mVertexShaderSource =
                "#version 330 core\n"
                "layout(location = 0) in vec2 position;"
                "layout(location = 1) in vec2 texcoord;"
                "out vec2 Texcoord;"
                "void main()"
                "{"
                "    Texcoord = texcoord;"
                "    gl_Position = vec4(position, 0.0, 1.0);"
                "}";

        const GLchar* mFragmentShaderSource =
                "#version 330 core\n"
                "in vec2 Texcoord;"
                "layout(location = 0) out vec4 outColor;"
                "uniform sampler2D tex;"
                "void main()"
                "{"
                "    outColor = texture(tex, Texcoord);"
                "}";

        // Create an OpenGL context and open a window
        void createWindow(int windowWidth, int windowHeight, const char* windowTitle);

        // Source image elements
        GLuint mSourceImageElementBufferObject;

        // Source image structure
        struct SourceImage {
            GLuint textureID;
            GLuint vertexBufferObject;
            float width;
            float height;
        };

        // Sources
        std::vector<SourceImage> mSources;

        // Renderable objects
        std::vector<Renderable*> mRenderables;
    };
}


#endif //GGROBOT_CONTROLPANEL_H
