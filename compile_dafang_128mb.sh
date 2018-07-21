#!/usr/bin/env bash
TOOLCHAIN=$(pwd)/../toolchain/bin
export PATH=$TOOLCHAIN:$PATH

make distclean
make dafang_128mb_config
make
cp u-boot-with-spl.bin dafang_128mb_v1.bin