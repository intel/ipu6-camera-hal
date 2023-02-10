# ipu6-camera-hal

This repository supports MIPI cameras through the IPU6 on Intel Alder Lake, Raptor Lake and Meteor Lake platforms. There are 4 repositories that provide the complete setup:

- https://github.com/intel/ipu6-drivers (branch:ccg_plat_adlp) - kernel drivers for the IPU and sensors
- https://github.com/intel/ipu6-camera-hal (branch:ccg_plat_adlp) - HAL for processing of images in userspace
- https://github.com/intel/ipu6-camera-bins (branch:ccg_plat_adlp) - IPU firmware and proprietary image processing libraries
- https://github.com/intel/icamerasrc (branch:icamerasrc_slim_api) - Gstreamer src plugin


## Content of this repository:
- IPU6 HAL

## Build instructions:
- Dependencies: ipu6-camera-bins
- Dependencies: libexpat-dev automake libtool libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
- Build and install:

1. Set IPU6 version and product name for build

    - For Alder Lake and Raptor Lake, set:
        ```shell
        IPU6_VER=ipu6ep
        PRODUCTION_NAME=ccg_platform
        ```

    - For Meteor Lake, set:
        ```shell
        IPU6_VER=ipu6epmtl
        PRODUCTION_NAME=mtl-ubuntu
        ```

2. Then continue:
    ```shell
    cd ipu6-camera-hal
    mkdir -p ./build/out/install/usr && cd ./build/

    cmake -DCMAKE_BUILD_TYPE=Release \
    -DIPU_VER=${IPU6_VER} \
    -DPRODUCTION_NAME=${PRODUCTION_NAME} \
    -DENABLE_VIRTUAL_IPU_PIPE=OFF \
    -DUSE_PG_LITE_PIPE=ON \
    -DUSE_STATIC_GRAPH=OFF \
    -DCMAKE_INSTALL_PREFIX=/usr ..
    # if don't want install to /usr, use:
    # -DCMAKE_INSTALL_PREFIX=./out/install/usr and
    # export PKG_CONFIG_PATH="$workdir/build/out/install/usr/lib/pkgconfig"

    make -j`nproc`
    sudo make install
    ```
