# ipu6-camera-hal

This repository supports MIPI cameras through the IPU6 on Intel Tiger Lake, Alder Lake, Raptor Lake and Meteor Lake platforms.
There are 4 repositories that provide the complete setup:

- https://github.com/intel/ipu6-drivers/tree/ia_ipu6 - kernel drivers for the IPU and sensors
- https://github.com/intel/ipu6-camera-bins/tree/ia_ipu6 - IPU firmware and proprietary image processing libraries
- https://github.com/intel/ipu6-camera-hal/tree/ia_ipu6 - HAL for processing of images in userspace
- https://github.com/intel/icamerasrc/tree/icamerasrc_slim_api (branch:icamerasrc_slim_api) - Gstreamer src plugin

## Content of this repository:
- IPU6 HAL

## Build instructions:
- Dependencies: ipu6-camera-bins
    Please follow https://github.com/intel/ipu6-camera-bins/tree/ia_ipu6 README to install.

- Dependencies: libexpat-dev automake libtool libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libdrm-dev

- Build and install:
    ```
# Please follow common cmake, make & make install flow
cd ipu6-camera-hal
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr \
      -DCMAKE_INSTALL_LIBDIR=lib \
      -DBUILD_CAMHAL_ADAPTOR=ON \
      -DBUILD_CAMHAL_PLUGIN=ON \
      -DIPU_VERSIONS="ipu6ep" \
      -DUSE_PG_LITE_PIPE=ON \
      ..
make && sudo make install
     ```
Users can specify CMAKE_INSTALL_LIBDIR per your request.
