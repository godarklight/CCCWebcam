cmake_minimum_required(VERSION 2.6)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++14)
project(cccwebcam)
add_executable(cccwebcam main.cpp)
target_link_libraries(cccwebcam curl opencv_core opencv_highgui opencv_imgproc GL zbar)

install(TARGETS cccwebcam RUNTIME DESTINATION bin)
