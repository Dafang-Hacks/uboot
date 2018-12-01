#!/usr/bin/env bash
HOST=192.168.0.39
ftp-upload -h ${HOST} -u root --password ismart12 -d /tmp/ u-boot-with-spl.bin
ftp-upload -h ${HOST} -u root --password ismart12 -d /tmp/ flash.sh
#ftp-upload -h ${HOST} -u root --password ismart12 -d /system/sdcard/ uEnv.txt