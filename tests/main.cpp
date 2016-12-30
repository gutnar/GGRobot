#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Timer.h"
#include "VideoCapture.h"
#include "ControlPanel.h"
#include "BlobDetector.h"

using namespace std;
using namespace gg;

// Shader sources
const GLchar* vertexSource =
        "#version 330 core\n"
                "in vec2 position;"
                "in vec3 color;"
                "in vec2 texcoord;"
                "out vec3 Color;"
                "out vec2 Texcoord;"
                "void main()"
                "{"
                "    Color = color;"
                "    Texcoord = texcoord;"
                "    gl_Position = vec4(position, 0.0, 1.0);"
                "}";
const GLchar* fragmentSource =
        "#version 330 core\n"
                "in vec3 Color;"
                "in vec2 Texcoord;"
                "out vec4 outColor;"
                "uniform sampler2D tex;"
                "void main()"
                "{"
                "    outColor = texture(tex, Texcoord) * vec4(Color, 1.0);"
                "}";

int main()
{
    // Control panel
    ControlPanel controlPanel;

    // Video capture
    VideoCapture video(0);
    GLuint texture = video.getTextureID();

    // Timer
    Timer timer;

    // Blob detector
    BlobDetector detector;

    // Vertex array object
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile the vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    // Create and compile the fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    // Link the vertex and fragment shader into a shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    // Create a Vertex Buffer Object and copy the vertex data to it
    GLuint vbo;
    glGenBuffers(1, &vbo);

    GLfloat vertices[] = {
            //  Position      Color             Texcoords
            -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, // Top-left
            0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, // Top-right
            0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
            -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f  // Bottom-left
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Specify the layout of the vertex data
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);

    GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
    glEnableVertexAttribArray(colAttrib);
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

    GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));

    // Create an element array
    GLuint ebo;
    glGenBuffers(1, &ebo);

    GLuint elements[] = {
            0, 1, 2,
            2, 3, 0
    };

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

    // Bind texture
    glBindTexture(GL_TEXTURE_2D, texture);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(controlPanel.window))
    {
        // Clear frame
        glClear(GL_COLOR_BUFFER_BIT);

        // Get frame
        video.capture();

        // Draw a rectangle from the 2 triangles using 6 indices
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        /* Swap front and back buffers */
        glfwSwapBuffers(controlPanel.window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}