# ipu6-camera-hal

This repository supports MIPI cameras through the IPU6SE on Intel Jasper Lake platforms. There are 4 repositories that provide the complete setup:

* https://github.com/intel/ipu6-drivers (branch:jsl_penguin_peak) - kernel drivers for the IPU and sensors
* https://github.com/intel/ipu6-camera-hal (branch:jsl_penguin_peak) - HAL for processing of images in userspace
* https://github.com/intel/ipu6-camera-bins (branch:jsl_penguin_peak) - IPU firmware and proprietary image processing libraries
* https://github.com/intel/icamerasrc (branch:icamerasrc_slim_api) - Gstreamer src plugin

## Content of this repository:
* IPU6SE HAL

## Build instructions:
* Dependencies: ipu6-camera-bins
* Dependencies: libexpat-dev automake libtool libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
* Building:
```
cd ipu6-camera-hal
mkdir -p ./build/out/install/usr && cd ./build/

cmake -DCMAKE_BUILD_TYPE=Release -DIPU_VER=ipu6sepla -DPRODUCTION_NAME=penguin_peak -DENABLE_VIRTUAL_IPU_PIPE=OFF -DUSE_PG_LITE_PIPE=ON -DUSE_STATIC_GRAPH=OFF -DCMAKE_INSTALL_PREFIX=/usr ..
# if don't want install to /usr, use -DCMAKE_INSTALL_PREFIX=./out/install/usr, export PKG_CONFIG_PATH="$workdir/build/out/install/usr/lib/pkgconfig"

make -j8
sudo make install
```

