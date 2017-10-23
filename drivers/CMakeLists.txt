cmake_minimum_required(VERSION 2.6)

add_library(nusb
  usb/nusb.cpp
  usb/nusb.h
)

add_library(webcam
  webcam.cc
  webcam.h
)
