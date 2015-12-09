cmake_minimum_required(VERSION 2.6)

project(cccwebcam)
add_executable(cccwebcam main.cpp)
target_link_libraries(cccwebcam curl opencv_core opencv_highgui opencv_imgproc GL zbar)
set_property(TARGET cccwebcam PROPERTY CXX_STANDARD 11)

install(TARGETS cccwebcam RUNTIME DESTINATION bin)
