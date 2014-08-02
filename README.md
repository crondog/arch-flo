# Arch Linux on Nexus 7 Flo Native

Here are my instructions on getting Arch Linux ARM natively booting on the
Nexus 7 Flo. Ideally this would be booting from a usb stick or a dedicated
partition on the mmc but this is working for me now.

## BUGS!!!
There is no frambuffer console :( 
If you cannot get it booting you will need to
reboot into recovery and check /proc/last_kmsg for the errors. Or try and build
the UART cable (which I failed at)

The Software Refresher in the kernel does not work. You will need to run my
refresher app or fix the kernel :)

The new xa code from x86-video-freedreno-git does not work. gpuaddr in
kgsl_bo_gpuaddr comes back a 0

## Quick Guide
1: I roughly followed the instructions form
https://github.com/borh/nexus-7-2013-arch-scripts to get a working chroot
first.

1a: remove trimslice packages from chroot

2: Edit the init script to point to your arch.img or partition

3: Edit and Run makebootimage.sh

4: fastboot boot newboot

5: ???

6: Profit

## Booting with MultiROM

You will need to follow the guide at http://forum.xda-developers.com/showthread.php?t=2457063 first.

1: The initcpio hooks have been modified to support this, so if you were using the old ones, you have to replace them with the new. You currently can't boot the same installation via both fastboot and MultiROM. 

2: Boot the Nexus into Android recovery mode. Make sure /data is mounted. /data/media/0/multirom cannot be seen from normal Android.

3: Create a ROM folder for ArchLinuxARM.

  adb shell mkdir /data/media/0/multirom/roms/ArchLinuxARM

4: Push the included rom_info.txt file. (it's a plaintext config file documented at https://github.com/Tasssadar/multirom/wiki/Add-support-for-new-ROM-type)

  adb push rom_info.txt /data/media/0/multirom/roms/ArchLinuxARM/
  
5: Move your root image to /data/media/0/multirom/roms/ArchLinuxARM/root.img

6: Make sure you have the kernel and ramdisk in /data/media/0/multirom/roms/ArchLinuxARM/boot/vmlinuz and /data/media/0/multirom/roms/ArchLinuxARM/boot/initrd.img.

6a: Mount the image, and copy them (with the same name) to /boot/ in it.


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

6: ln -s /lib/systemd/system/wpa_supplicant-nl80211@wlan0.service /etc/systemd/system/multi-user.target.wants/wpa_supplicant-nl80211@wlan0.service
7: ln -s /lib/systemd/system/dhcpcd.service /etc/systemd/system/multi-user.target.wants/dhcpcd.service
8: ln -s /lib/systemd/system/sshd.service /etc/systemd/system/multi-user.target.wants/sshd.service

Note: Disable the wpa_supplicant hook in /etc/dhcpcd.conf

# Setting up shared Wi-Fi configuration between Arch and Android

1: Make sure /data is mounted at boot:
  mkdir /data
  echo '/dev/disk/by-partlabel/userdata /data ext4 errors=remount-ro 0 0' >> /etc/fstab
  mount /data

2: Forcibly symlink the Android wpa_supplicant.conf to the Linux location.
  ln -fs /data/misc/wifi/wpa_supplicant.conf /etc/wpa_supplicant/wpa_supplicant-nl80211-wlan0.conf
  
3: Create a name for the Android wifi group
  groupadd -g $(stat -c '%g' /data/misc/wifi) -r android_wifi
  
4: Add your main user to said group, so you can access the control sockets. You will need to manually their path if you want to edit the WiFi network configureation.

  ## To get fbterm console
Copy fbterm/getty@.service to /etc/systemd/system/ and fbterm/fbterm-login to
/opt/fbterm-login and then systemctl enable getty@tty1.service

ln -s getty@.service autovt@.service to get it on all vt's

## To get X working
Install xf86-video-freedreno-git (aur) Use the xorg.conf included

Instead of filling the AUR with crap you can use the mesa and libdrm PKGBUILDs
included. Always use the git versions as they have the latest freedreno fixes

Your .xinitrc should contain something like
~/refresher &
exec awesome

## Refreshing (refresher.c)
Since the MSMFB_SW_REFRESHER does not seem to be working i wrote this instead.
Just compile and put in you ~/ or whereever so xinit can run it.

You can also use the included systemd unit file for the refresher.

1: inside the initial chroot compile refresher.c and move to /bin/refresher
2: copy systemd/refresher.service to /etc/systemd/system/refresher.service

## Enabling ADB and RNDIS (reverse USB tethering)
1: Create this link for ADB to be able to find the shell, etc.
  ln -s / /system
  
2: Add the included systemd-tmpfiles config files. They go in /etc/tmpfiles.d, and require a reboot to apply them.

ADB shell works, but you will need to set the size ($COLUMNS and $LINES) and terminal type ($TERM) manually. 

To use RNDIS, plug the Nexus into a computer and bridge the virtual ethernet adapter with your local network. Then, bring up the link and get an address as usual. The interface name should be 'usb0'.

# Kernel stuff

You can use the kernel from here https://github.com/crondog/kernel_msm

## To get Touchscreen working
Apply 0001-make-ektf3k-driver-report-non-MT-events-too.patch

## Debugging
USB serial works. I had some problems connecting with the Android Composite
Gadget however just using USB_G_SERIAL seems to work. To enable run systemctl
enable serial-getty@ttyGS0.service and then you will be able to connect to the
device via minicom or similar on /dev/ttyACM0. Getting kernel console boot
messages is still a no go as the device is registered too late to work

## Kernel configs
I used the following additional configs to get this working. Some might not be
necessary but it helps with debugging

Required for display/graphics:

CONFIG_FB_MSM_DEFAULT_DEPTH_BGRA8888 (with RGBA patch from robclark)
CONFIG_DRM=y
CONFIG_MSM_KGSL_DRM=y

Required for Wi-Fi/network:

CONFIG_ANDROID_PARANOID_NETWORK=n
CONFIG_MODULES=y
CONFIG_PRIMA_WLAN=y
CONFIG_WCNSS_CORE=y

Required to boot:

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
CONFIG_SECURITY_SELINUX_BOOTPARAM=y

Debugging:

CONFIG_DETECT_HUNG_TASK=y
CONFIG_DEBUG_SPINLOCK=y
CONFIG_DEBUG_MUTEXES=y
CONFIG_DEBUG_ATOMIC_SLEEP=y
CONFIG_STACKTRACE=y
CONFIG_DEBUG_BUGVERBOSE=y
CONFIG_LOG_BUF_SHIFT=21
CONFIG_PANIC_TIMEOUT=0

If you use the kernel at https://github.com/crondog/kernel_msm, there is a default configuration named flo_defconfig which contains most of the needed options.

## Build initramfs (Manually)
1: Edit makebootimage.sh with paths
2: cd systemd-initramfs; find . | cpio -o -H newc | gzip > ../minimal.initramfs
3: ./makebootimage.sh


## Build initramfs (mkinitcpio)
1: Copy zImage to /boot/
2: Copy modules to /lib/modules/`uname -r`
3: Apply mkinitcpio.patch
4: Copy hooks/imgmount and install/imgmount to /lib/initcpio
5: sudo mkinitcpio -p linux
6: abootimg --create boot.img -f bootimg.cfg -k /boot/zImage -r /boot/initramfs-linux.img


# Whats Working
Wifi
ADB
USB Reverse Tethering/RNDIS (guest)
USB OTG (host)
fb0
Audio -- Need to setup Pulse Audio as systemwide. Not sure why it does not work as a normal user
You will also need the alsaucm files from libasound2-data_1.0.27.2-1ubuntu6_all.deb
Bluetooth - We now have bluez5 support :) Only tested with a Wedge keyboard

# Not tested
NFC
