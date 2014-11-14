# Arch Linux on Nexus 7 2013 Flo (Native)

Here are my instructions on getting Arch Linux ARM natively booting on the
Nexus 7 2013 Flo. Ideally this would be booting from a usb stick or a dedicated
partition on the mmc but this is working for me now.

If you cannot get it booting you will need to
reboot into recovery and check /proc/last_kmsg for the errors. Or try and build
the UART cable (which I failed at)

### What's working

- Wi-Fi
- Bluetooth
- ADB
- USB Reverse Tethering/RNDIS (guest)
- USB OTG (host)
- Framebuffer
- Audio - you need to setup PulseAudio as systemwide. Not sure why it does not work as a normal user. You will also need the alsaucm files from libasound2-data_1.0.27.2-1ubuntu6_all.deb

### Bugs/What's not working

- There is no frambuffer console
- The Software Refresher in the kernel does not work. You will need to run my
refresher app or fix the kernel :)
- The new xa code from x86-video-freedreno-git does not work. gpuaddr in
kgsl_bo_gpuaddr comes back a 0
- Hardware acceleration. Needs LibHybris, we're working on it.

### Not tested

- NFC (if you know how to test it please let @Davideddu know, I have a bunch of NFC tags to test it with. You can tag me in an issue).

# Setup guides
## Basic setup and booting with fastboot

1. Follow the instuctions at https://github.com/borh/nexus-7-2013-arch-scripts to get a working chroot.
1. Remove trimslice packages from chroot.
1. Edit the init script (systemd-initramfs/init) to point to your arch.img.
1. Build the kernel (instructions below).
1. Build the initramfs (instructions below).
1. Edit makebootimage.sh to point to the new kernel and to the initramfs you just built and run it to create an Android boot image.
1. Use ```fastboot boot <image>``` to boot the image you just created.
1. Enjoy ;)

## Booting with MultiROM (using an image)

You will need to follow the guide at http://forum.xda-developers.com/showthread.php?t=2457063 to install MultiROM on your device.

Also follow the basic guide above to create the chroot, then follow the following steps.

The init script needs to be modified to support this. You currently can't boot the same boot image via both fastboot and MultiROM.

1. Restart adbd as root using ```adb root```; if your ROM doesn't support it, reboot to recovery.
1. Create a ROM folder for Arch Linux
    
    ```adb shell mkdir /data/media/0/multirom/roms/<rom name>```
3. Push the provided ```multirom/rom_info.txt``` file. You might want to edit it, it's a plaintext config file documented at https://github.com/Tasssadar/multirom/wiki/Add-support-for-new-ROM-type
    
    ```adb push multirom/rom_info.txt /data/media/0/multirom/roms/<rom name>/```
4. Move your root image to ```/data/media/0/multirom/roms/<rom name>/root.img````
5. Edit the init script (systemd-initramfs/init) to point to your new root.img location, then rebuild the initramfs (you don't need to build the boot image for MultiROM).
6. Make sure you have the kernel zImage in ```/data/media/0/multirom/roms/<rom name>/boot/vmlinuz``` and the initramfs in ```/data/media/0/multirom/roms/<rom name>/boot/initrd.img```
7. Mount the image, and copy these files (with the same names) to /boot/ into it.

## Booting with MultiROM (directly from /data, no image)
It **is** possible to run the chroot without using an image, and it's even possible to boot it.

The non-image method has, however, some disadvantages over the image:

- While using the chroot through Android, some security features such as SELinux (enforces low-level security features), *nosuid* (prevents using sudo), *noexec* (prevents running programs) and *nodev* (prevents devices from being into some mountpoint) need to be disabled on the entire /data partition where the chroot is stored. They are re-enabled on exit, but while it's running they need to be off. I don't think you should worry about this, I just want to make sure you're aware of this behavior.
- There is a bug in my patched chroot script which causes **all** the processes in the chroot to die when **any** of the chroots exits. This happens because the process-killing code in the original script doesn't work outside of an image. I had to replace it with a slower implementation that looks for processes that come from the chroot in /proc and kills them one by one. It's not smart enough to know you're using bash in *that other chroot instance* and that you might need it. To avoid this bug you just have to brutally exit from the chroot (e.g. you close the terminal window); this way the tear-down code will not be executed and the other chroots will keep running.
- You can't use ```df``` to know how much disk space is being taken by the chroot, because this isn't a separate filesystem. You either have to recursively use ```du``` or stay in the mistery.

Now that you've been warned, you can start installing your chroot.
If you previously followed the image chroot instructions, make sure you delete the old chroot or that you use a different ROM for this chroot.

1. Download and use my [patched chroot installation scripts](https://github.com/Davideddu/nexus-7-2013-arch-scripts). Before running them, make sure you edit **both** ```install-arch.sh``` and ```chroot.sh``` and set the ROM name.
1. Remove trimslice packages from chroot.
1. Replace the init script ```systemd-initramfs/sbin/init``` with my patched one from ```multirom/init.noimageboot``` and edit it; make sure the ROM name is correct.
1. Build the initramfs (instructions at the bottom)
1. Restart adbd as root using ```adb root``` to make it easier to push files; if your ROM doesn't support it, reboot to recovery.
2. Make sure ```/data/arch``` exists:
    
    ```mkdir -p /data/arch```
6. Make sure you have the kernel zImage in ```/data/media/0/multirom/roms/<rom name>/boot/vmlinuz``` and the initramfs in ```/data/media/0/multirom/roms/<rom name>/boot/initrd.img```
7. Also copy these files (with the same names) to /boot/ into the chroot (```/data/media/0/multirom/roms/<rom name>/root/boot/```)
3. Push the provided ```multirom/rom_info.txt``` file. You might want to edit it, it's a plaintext config file documented at https://github.com/Tasssadar/multirom/wiki/Add-support-for-new-ROM-type. Note that this file doesn't need to be patched, it's the same regardless of how you installed the chroot.
    
    ```adb push multirom/rom_info.txt /data/media/0/multirom/roms/<rom name>/```

Your chroot should now be fully working and bootable, congratulations! ;)

You should be able to adapt these instructions to boot Arch using fastboot. While it should work out of the box if you simply build the Android boot image and boot it (not tested), you will have to edit ```install-arch.sh```, ```chroot.sh``` and the init script if you want to change the installation directory. Always make sure you have an empty mountpoint in /data/, e.g. ```/data/arch```, otherwise you'll get a kernel panic on boot as ```switch_root``` works only with mountpoints.

## Booting from USB with MultiROM (not tested)
You basically have to follow the MultiROM image booting steps, but instead of placing the files into /data/media/0/multirom you will want to put them into a ```multirom``` directory inside of your fat32-formatted USB drive. You will however have to edit the init script, find the USB drive's device (it should be /dev/block/sda1 but don't count on that), mount it and pass the torch to systemd's init.

This hasn't been tested yet. Detailed instructions will come as soon as I test it.

## Getting Wi-Fi working

A normal Nexus 7 Flo boot will call ```/system/bin/conn_init```, which does a whole lot of verifying your MAC address and then writes out a config with the MAC address in it. Very annoying (especially if you want to spoof it). To get around this you need a copy of the Wi-Fi configs and manually enable the wifi driver since it's not loaded on boot.

The firmware.service is only needed since the firmware is not loaded in the initramfs.

1. Copy ```/system/vendor/firmware``` (android) to ```/usr/lib/firmware``` (arch) (you might want to mount system in the chroot or copy the files to /sdcard, then access them from /media/sdcard in the chroot).
1. ```rm /usr/lib/firmware/wlan/prima/{WCNSS_qcom_cfg.ini,WCNSS_qcom_wlan_nv.bin}``` ← these are simlinks
1. ```cp /data/misc/wifi/{WCNSS_qcom_cfg.ini,WCNSS_qcom_wlan_nv.bin} /usr/lib/firmware/wlan/prima/``` ← these are the files which conn_init writes your mac address to - again, use /sdcard or mount /data into the chroot.
1. Copy the provided ```firmware.service``` to ```/etc/systemd/system/```
1. Copy the provided ```firmware.sh``` to ```/usr/local/sbin/```
1. Copy the provided ```50-firmware.rules``` to ```/etc/udev/rules.d/```
1. ```ln -s /etc/systemd/system/firmware.service /etc/systemd/system/multi-user.target.wants/firmware.service```
1. ```ln -s /usr/lib/systemd/system/wpa_supplicant-nl80211@wlan0.service /etc/systemd/system/multi-user.target.wants/wpa_supplicant-nl80211@wlan0.service```
1. ```ln -s /lib/systemd/system/dhcpcd.service /etc/systemd/system/multi-user.target.wants/dhcpcd.service```
1. ```ln -s /lib/systemd/system/sshd.service /etc/systemd/system/multi-user.target.wants/sshd.service```
1. Disable the wpa_supplicant hook in ```/etc/dhcpcd.conf``` by adding ```nohook wpa_supplicant``` at the end.

### Setting up shared Wi-Fi configuration between Arch and Android

1. Make sure /data is mounted at boot:
    
    ```
    mkdir /data
    echo '/dev/disk/by-partlabel/userdata /data ext4 errors=remount-ro 0 0' >> /etc/fstab
    mount /data
    ```
2. Forcibly symlink the Android wpa_supplicant.conf to the Linux location.
    
    ```
    ln -fs /data/misc/wifi/wpa_supplicant.conf /etc/wpa_supplicant/wpa_supplicant-nl80211-wlan0.conf
    ```
1. Create a name for the Android wifi group
    
    ```
    groupadd -g $(stat -c '%g' /data/misc/wifi) -r android_wifi
    ```
1. Add your main user to said group, so you can access the control sockets. You will need to manually their path if you want to edit the Wi-Fi network configuration.
    
    ```
	usermod -aG android_wifi *\<your username\>*
    ```

## Getting fbterm console

1. Copy provided ```fbterm/getty@.service``` to ```/etc/systemd/system/```
1. Copy ```fbterm/fbterm-login``` to ```/opt/fbterm-login``` and make it executable
1. ```systemctl enable getty@tty1.service```

```ln -s getty@.service autovt@.service``` to get it on all vt's

## Getting Xorg

Install [xf86-video-freedreno-git](https://aur.archlinux.org/packages/xf86-video-freedreno-git) (AUR). Use the xorg.conf included (xorg directory).

Instead of filling the AUR with crap you can use the mesa and libdrm PKGBUILDs included in the ```xorg``` directory. Always use the git versions as they have the latest freedreno fixes.

## Refreshing (refresher.c)

Since the ```MSMFB_SW_REFRESHER``` does not seem to be working, I wrote this instead.
Just compile it inside of the chroot (do not crosscompile it on your computer), copy it to ```/bin/refresher``` and make it executable.

You can use the included systemd unit file for the refresher:
Copy the provided ```systemd/refresher.service``` to ```/etc/systemd/system/refresher.service```.

### Step-by-step
1. Push ```refresher.c``` to your tablet. Actually, if I were you, I'd probably clone the entire ```arch-flo``` repo to the tablet. Android works, don't bother booting Arch yet if you don't want to.
1. Make sure ```gcc``` is installed.
1. ```$ gcc -o refresher refresher.c```
1. ```# cp refresher /bin/refresher```
1. ```# chmod +x /bin/refresher```
1. ```# cp path-to/arch-flo/systemd/refresher.service /etc/systemd/system/refresher.service```
1. ```# systemctl enable refresher.service```

From now on you'll be able to use xinit like you would on your computer, without doing weird stuff.

## Serial console

USB serial works. I had some problems connecting with the Android Composite Gadget, however just using the ```USB_G_SERIAL``` kernel config seems to work (it's enabled by default in the patched kernel).

To enable it, run

```
systemctl enable serial-getty@ttyGS0.service
```

and then you will be able to connect to the device via minicom or similar on /dev/ttyACM0.

Getting kernel console boot messages is still a no go as the device is registered too late to work.

## Enabling ADB and RNDIS (reverse USB tethering)

1. Create this link for ADB to be able to find the shell, etc.
    ```
    ln -s / /system
	```
1. Add the included systemd-tmpfiles config files. They go in  ```/etc/tmpfiles.d```, and require a reboot to apply them.

ADB shell works, but you will need to set the size ($COLUMNS and $LINES) and terminal type ($TERM) manually. 

To use RNDIS, plug the Nexus into a computer and bridge the virtual ethernet adapter with your local network. Then, bring up the link and get an address as usual. The interface name should be 'usb0'.

# Kernel

You can use the kernel from here: https://github.com/crondog/kernel_msm. Use the **mr1** branch.

## Building the kernel

A guide to build kernels for Nexus devices can be obtained from Google: http://source.android.com/source/building-kernels.html

Just make sure you use the patched fork provided above instead of the default repository, or that you apply the patches below, otherwise you will likely have issues.

## Getting touchscreen working

Apply provided ```0001-make-ektf3k-driver-report-non-MT-events-too.patch```.

## Kernel configuration
I used the following additional configs to get this working. Some might not be
necessary but it helps with debugging

*Required for display/graphics:*

- CONFIG_FB_MSM_DEFAULT_DEPTH_BGRA8888 (with RGBA patch from robclark)
- CONFIG_DRM=y
- CONFIG_MSM_KGSL_DRM=y

*Required for Wi-Fi/network:*

- CONFIG_ANDROID_PARANOID_NETWORK=n
- CONFIG_MODULES=y
- CONFIG_PRIMA_WLAN=y
- CONFIG_WCNSS_CORE=y

*Required for USB serial:*

- CONFIG_USB_G_SERIAL=y

*Required to boot:*

- CONFIG_FHANDLE=y
- CONFIG_DEVTMPFS=y
- CONFIG_UTS_NS=y
- CONFIG_IPC_NS=y
- CONFIG_USER_NS=y
- CONFIG_PID_NS=y
- CONFIG_NET_NS=y
- CONFIG_DEVPTS_MULTIPLE_INSTANCES=y
- CONFIG_FANOTIFY=y
- CONFIG_FANOTIFY_ACCESS_PERMISSIONS=y
- CONFIG_SECURITY_SELINUX_BOOTPARAM=y

*Debugging:*

- CONFIG_DETECT_HUNG_TASK=y
- CONFIG_DEBUG_SPINLOCK=y
- CONFIG_DEBUG_MUTEXES=y
- CONFIG_DEBUG_ATOMIC_SLEEP=y
- CONFIG_STACKTRACE=y
- CONFIG_DEBUG_BUGVERBOSE=y
- CONFIG_LOG_BUF_SHIFT=21
- CONFIG_PANIC_TIMEOUT=0

If you use the kernel at https://github.com/crondog/kernel_msm (mr1 branch), there is a default configuration named flo_defconfig which contains most of the needed options.

# Building the initramfs
## Manually

Make sure *abootimg* is installed.

1. Edit makebootimage.sh and correct paths
1. *cd systemd-initramfs; find . | cpio -o -H newc | gzip > ../minimal.initramfs*
1. *./makebootimage.sh*


## Using mkinitcpio
1. Copy ```zImage``` to ```/boot```
1. Copy modules to ```/lib/modules/`uname -r````
1. Apply ```mkinitcpio.patch```
1. Copy ```hooks/imgmount``` and ```install/imgmount``` to ```/lib/initcpio```
1. ```sudo mkinitcpio -p linux```
1. ```abootimg --create boot.img -f bootimg.cfg -k /boot/zImage -r /boot/initramfs-linux.img```

