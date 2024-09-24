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
    export INSTALL_DIR=${PWD}/out
    export BOARD_LIST="ipu_tgl ipu_adl ipu_mtl"
    export BUILD_RPM=ON

    while true; do
        case $1 in
        -s|--source)
            export SOURCE_DIR=$2
            echo "$2"
            shift
            ;;
        -i|--install)
            export INSTALL_DIR=$2
            echo "$2"
            shift
            ;;
        --no-rpm)
            export BUILD_RPM=OFF
            ;;
        --board)
            shift
            export BOARD_LIST=$@
            echo "$$"
            break
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
    echo "INSTALL_DIR:               $INSTALL_DIR"
    echo "BUILD_RPM:                 $BUILD_RPM"
    echo "BOARD_LIST:                $BOARD_LIST"
    echo "====================================================================="
}

function print_helper() {
    echo
    echo "========================= Command Helper ============================"
    echo "./build_release.sh -s source_dir -i install_dir --board target1 targert2 ..."
    echo "====================================================================="
    echo
}

standardize_pkg_config_path(){
    if [[ ! -f $1 ]]; then echo "file $1 not exsist."; exit 0; fi
    sed -i '1c prefix=\/usr' $*
}

function build_target() {
    rm -fr build && mkdir -p build && cd build

    local target=$1
    export IPU_VERSION=ipu6ep

    # indicate the install folder of binary package
    # export PKG_CONFIG_PATH=~/work/linux/camera_submit/camera/out/install/lib/$target/pkgconfig:$PKG_CONFIG_PATH
    export PKG_CONFIG_PATH=/usr/lib/$target/pkgconfig:$PKG_CONFIG_PATH

    command cmake -DCMAKE_BUILD_TYPE=Release \
                  -DIPU_VER=$IPU_VERSION \
                  -DBUILD_CAMHAL_TESTS=OFF   \
                  -DUSE_PG_LITE_PIPE=ON \
		  -DBUILD_CAMHAL_ADAPTOR=ON   \
                  -DBUILD_CAMHAL_PLUGIN=ON   \
                  -DCMAKE_INSTALL_PREFIX=/usr   \

    # make and install
    make -j`nproc`
    check_result $? $FUNCNAME

    make install
    check_result $? $FUNCNAME
    standardize_pkg_config_path ${INSTALL_DIR}/install/usr/lib/pkgconfig/libcamhal.pc
    cd ..
}

function build_hal() {
    cd $SOURCE_DIR/ipu6-camera-hal

    for target in $BOARD_LIST
    do
        build_target $target
    done
}

function build_hal_adaptor() {
    cd $SOURCE_DIR/ipu6-camera-hal/src/hal/hal_adaptor
    rm -fr build && mkdir -p build && cd build

    command cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR/install ../

    # make and install
    make -j
    check_result $? $FUNCNAME

    make install
    check_result $? $FUNCNAME
    standardize_pkg_config_path ${INSTALL_DIR}/install/lib/pkgconfig/hal_adaptor.pc
}

function build_icamerasrc() {
    cd $SOURCE_DIR/icamerasrc/

    export CAMHAL_LIBS="-L$INSTALL_DIR/install/lib -lhal_adaptor"
    export CAMHAL_CFLAGS="-I$INSTALL_DIR/install/include/hal_adaptor  \
                          -I$INSTALL_DIR/install/include/hal_adaptor/api \
                          -I$INSTALL_DIR/install/include/hal_adaptor/utils \
                          -I$INSTALL_DIR/install/include/hal_adaptor/linux"
    export CHROME_SLIM_CAMHAL=ON
    export PKG_CONFIG_PATH="/usr/lib/x86_64-linux-gnu/pkgconfig:/usr/lib/pkgconfig:${PKG_CONFIG_PATH}"

    rm -fr config.h.in autom4te.cache/ aclocal.m4 *-libtool config.guess compile \
           config.sub configure depcomp install-sh ltmain.sh m4
    autoreconf --install
    CFLAGS="-O2" CXXFLAGS="-O2" ./configure --with-haladaptor=yes ${CONFIGURE_FLAGS} \
                                            --prefix=$INSTALL_DIR/install DEFAULT_CAMERA=0
    check_result $? $FUNCNAME

    make clean
    make -j
    check_result $? $FUNCNAME

    make install
    check_result $? $FUNCNAME
    find $INSTALL_DIR/install/ -name "*.la" -exec rm -f "{}" \;
    standardize_pkg_config_path ${INSTALL_DIR}/install/lib/pkgconfig/libgsticamerasrc.pc
}

function main () {
    parse_argument $@
    if [ -z "$SOURCE_DIR" ] || [ -z "$INSTALL_DIR" ] || [ -z "$BOARD_LIST" ]; then
        print_helper
        exit 1
    fi
    rm -fr $INSTALL_DIR/install $INSTALL_DIR/rpm && mkdir -p $INSTALL_DIR/install $INSTALL_DIR/rpm
    build_hal
    build_hal_adaptor
    build_icamerasrc

    # TODO: Generate the RPM for hal, hal_adaptor and icamerasrc together
}

main $@
