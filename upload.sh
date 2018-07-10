#!/usr/bin/env bash
HOST=192.168.0.99
ftp-upload -h ${HOST} -u root --password ismart12 -d /system/sdcard/ u-boot-with-spl.bin
ftp-upload -h ${HOST} -u root --password ismart12 -d /system/sdcard/ flash.sh
ftp-upload -h ${HOST} -u root --password ismart12 -d /system/sdcard/ uEnv.txt