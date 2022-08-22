# Use guide

## build environment
ubuntu 21/22

## build libcamerahal-xxx.rpm
1. cd ipu6-camera-hal
2. mkdir build & cd build
3. cmake -DCMAKE_BUILD_TYPE=Release -DIPU_VER=ipu6ep\ 
         -DUSE_PG_LITE_PIPE=ON -DCMAKE_INSTALL_PREFIX=/usr ../
4. make -j8
5. make package

## use libcamerahal-xxx.rpm
rpm -ivh --force --nodeps libcamerahal-xxx.rpm
