#  User Guide
##  1. Build environment<br>
      ubuntu21/22 <br>
      sudo apt-get install autoconf libtool linux-libc-dev<br>

##  2. Building libcamhal<br>
        mkdir build<br>
        cd build<br>
        cmake -DCMAKE_BUILD_TYPE=Release   -DIPU_VER=ipu6 -DUSE_PG_LITE_PIPE=ON ../<br>
        make -j<br>
        make package<br>

##  3.  Install libcamhal<br>
      rpm -ivh --force --nodeps libcamhal-xxx.rpm 
