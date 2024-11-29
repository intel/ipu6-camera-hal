# Install script for directory: /home/yons/workplace/git_release/forked/ipu6-camera-hal/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/yons/workplace/git_release/forked/ipu6-camera-hal/build/src/3a/cmake_install.cmake")
  include("/home/yons/workplace/git_release/forked/ipu6-camera-hal/build/src/core/cmake_install.cmake")
  include("/home/yons/workplace/git_release/forked/ipu6-camera-hal/build/src/hal/cmake_install.cmake")
  include("/home/yons/workplace/git_release/forked/ipu6-camera-hal/build/src/iutils/cmake_install.cmake")
  include("/home/yons/workplace/git_release/forked/ipu6-camera-hal/build/src/scheduler/cmake_install.cmake")
  include("/home/yons/workplace/git_release/forked/ipu6-camera-hal/build/src/metadata/cmake_install.cmake")
  include("/home/yons/workplace/git_release/forked/ipu6-camera-hal/build/src/platformdata/cmake_install.cmake")
  include("/home/yons/workplace/git_release/forked/ipu6-camera-hal/build/src/v4l2/cmake_install.cmake")
  include("/home/yons/workplace/git_release/forked/ipu6-camera-hal/build/src/image_process/cmake_install.cmake")

endif()

