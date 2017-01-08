//
// Created by Gutnar Leede on 08/01/2017.
//

#include <iostream>
#include "CL.h"

namespace gg {
    int CL::mOpenPrograms;
    cl_device_id CL::mDeviceId;
    cl_context CL::mContext;

    CL::CL()
    {
        // Increment number of open programs
        mOpenPrograms++;

        // Skip if CL already set up
        if (mOpenPrograms > 1) {
            return;
        }
        // OpenCL error code
        int err;

        // Connect to a compute device
        err = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &mDeviceId, NULL);

        if (err != CL_SUCCESS) {
            printf("Error: Failed to create a device group!\n");
            return;
        }

        // Create a compute context
        mContext = clCreateContext(0, 1, &mDeviceId, NULL, NULL, &err);

        if (!mContext) {
            printf("Error: Failed to create a compute context!\n");
            return;
        }
    }

    CL::~CL()
    {
        // Decrement number of open programs
        mOpenPrograms--;

        // Release program
        if (mProgram)
        {
            clReleaseProgram(mProgram);
        }

        // Close context if all programs are closed
        if (!mOpenPrograms)
        {
            clReleaseContext(mContext);
        }
    }

    void CL::compile(std::string filename)
    {
        // OpenCL error code
        int err;

        // Kernel source
        std::ifstream clFile("kernels/" + filename + ".cl");
        std::string clString(std::istreambuf_iterator<char>(clFile),
                             (std::istreambuf_iterator<char>()));
        const char* kernelSource = clString.c_str();

        // Create the compute program from the source buffer
        mProgram = clCreateProgramWithSource(mContext, 1, (const char **) & kernelSource, NULL, &err);

        if (!mProgram)
        {
            printf("Error: Failed to create compute program!\n");
            return;
        }

        // Build the program executable
        err = clBuildProgram(mProgram, 0, NULL, NULL, NULL, NULL);

        if (err != CL_SUCCESS)
        {
            size_t len;
            char buffer[2048];

            printf("Error: Failed to build program executable!\n");
            clGetProgramBuildInfo(mProgram, mDeviceId, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
            printf("%s\n", buffer);
            exit(1);
        }
    }

    void CL::setKernel(std::string name)
    {
        int err;
        mKernel = clCreateKernel(mProgram, name.c_str(), &err);
        mBufferArguments.clear();
    }

    CL::Buffer CL::createReadBuffer(size_t size, cl_uint argIndex)
    {
        BufferArgument bufferArgument;
        bufferArgument.buffer = clCreateBuffer(mContext, CL_MEM_READ_ONLY, size, NULL, NULL);
        bufferArgument.size = size;
        bufferArgument.flags = CL_MEM_READ_ONLY;
        mBufferArguments.push_back(bufferArgument);

        setArg(argIndex, size, &bufferArgument.buffer);

        return bufferArgument.buffer;
    }

    CL::Buffer CL::createWriteBuffer(size_t size, cl_uint argIndex)
    {
        BufferArgument bufferArgument;
        bufferArgument.buffer = clCreateBuffer(mContext, CL_MEM_WRITE_ONLY, size, NULL, NULL);
        bufferArgument.size = size;
        bufferArgument.flags = CL_MEM_WRITE_ONLY;
        mBufferArguments.push_back(bufferArgument);

        setArg(argIndex, sizeof(CL::Buffer), &bufferArgument.buffer);

        return bufferArgument.buffer;
    }

    void CL::setBufferData(CL::Buffer &buffer, void *dataPtr)
    {
        for (int i = 0; i < mBufferArguments.size(); ++i) {
            if (mBufferArguments[i].buffer == buffer) {
                if (mBufferArguments[i].flags == CL_MEM_READ_ONLY)
                {
                    mBufferArguments[i].readPtr = dataPtr;
                } else {
                    mBufferArguments[i].writePtr = dataPtr;
                }
            }
        }
    }

    void CL::setArg(cl_uint index, size_t size, const void *value)
    {
        clSetKernelArg(mKernel, index, size, value);
    }

    void CL::run(cl_uint dim, const size_t *workSize)
    {
        int err;

        cl_command_queue commands = clCreateCommandQueue(mContext, mDeviceId, 0, &err);

        for (int i = 0; i < mBufferArguments.size(); ++i) {
            if (mBufferArguments[i].flags == CL_MEM_READ_ONLY) {
                clEnqueueWriteBuffer(commands, mBufferArguments[i].buffer, CL_TRUE, 0, mBufferArguments[i].size, mBufferArguments[i].readPtr, 0, NULL, NULL);
            }
        }

        clEnqueueNDRangeKernel(commands, mKernel, dim, NULL, workSize, NULL, 0, NULL, NULL);
        clFinish(commands);

        for (int i = 0; i < mBufferArguments.size(); ++i) {
            if (mBufferArguments[i].flags == CL_MEM_WRITE_ONLY) {
                clEnqueueReadBuffer(commands, mBufferArguments[i].buffer, CL_TRUE, 0, mBufferArguments[i].size, mBufferArguments[i].writePtr, 0, NULL, NULL);
            }
        }
    }
}