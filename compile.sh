#!/usr/bin/env bash
TOOLCHAIN=$(pwd)/../toolchain/bin
export PATH=$TOOLCHAIN:$PATH
HOST=192.168.0.99
make distclean
make isvp_t20_sfcnor_ddr128M_config
make

ftp-upload -h ${HOST} -u root --password ismart12 -d /system/sdcard/ u-boot-with-spl.bin
ftp-upload -h ${HOST} -u root --password ismart12 -d /system/sdcard/ flash.sh