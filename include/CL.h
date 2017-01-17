//
// Created by Gutnar Leede on 08/01/2017.
//

#ifndef GGROBOT_CL_H
#define GGROBOT_CL_H

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include <OpenCL/opencl.h>


namespace gg {

    class CL
    {
    public:
        // OpenCL buffer type on Apple
        typedef cl_mem Buffer;

        /// @brief Default constructor
        CL();

        /// @brief Close OpenCL context when closed
        ~CL();

        /// @brief Load a program
        /// @param filename File name in kernels directory
        void compile(std::string filename);

        /// @brief Choose a kernel
        /// @param name Function name
        void setKernel(std::string name);

        /// @brief Create a read-only buffer
        /// @param size Number of bytes in the buffer
        Buffer createReadBuffer(size_t size, cl_uint argIndex);

        /// @brief Create a write-only buffer
        /// @param size Number of bytes in the buffer
        Buffer createWriteBuffer(size_t size, cl_uint argIndex);

        /// @brief Set buffer data pointer
        /// @param ptr Buffer data pointer
        void setBufferData(CL::Buffer &buffer, void *ptr);

        /// @brief Set kernel argument
        /// @param index Argument index
        /// @param size Size of the value
        /// @param value Value
        void setArg(cl_uint index, size_t size, const void *value);

        /// @brief Run OpenCL program
        /// @param dim Number of dimensions in the work group
        /// @param workSize Global work size
        void run(cl_uint dim, const size_t *workSize);

        /// @brief Release kernel and buffers
        void releaseKernel();

        // Global OpenCL variables
        static cl_device_id mDeviceId;
        static cl_context mContext;

        // Local OpenCL variables
        cl_program mProgram;
        cl_kernel mKernel;

    private:

        // Number of currently open programs
        static int mOpenPrograms;

        // Buffer argument
        struct BufferArgument {
            Buffer buffer;
            size_t size;
            cl_mem_flags flags;
            const void *readPtr;
            void *writePtr;
        };

        // Current buffers
        std::vector<BufferArgument> mBufferArguments;
    };
}

#endif //GGROBOT_CL_H
