## Bootloader for T20 Devices

## Scripts:
Compile_ *** => compiles a Version for a specific Device

## Build using a ubuntu container
1) Checkout the repo
2) `docker run -it -v d:<location>:/uboot ubuntu /bin/bash`
3) `apt-get update && apt-get upgrade && apt-get install -y build-essential make git`
4) `git clone https://github.com/Dafang-Hacks/mips-gcc472-glibc216-64bit.git toolchain`
5) `cd /uboot && ./Compile_*`