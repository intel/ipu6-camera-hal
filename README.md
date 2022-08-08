##  Build environment:
      ubuntu21/22<br>

##  Build steps:
  a. Copy ipu6 binary to build environment.<br>
  b. Build ipu6-camera-hal to generate "libcamhal-xxx.rpm".<br>
  c. Build icamerasrc to generate "icamerasrc-xxx.rpm".<br>



- ##build steps:<br>
1.mkdir build<br>
2.cd build<br>
3.cmake -DCMAKE_BUILD_TYPE=release -DIPU_VER=ipu6ep -DUSE_PG_LITE_PIPE=ON ../<br>
4.make -j8
5.make package

- ### install libcamerahal-xxx.rpm
- rpm -ivh --nodeps --force libcamerahal-xxx.rpm
