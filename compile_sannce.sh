#!/usr/bin/env bash
TOOLCHAIN=$(pwd)/../toolchain/bin
export PATH=$TOOLCHAIN:$PATH
make distclean
make sannce_config
make
cp u-boot-with-spl.bin compiled_bootloader/sannce_v1.bin