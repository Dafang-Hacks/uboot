#!/usr/bin/env bash
TOOLCHAIN=$(pwd)/../toolchain/bin
export PATH=$TOOLCHAIN:$PATH

make distclean
make isvp_t20_msc0_ddr128M
make
make env
