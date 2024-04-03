MISC_CHIP=sun8iw11p1
MISC_BOARD=a40i-p3
MISC_GNUEABI=gnueabihf
MISC_FBDEV=fbdev_hf
MISC_SDKLIB=libs_hf
MISC_CEDARXLIB=lib_hf
MISC_QTVER=5.9.0

LICHEE_TOOL=/home/qinzhencai/out/${MISC_CHIP}/linux/common/buildroot/host/opt/ext-toolchain/bin
export LICHEE_CHIP=${MISC_CHIP}
export FBDEV=${FBDEV}
export SDK_LIB=${SDK_LIB}
export CEDARX_LIB=${CEDARX_LIB}
export PATH=$PATH:$LICHEE_TOOL
/home/qinzhencai/buildroot-201611/dl/qt-everywhere-opensource-src-5.9.0/Qt-5.9.0/bin/qmake 
make -j32
