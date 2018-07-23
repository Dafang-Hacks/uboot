#!/usr/bin/env bash
TOOLCHAIN=$(pwd)/../toolchain/bin
export PATH=$TOOLCHAIN:$PATH
make distclean
make xiaofang1s_64mb_config
make
cp u-boot-with-spl.bin compiled_bootloader/xiaofang1s_64mb_v2.bin