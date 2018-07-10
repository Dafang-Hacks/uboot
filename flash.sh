md5sum u-boot-with-spl.bin
flash_eraseall /dev/mtd0
dd if=u-boot-with-spl.bin of=/dev/mtd0
dd if=/dev/mtd0 conv=sync,noerror 2> /dev/null | md5sum
