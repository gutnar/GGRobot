# GGRobot
cmake_minimum_required(VERSION 3.6)
project(GGRobot)

# Flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Sources
set(SOURCE_FILES tests/main.cpp lib/Timer.cpp lib/VideoCapture.cpp lib/Image.cpp lib/ControlPanel.cpp lib/BlobDetector.cpp)
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

# Link
target_include_directories(GGRobot PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib ${CMAKE_CURRENT_SOURCE_DIR}/include)
