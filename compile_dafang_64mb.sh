#!/usr/bin/env bash
TOOLCHAIN=$(pwd)/../toolchain/bin
export PATH=$TOOLCHAIN:$PATH

make distclean
make dafang_64mb_config
make
cp u-boot-with-spl.bin dafang_64mb_v1.bin