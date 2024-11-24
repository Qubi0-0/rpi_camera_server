cmake_minimum_required(VERSION 3.10)

project(CameraServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    # /usr/include/libcamera
    /lib/cpp-httplib
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

set(SOURCES
    src/camera_server.cpp
)

add_executable(camera_server ${SOURCES})

target_link_libraries(camera_server
    # camera
    pthread
    ssl
    crypto
    # camera 
    # camera-base
)
