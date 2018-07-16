#!/usr/bin/env bash
TOOLCHAIN=$(pwd)/../toolchain/bin
export PATH=$TOOLCHAIN:$PATH

make distclean
make isvp_t20_sfcnor_ddr128M_config
make
cp u-boot-with-spl.bin opensource-T20-V1.3-128MB.bin