# ipu6-camera-hal

This repository supports MIPI cameras through the IPU6 on Intel Tiger Lake, Alder Lake, Raptor Lake and Meteor Lake platforms.
There are 4 repositories that provide the complete setup:

- https://github.com/intel/ipu6-drivers - kernel drivers for the IPU and sensors
- https://github.com/intel/ipu6-camera-bins - IPU firmware and proprietary image processing libraries
- https://github.com/intel/ipu6-camera-hal - HAL for processing of images in userspace
- https://github.com/intel/icamerasrc/tree/icamerasrc_slim_api (branch:icamerasrc_slim_api) - Gstreamer src plugin

## Content of this repository:
- IPU6 HAL

## Build instructions:
- Dependencies: ipu6-camera-bins
    Please follow https://github.com/intel/ipu6-camera-bins README to install.

- Dependencies: libexpat-dev automake libtool libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libdrm-dev

- Build and install:
    Recommend to build together with icamerasrc and using `build.sh` in this repo.
1. Put ipu6-camera-hal, icameasrc and build.sh parallel as below:
    ```
    cp build.sh .. && cd ..
    # work
    # ├── build.sh
    # ├── icamerasrc
    # ├── ipu6-camera-hal
    # └── out
    ```

2. For example, in `work` folder, run `./build.sh -d --board ipu_mtl`. Output binaries will be in `out/ipu_mtl/install`.
   Notice: Use `./build.sh --board ipu_mtl` to disable dma build.
           The dma build depends on 'gstreamer >= 1.23' and 'libva-drm'.

3. Install built out drivers in `out/<target>/install` to target:
    ```sh
    cp -r ./out/<target>/install/etc* /etc/
    cp -r ./out/<target>/install/usr/include/* /usr/include/
    cp -r ./out/<target>/install/usr/lib/* /usr/lib/
    ```

For more building details please reference the `build.sh`.
