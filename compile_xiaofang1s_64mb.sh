#!/usr/bin/env bash
TOOLCHAIN=$(pwd)/../toolchain/bin
export PATH=$TOOLCHAIN:$PATH

make distclean
make dafang_64mb_config
make
cp u-boot-with-spl.bin xiaofang1s_64mb.bin