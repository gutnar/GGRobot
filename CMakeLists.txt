# GGRobot
cmake_minimum_required(VERSION 3.6)
project(GGRobot)

# Flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Sources
set(SOURCE_FILES tests/main.cpp lib/Timer.cpp lib/VideoCapture.cpp lib/ControlPanel.cpp lib/BlobDetector.cpp lib/Blob.cpp lib/Tresholder.cpp)
add_executable(GGRobot ${SOURCE_FILES})

# Link OpenCL
find_package(OpenCL REQUIRED)
include_directories(${OpenCL_INCLUDE_DIRS})
link_directories(${OpenCL_LIBRARY})
target_link_libraries(GGRobot ${OpenCL_LIBRARY})

# Link OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(GGRobot ${OpenCV_LIBS})

# Link GLFW
#find_package(glfw3 REQUIRED)
#find_package(glfw3 3.2 REQUIRED)
#find_package(OpenGL REQUIRED)
#include_directories(${glfw3_INCLUDE_DIR})
#target_link_libraries(GGRobot ${glfw3_LIBRARY})

# OS X dependencies
if(APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/lib -lglfw3 -lglew")
endif(APPLE)

# Include directories
target_include_directories(GGRobot PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib ${CMAKE_CURRENT_SOURCE_DIR}/include)
