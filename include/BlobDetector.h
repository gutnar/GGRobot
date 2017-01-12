//
// Created by Gutnar Leede on 06/12/2016.
//

#ifndef GGROBOT_BLOBDETECTOR_H
#define GGROBOT_BLOBDETECTOR_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <OpenCL/opencl.h>
#include <GLFW/glfw3.h>
#include <cv.h>
#include <fstream>
#include "Blob.h"
#include "CL.h"
#include "Renderable.h"

namespace gg {
    class BlobDetector : public Renderable
    {
    public:
        /// @brief Default constructor
        /// @param width Image width to be processed
        /// @param height Image height to be processed
        BlobDetector(int width, int height);

        /// @brief Clean up OpenCL buffers
        ~BlobDetector();

        /// @brief Process texture
        //void processImage(GLuint texture);

        /// @brief Process OpenCV material
        void processImage(const cv::Mat &mat);

        /// @brief Get OpenGL texture ID
        GLuint getTextureID();

        /// @brief Render
        void render();

    private:
        /// @brief Connect same color pixels into blobs
        void findBlobs();

        // Image size
        int mWidth;
        int mHeight;
        int mNumberOfPixels;

        // OpenCL buffers
        CL::Buffer mBufferIn;
        CL::Buffer mBufferOut;

        // OpenCL output
        int* mColorMap;

        /// OpenGL
        // Shaders
        const GLchar *mVertexShaderSource =
                "#version 330 core\n"
                        "layout(location = 0) in vec2 position;"
                        "layout(location = 1) in vec2 texcoord;"
                        "out vec2 Texcoord;"
                        "void main()"
                        "{"
                        "    Texcoord = texcoord;"
                        "    gl_Position = vec4(position, 0.0, 1.0);"
                        "}";

        const GLchar *mFragmentShaderSource =
                "#version 330 core\n"
                        "in vec2 Texcoord;"
                        "layout(location = 0) out vec4 outColor;"
                        "uniform sampler2D tex;"
                        "void main()"
                        "{"
                        "    outColor = texture(tex, Texcoord);"
                        "}";

        // OpenGL texture ID
        GLuint mTextureID;

        // OpenCL instance
        CL mCLImageProcessor;

        // Detected blobs
        std::vector<Blob> mBlobs;
    };
}


#endif //GGROBOT_BLOBDETECTOR_H
