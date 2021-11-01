# ipu6-camera-hal

This repository supports MIPI cameras through the IPU6 on Intel Tiger Lake and
Alder Lake platforms. There are 4 repositories that provide the complete setup:

- https://github.com/intel/ipu6-drivers - kernel drivers for the IPU and sensors
- https://github.com/intel/ipu6-camera-hal - HAL for processing of images in userspace
- https://github.com/intel/ipu6-camera-bins (branch: main for Tiger Lake, tributo
  for Alder Lake) - IPU firmware and proprietary image processing libraries
- https://github.com/intel/icamerasrc (branch:icamerasrc_slim_api) - Gstreamer src plugin

## Content of this repository:
- IPU6 HAL

## Build instructions:
- Dependencies: ipu6-camera-bins
- Dependencies: libexpat-dev automake libtool libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
- Build and install:

1. Set IPU6 version and product name for build
    - For Tiger Lake, set:
        ```shell
        IPU6_VER=ipu6
        PRODUCT_NAME=Andrews
        ```

    - For Alder Lake, set:
        ```shell
        IPU6_VER=ipu6ep
        PRODUCT_NAME=ccg_cce_tributo
        ```

2. Then continue:
    ```shell
    cd ipu6-camera-hal
    mkdir -p ./build/out/install/usr && cd ./build/

    cmake -DCMAKE_BUILD_TYPE=Release \
    -DIPU_VER=${IPU6_VER} \
    -DPRODUCTION_NAME=${PRODUCT_NAME} \
    -DENABLE_VIRTUAL_IPU_PIPE=OFF \
    -DUSE_PG_LITE_PIPE=ON \
    -DUSE_STATIC_GRAPH=OFF \
    -DCMAKE_INSTALL_PREFIX=/usr ..
    # if don't want install to /usr, use -DCMAKE_INSTALL_PREFIX=./out/install/usr, export PKG_CONFIG_PATH="$workdir/build/out/install/usr/lib/pkgconfig"

    make -j`nproc`
    sudo make install
    ```
    
3. Move user space driver for different platforms:
    ```shell
    mkdir /usr/lib/${IPU6_VER}
    mkdir /usr/share/defaults/etc/${IPU6_VER}

    mv /usr/lib/libtbd.so.0.0.0 /usr/lib/${IPU6_VER}/libtbd.so.0.0.0
    
    mv /usr/lib/libSkyCamAIC.so /usr/lib/${IPU6_VER}/libSkyCamAIC.so
    mv /usr/lib/libSkyCamAICKBL.so /usr/lib/${IPU6_VER}/libSkyCamAICKBL.so
    mv /usr/lib/libStatsConverter.so /usr/lib/${IPU6_VER}/libStatsConverter.so
    mv /usr/lib/libbroxton_ia_pal.so /usr/lib/${IPU6_VER}/libbroxton_ia_pal.so
    mv /usr/lib/libia_aec.so /usr/lib/${IPU6_VER}/libia_aec.so
    mv /usr/lib/libia_aic.so /usr/lib/${IPU6_VER}/libia_aic.so
    mv /usr/lib/libia_aic_engine.so /usr/lib/${IPU6_VER}/libia_aic_engine.so
    mv /usr/lib/libia_aic_myriad.so /usr/lib/${IPU6_VER}/libia_aic_myriad.so
    mv /usr/lib/libia_aiq.so /usr/lib/${IPU6_VER}/libia_aiq.so
    mv /usr/lib/libia_aiq_file_debug.so /usr/lib/${IPU6_VER}/libia_aiq_file_debug.so
    mv /usr/lib/libia_aiqb_parser.so /usr/lib/${IPU6_VER}/libia_aiqb_parser.so
    mv /usr/lib/libia_bcomp.so /usr/lib/${IPU6_VER}/libia_bcomp.so
    mv /usr/lib/libia_cca.so /usr/lib/${IPU6_VER}/libia_cca.so
    mv /usr/lib/libia_cca_file_debug.so /usr/lib/${IPU6_VER}/libia_cca_file_debug.so
    mv /usr/lib/libia_ccat.so /usr/lib/${IPU6_VER}/libia_ccat.so
    mv /usr/lib/libia_cmc_parser.so /usr/lib/${IPU6_VER}/libia_cmc_parser.so
    mv /usr/lib/libia_coordinate.so /usr/lib/${IPU6_VER}/libia_coordinate.so
    mv /usr/lib/libia_dpc.so /usr/lib/${IPU6_VER}/libia_dpc.so
    mv /usr/lib/libia_dvs.so /usr/lib/${IPU6_VER}/libia_dvs.so
    mv /usr/lib/libia_dvs_file_debug.so /usr/lib/${IPU6_VER}/libia_dvs_file_debug.so
    mv /usr/lib/libia_emd_decoder.so /usr/lib/${IPU6_VER}/libia_emd_decoder.so
    mv /usr/lib/libia_exc.so /usr/lib/${IPU6_VER}/libia_exc.so
    mv /usr/lib/libia_intrinsics.so /usr/lib/${IPU6_VER}/libia_intrinsics.so
    mv /usr/lib/libia_isp_bxt.so /usr/lib/${IPU6_VER}/libia_isp_bxt.so
    mv /usr/lib/libia_isp_bxt_file_debug.so /usr/lib/${IPU6_VER}/libia_isp_bxt_file_debug.so
    mv /usr/lib/libia_lard.so /usr/lib/${IPU6_VER}/libia_lard.so
    mv /usr/lib/libia_log.so /usr/lib/${IPU6_VER}/libia_log.so
    mv /usr/lib/libia_ltm.so /usr/lib/${IPU6_VER}/libia_ltm.so
    mv /usr/lib/libia_ltm_file_debug.so /usr/lib/${IPU6_VER}/libia_ltm_file_debug.so
    mv /usr/lib/libia_me_corner.so /usr/lib/${IPU6_VER}/libia_me_corner.so
    mv /usr/lib/libia_mkn.so /usr/lib/${IPU6_VER}/libia_mkn.so
    mv /usr/lib/libia_nvm.so /usr/lib/${IPU6_VER}/libia_nvm.so
    mv /usr/lib/libia_ob.so /usr/lib/${IPU6_VER}/libia_ob.so
    mv /usr/lib/libia_pd.so /usr/lib/${IPU6_VER}/libia_pd.so
    mv /usr/lib/libia_view.so /usr/lib/${IPU6_VER}/libia_view.so
    mv /usr/lib/libpvl_blink_detection.so /usr/lib/${IPU6_VER}/libpvl_blink_detection.so
    mv /usr/lib/libpvl_eye_detection.so /usr/lib/${IPU6_VER}/libpvl_eye_detection.so
    mv /usr/lib/libpvl_face_detection.so /usr/lib/${IPU6_VER}/libpvl_face_detection.so
    mv /usr/lib/libpvl_face_recognition.so /usr/lib/${IPU6_VER}/libpvl_face_recognition.so
    mv /usr/lib/libpvl_mouth_detection.so /usr/lib/${IPU6_VER}/libpvl_mouth_detection.so
    mv /usr/lib/libpvl_object_tracking.so /usr/lib/${IPU6_VER}/libpvl_object_tracking.so
    mv /usr/lib/libpvl_smile_detection.so /usr/lib/${IPU6_VER}/libpvl_smile_detection.so
    
    mv /usr/lib/libcisu.so.0.0.0 /usr/lib/${IPU6_VER}/libcisu.so.0.0.0
    mv /usr/lib/libcisu_os_plugin.so.1.0.0 /usr/lib/${IPU6_VER}/libcisu_os_plugin.so.1.0.0
    mv /usr/lib/libgcss.so.0.0.0 /usr/lib/${IPU6_VER}/libgcss.so.0.0.0
    mv /usr/lib/libia_camera.so.0.0.0 /usr/lib/${IPU6_VER}/libia_camera.so.0.0.0
    mv /usr/lib/libia_cipf.so.0.0.0 /usr/lib/${IPU6_VER}/libia_cipf.so.0.0.0
    
    mv /usr/lib/libcamhal.so /usr/lib/${IPU6_VER}/libcamhal.so
    mv /usr/share/defaults/etc/camera /usr/share/defaults/etc/${IPU6_VER}/
    ```
    
4. Copy ipu6-camera-hal/config/linux/rules.d/*.rules to /lib/udev/rules.d/:
    ```shell
    cp ../config/linux/rules.d/*.rules /lib/udev/rules.d/
    ```

5. Reboot to make udev rules take effect.
## Tagged for removal:
The folder icamerasrc contains a gst plugin. This has been moved to https://github.com/intel/icamerasrc and will eventually be removed.
