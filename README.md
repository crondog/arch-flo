# Arch Linux on Nexus 7 Flo Native

Here are my instructions on getting Arch Linux ARM natively booting on the
Nexus 7 Flo. Ideally this would be booting from a usb stick or a dedicated
partition on the mmc but this is working for me now. Once I get a bluetooth
keyboard I will try to get it working from a usb stick.

## BUGS!!!
There is no frambuffer console :( 
If you cannot get it booting you will need to
reboot into recovery and check /proc/last_kmsg for the errors. Or try and build
the UART cable (which I failed at)

There is only xf86-video-fbdev working and no DRM or anything fancy

The Software Refresher in the kernel does not work. You will need to run my
refresher app or fix the kernel :)

The new xa code from x86-video-freedreno-git does not work. gpuaddr in
kgsl_bo_gpuaddr comes back a 0

## Quick Guide
1: I roughly followed the instructions form
https://github.com/borh/nexus-7-2013-arch-scripts to get a working chroot
first.

2: Edit the init script to point to your arch.img or partition

3: Edit and Run makebootimage.sh

4: fastboot boot newboot

5: ???

6: Profit

## To get WiFi Working

A normal Nexus 7 Flo boot will call /system/bin/conn_init which does a whole
lot of verifying your mac address and then writes out a config with the mac
address in it. Very annoying. To get around this you need a copy of the wifi
configs and manually enable the wifi driver since it does not start on boot.
The firmware.service is only needed since I do not load the firmware in the
initramfs (which I am haven't done yet)

1: Copy /system/vendor/firmware [android] to /usr/lib/firmware [arch]
2: rm /usr/lib/firmware/wlan/prima/{WCNSS_qcom_cfg.ini,WCNSS_qcom_wlan_nv.bin} <-- these are simlinks
3: cp /data/misc/wifi/{WCNSS_qcom_cfg.ini,WCNSS_qcom_wlan_nv.bin} /usr/lib/firmware/wlan/prima/ <-- these are the files which conn_init writes your mac address to

4: Copy firmware.service to /etc/systemd/system/ 
5: ln -s /etc/systemd/system/firmware.service /etc/systemd/system/multi-user.target.wants/firmware.service

6: Create a wpa_supplicant-nl80211@wlan0.service with your wpa_supplicant.conf
details
7: ln -s /etc/systemd/system/dhcpcd.service /etc/systemd/system/multi-user.target.wants/dhcpcd.service
8: ln -s /etc/systemd/system/sshd.service /etc/systemd/system/multi-user.target.wants/sshd.service

NOTE: The wifi drivers are really flakey. You might have better luck with a
static ip. If anything fails reboot into recovery and check /proc/last_kmsg

## To get X working
Since there is no framebuffer console you cannot start X manually (unless you
are able to get ssh working, see above) Auto login is your friend.

Be sure to install a window manager or something first and then follow
https://wiki.archlinux.org/index.php/Automatic_login_to_virtual_console and 
https://wiki.archlinux.org/index.php/Start_X_at_Login

Your .xinitrc should contain something like
~/refresher &
exec awesome

## Refreshing (refresher.c)
Since the MSMFB_SW_REFRESHER does not seem to be working i wrote this instead.
Just compile and put in you ~/ or whereever so xinit can run it.

# Kernel stuff

## To get Touchscreen working
Apply 0001-make-ektf3k-driver-report-non-MT-events-too.patch

## Kernel configs
I used the following additional configs to get this working. Some might not be
necessary but it helps with debugging

I havent figured out how to get fbdev working with RGBA8888 so use RGB565 for
now

CONFIG_FB_MSM_DEFAULT_DEPTH_RGB565=y
CONFIG_FHANDLE=y
CONFIG_DEVTMPFS=y
CONFIG_UTS_NS=y
CONFIG_IPC_NS=y
CONFIG_USER_NS=y
CONFIG_PID_NS=y
CONFIG_NET_NS=y
CONFIG_DEVPTS_MULTIPLE_INSTANCES=y
CONFIG_FANOTIFY=y
CONFIG_FANOTIFY_ACCESS_PERMISSIONS=y
CONFIG_DETECT_HUNG_TASK=y
CONFIG_DEBUG_SPINLOCK=y
CONFIG_DEBUG_MUTEXES=y
CONFIG_DEBUG_ATOMIC_SLEEP=y
CONFIG_STACKTRACE=y
CONFIG_DEBUG_BUGVERBOSE=y
CONFIG_SECURITY_SELINUX_BOOTPARAM=y

## Build initramfs
1: Edit makebootimage.sh with paths
2: cd systemd-initramfs; find . | cpio -o -H newc | gzip > ../minimal.initramfs
3: ./makebootimage.sh

# Whats Working
Wifi
fb0
Audio -- Need to setup Pulse Audio as systemwide. Not sure why it does not work as a normal user
You will also need the alsaucm files from libasound2-data_1.0.27.2-1ubuntu6_all.deb

# Whats Maybe
Bluetooth - I can pair with my i9300 with bluez4 but obexftp doesnt work...Havent tried a keyboard yet

# Not tested
NFC
