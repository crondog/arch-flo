#!/bin/bash

#KERNEL=~/android/system/out/target/product/flo/kernel
KERNEL=~/android/kernel_msm/arch/arm/boot/zImage
INITRAMFS=minimal.initramfs

~/android/system/out/host/linux-x86/bin/mkbootimg --kernel $KERNEL --cmdline "console=ttyHSL0,115200,n8 g_serial.n_ports=2 console=tty1 fbcon=rotate:1 androidboot.hardware=flo user_debug=31 msm_rtb.filter=0x3F ehci-hcd.park=3 selinux=0 fsck.mode=skip" --base 0x80200000 --pagesize 2048 --ramdisk_offset 0x02000000 --ramdisk $INITRAMFS --output newboot
