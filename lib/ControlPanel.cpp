//
// Created by Gutnar Leede on 05/12/2016.
//

#include "ControlPanel.h"

namespace gg {
    ControlPanel::ControlPanel()
    {
        // Initialize GLFW
        if (!glfwInit())
        {
            return;
        }

        // Apple only! Maybe!
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        /* Create a windowed mode window and its OpenGL context */
        window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);

        if (!window)
        {
            glfwTerminate();
            return;
        }

        /* Make the window's context current */
        glfwMakeContextCurrent(window);

        // Initialize GLEW
        glewExperimental = GL_TRUE;

        if (glewInit() != GLEW_OK) {
            std::fprintf(stderr, "Failed to initialize GLEW\n");
            return;
        }

        // Get version info
        const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
        const GLubyte* version = glGetString(GL_VERSION); // version as a string
        printf("Renderer: %s\n", renderer);
        printf("OpenGL version supported %s\n", version);
    }
}