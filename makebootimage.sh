#!/bin/bash

KERNEL=~/android/system/out/target/product/flo/kernel
INITRAMFS=minimal.initramfs

~/android/system/out/host/linux-x86/bin/mkbootimg --kernel $KERNEL --cmdline "console=ttyHSL0,115200,n8 console=tty1 androidboot.hardware=flo user_debug=31 msm_rtb.filter=0x3F ehci-hcd.park=3 selinux=0" --base 0x80200000 --pagesize 2048 --ramdisk_offset 0x02000000 --ramdisk $INITRAMFS --output newboot
