# use introduce

## build configure
ubuntu 21/22

## build ipu6 camerahal
1. mkdir build
2. cd build
3. cmake -DCMAKE_BUILD_TYPE=Release   -DIPU_VER=ipu6ep -DUSE_PG_LITE_PIPE=ON  -DCMAKE_INSTALL_PREFIX=/usr ../

## install libcamerahal-xxx.rpm

rpm -ivh --nodeps --force libcamerahal-xxx.rpm
