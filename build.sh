#!/bin/bash

function check_result() {
    local res=$1
    local func=$2

    if [ $res -eq 0 ] ; then
        echo "###############" "  $func  OK  " "#############"
    else
        echo "###############" "  $func  FAIL  " "#############"
        exit 1
    fi
}

function parse_argument() {
    export SOURCE_DIR=${PWD}
    export TARGET_BUILD_INSTALL_DIR=${PWD}/out
    export BOARD_LIST="ipu_tgl ipu_adl ipu_mtl"
    export BUILD_RPM=OFF
    export ENABLE_DMA_DRM=OFF

    while true; do
        case $1 in
        --no-rpm)
            export BUILD_RPM=OFF
            ;;
        -d|--dma)
            export ENABLE_DMA_DRM=ON
            ;;
        --board)
            export BOARD_LIST=$2
            echo "$2"
            ;;
        -?*)
            echo "WARNING: not supported option: $1"
            ;;
        *)
            break
        esac

        shift
    done

    echo "=========================== Build Config ============================"
    echo "SOURCE_DIR:                $SOURCE_DIR"
    echo "INSTALL_DIR:               $TARGET_BUILD_INSTALL_DIR"
    echo "BUILD_RPM:                 $BUILD_RPM"
    echo "ENABLE_DMA_DRM:            $ENABLE_DMA_DRM"
    echo "BOARD_LIST:                $BOARD_LIST"
    echo "====================================================================="
}

function print_helper() {
    echo
    echo "========================= Command Helper ============================"
    echo "==== disable dma ===="
    echo "./build.sh --board target"
    echo "==== enable dma ===="
    echo "./build.sh -d --board target"
    echo "====================================================================="
    echo
}

function build_libcamhal() {
    rm -fr build && mkdir -p build && cd build

    local target=$1

    rm -fr $TARGET_BUILD_INSTALL_DIR/$target && mkdir -p $TARGET_BUILD_INSTALL_DIR/$target
    export BUILD_INSTALL_DIR=$TARGET_BUILD_INSTALL_DIR/$target/install

    export IPU_VERSIONS=
    if [ "$target" = "ipu_tgl" ]; then
        IPU_VERSIONS=ipu6
    elif [ "$target" = "ipu_adl" ]; then
        IPU_VERSIONS=ipu6ep
    elif [ "$target" = "ipu_mtl" ]; then
        IPU_VERSIONS=ipu6epmtl
    else
        echo "Error: doesn't support the target : $target"
        exit -1
    fi

    # indicate the install folder of binary package
    sed -i 's|^prefix=.*|prefix='$BUILD_INSTALL_DIR/usr/'|' $BUILD_INSTALL_DIR/usr/lib/pkgconfig/*.pc

    command cmake -DCMAKE_BUILD_TYPE=Release \
                  -DIPU_VERSIONS=$IPU_VERSIONS \
                  -DBUILD_CAMHAL_TESTS=ON \
                  -DUSE_PG_LITE_PIPE=ON \
                  -DUSE_STATIC_GRAPH=ON   \
                  -DUSE_STATIC_GRAPH_AUTOGEN=ON   \
                  -DCMAKE_C_FLAGS=-O2 \
                  -DCMAKE_CXX_FLAGS=-O2 \
                  -DBUILD_CAMHAL_ADAPTOR=ON \
                  -DBUILD_CAMHAL_PLUGIN=ON \
                  -DCMAKE_INSTALL_PREFIX=/usr \
                  -DCMAKE_INSTALL_LIBDIR=lib \
                  ..

    # make and install
    make -j`nproc`
    check_result $? "$FUNCNAME: $target"

    make DESTDIR=$BUILD_INSTALL_DIR install
    check_result $? "$FUNCNAME: $target"

    cd ..
}

function build_libcamhal_target() {
    cd $SOURCE_DIR/ipu6-camera-hal

    for target in $BOARD_LIST
    do
        build_libcamhal $target
    done
}

function build_icamerasrc() {
    cd $SOURCE_DIR/icamerasrc/

    export DEFAULT_PKG_CONFIG_PATH="$PKG_CONFIG_PATH"
    export PKG_CONFIG_PATH=$DEFAULT_PKG_CONFIG_PATH:/usr/lib/x86_64-linux-gnu/pkgconfig:$BUILD_INSTALL_DIR/usr/lib/pkgconfig

    sed -i 's|^prefix=.*|prefix='$BUILD_INSTALL_DIR/usr/'|' $BUILD_INSTALL_DIR/usr/lib/pkgconfig/libcamhal.pc

    rm -fr config.h.in autom4te.cache/ aclocal.m4 *-libtool config.guess compile \
           config.sub configure depcomp install-sh ltmain.sh m4
    autoreconf --install

    export CAMHAL_CFLAGS="-I$BUILD_INSTALL_DIR/usr/include/libcamhal  \
                          -I$BUILD_INSTALL_DIR/usr/include/libcamhal/api \
                          -I$BUILD_INSTALL_DIR/usr/include/libcamhal/utils \
                          -I$BUILD_INSTALL_DIR/usr/include/libcamhal/linux"

    export CHROME_SLIM_CAMHAL=ON

    local ENABLE_FEATURE_ARGS=
    if [ "$ENABLE_DMA_DRM" = "ON" ]; then
            ENABLE_FEATURE_ARGS+="--enable-gstdrmformat=yes "
    fi

    CFLAGS="-O2" CXXFLAGS="-O2" ./configure --prefix=/usr DEFAULT_CAMERA=0 ${ENABLE_FEATURE_ARGS}
    check_result $? $FUNCNAME

    make clean
    make -j
    check_result $? $FUNCNAME

    make DESTDIR=$BUILD_INSTALL_DIR install
    check_result $? $FUNCNAME

    find $BUILD_INSTALL_DIR -name "*.la" -exec rm -f "{}" \;
}

function main () {
    parse_argument $@
    if [ -z "$SOURCE_DIR" ] || [ -z "$TARGET_BUILD_INSTALL_DIR" ] || [ -z "$BOARD_LIST" ]; then
        print_helper
        exit 1
    fi

    rm -fr $TARGET_BUILD_INSTALL_DIR && mkdir -p $TARGET_BUILD_INSTALL_DIR

    build_libcamhal_target
    build_icamerasrc

    # TODO: Generate the RPM for hal, hal_adaptor and icamerasrc together
}

main $@
