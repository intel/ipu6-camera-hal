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

2. In `work` folder, run `./build.sh`. Output binaries will be in `out/install`. (RPM build is not implemented yet)

3. Install contents in `out/install` to your `/usr`:
    ```sh
    sudo cp -r ./out/install/etc/* /etc/
    sudo cp -r ./out/install/include/* /usr/include/
    sudo cp -r ./out/install/lib/* /usr/lib/
    sudo cp -r ./out/install/usr/* /usr/
    ```

For more building details please reference the `build.sh`.