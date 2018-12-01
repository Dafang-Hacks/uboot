#!/usr/bin/env bash
TOOLCHAIN=$(pwd)/../toolchain/bin
export PATH=$TOOLCHAIN:$PATH

make distclean
make dafang_128mb_config
make
cp u-boot-with-spl.bin compiled_bootloader/dafang_128mb_v2.bin