cmake_minimum_required(VERSION 2.8)

project(camserv)

include_directories(.
libuv/include
zlib/include
#opencv/include
#opencv/modules/aruco/include
#opencv/modules/core/include
)

#set(BUILD_WITH_STATIC_CRT OFF)
#set(BUILD_DOCS OFF)
#set(BUILD_EXAMPLES OFF)
#set(BUILD_TESTS OFF)
#set(BUILD_PERF_TESTS OFF)
#set(BUILD_SHARED_LIBS OFF)
#set(BUILD_opencv_imgcodecs OFF)
#set(BUILD_opencv_videoio OFF)
#set(BUILD_opencv_highgui OFF)
#set(WITH_JPEG OFF)
#set(WITH_JASPER OFF)
#set(WITH_OPENEXR OFF)
#set(WITH_PNG OFF)
#set(WITH_TIFF OFF)
#set(WITH_WEBP OFF)
#set(WITH_FFMPEG OFF)
#set(WITH_VFW OFF)
#set(WITH_DSHOW OFF)
#set(WITH_MSMF OFF)

#add_subdirectory(opencv)

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "-Wall -Wno-unused-parameter -fno-strict-aliasing -O3")
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -g -std=c++11 ${CMAKE_CXX_FLAGS}")
    add_definitions(-DGCC)
else()
  #set(CMAKE_CXX_FLAGS_DEBUG "/MTd /Ob0 /Zl /Od /DWIN32 /D_WINDOWS /EHsc /WX /wd4355 /wd4251 /wd4250 /wd4996")
  file(GLOB OpenCV_LIBS *.lib)
endif()

add_subdirectory(libuv)
add_subdirectory(zlib)
add_subdirectory(drivers)

add_library(markpos
  markpos.cc
  markpos.h
)

#add_executable(marktest
#  png/png.cc
#  png/png.h
#  marktest.cc
#)

#target_link_libraries(marktest markpos)

add_executable(camserv
  http-parser/http_parser.c
  http-parser/http_parser.h
  png/png.cc
  png/png.h
  main.cc
)

target_link_libraries(camserv libuv markpos zlib webcam)
