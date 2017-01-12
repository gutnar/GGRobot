#include "ControlPanel.h"

namespace gg {
    void character_callback(GLFWwindow* window, unsigned int codepoint)
    {
        printf("test %d", codepoint);
    }

    ControlPanel::ControlPanel()
    {
        createWindow(640, 480, "Control panel");
    }

    ControlPanel::ControlPanel(int windowWidth, int windowHeight)
    {
        createWindow(windowWidth, windowHeight, "Control panel");
    }

    ControlPanel::ControlPanel(int windowWidth, int windowHeight, const char *windowTitle)
    {
        createWindow(windowWidth, windowHeight, windowTitle);
    }

    ControlPanel::~ControlPanel()
    {
        glfwTerminate();
    }

    // Create an OpenGL context and open a window
    void ControlPanel::createWindow(int windowWidth, int windowHeight, const char *windowTitle)
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
        mWindow = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL);

        if (!mWindow)
        {
            glfwTerminate();
            return;
        }

        /* Make the window's context current */
        glfwMakeContextCurrent(mWindow);

        // Initialize GLEW
        glewExperimental = GL_TRUE;

        if (glewInit() != GLEW_OK)
        {
            std::fprintf(stderr, "Failed to initialize GLEW\n");
            return;
        }

        // Get version info
        const GLubyte *renderer = glGetString(GL_RENDERER); // get renderer string
        const GLubyte *version = glGetString(GL_VERSION); // version as a string
        printf("Renderer: %s\n", renderer);
        printf("OpenGL version supported %s\n", version);

        // Vertex array object
        GLuint VertexArrayID;
        glGenVertexArrays(1, &VertexArrayID);
        glBindVertexArray(VertexArrayID);

        // Create and compile the vertex shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &mVertexShaderSource, NULL);
        glCompileShader(vertexShader);

        // Create and compile the fragment shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &mFragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        // Link the vertex and fragment shader into a shader program
        GLuint shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        glUseProgram(shaderProgram);

        // Source images as rectangles are composed of two triangles
        glGenBuffers(1, &mSourceImageElementBufferObject);

        GLuint elements[] = {
                0, 1, 2,
                2, 3, 0
        };

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mSourceImageElementBufferObject);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

        // Remember window size
        mWindowWidth = windowWidth;
        mWindowHeight = windowHeight;
    }

    /// Set keyboard event callback
    void ControlPanel::setCharCallback(callbackType callback)
    {
        glfwSetCharCallback(mWindow, callback);
    }

    /// Add a texture source onto the control panel with a certain size
    void ControlPanel::addSource(GLuint textureID, int leftPixelOffset, int topPixelOffset, int width, int height)
    {
        // New source
        SourceImage source;

        // Texture
        source.textureID = textureID;

        // Size
        source.width = (float) width / mWindowWidth * 2;
        source.height = (float) height / mWindowHeight * 2;

        // Offset
        float leftOffset = -1.0f + (float) leftPixelOffset / mWindowWidth * 2;
        float topOffset = 1.0f - (float) topPixelOffset / mWindowHeight * 2;

        // Buffer object
        glGenBuffers(1, &source.vertexBufferObject);

        GLfloat vertices[] = {
                // Position   Texcoords
                leftOffset, topOffset, 0.0f, 0.0f,
                leftOffset + source.width, topOffset, 1.0f, 0.0f,
                leftOffset + source.width, topOffset - source.height, 1.0f, 1.0f,
                leftOffset, topOffset - source.height, 0.0f, 1.0f
        };

        glBindBuffer(GL_ARRAY_BUFFER, source.vertexBufferObject);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Add to sources vector
        mSources.push_back(source);
    }

    /// Add a renderable object onto the control panel
    void ControlPanel::addSource(Renderable &renderable, int leftOffset, int topOffset, int width, int height)
    {
        mRenderables.push_back(&renderable);
    }

    /// Check if the window is open
    bool ControlPanel::isOpen()
    {
        return !glfwWindowShouldClose(mWindow);
    }

    /// Redraw control panel content
    void ControlPanel::redraw()
    {
        // Clear
        glClear(GL_COLOR_BUFFER_BIT);

        // Render objects
        for (int i = 0; i < mRenderables.size(); ++i) {
            mRenderables[i]->render();
        }

        // Draw source images
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mSourceImageElementBufferObject);

        for (int i = 0; i < mSources.size(); ++i)
        {
            // Specify the layout of the vertex data
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *) (2 * sizeof(GLfloat)));

            // Bind vertices
            glBindBuffer(GL_ARRAY_BUFFER, mSources[i].vertexBufferObject);

            // Bind texture
            glBindTexture(GL_TEXTURE_2D, mSources[i].textureID);

            // Draw a rectangle from the 2 triangles using 6 indices
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        // Swap front and back buffers
        glfwSwapBuffers(mWindow);

        // Poll for and process events
        glfwPollEvents();
    }
}