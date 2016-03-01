#!/bin/bash
echo "Made By KadjarT Edited by SR"
echo "#################"
echo "Choose the device"
echo "#################"
 choice=""
while [ "$choice" != "q" ]
do
echo "  kyleve  "
echo "  kylevess   " &&   echo -n ""
read choice

case $choice in

kyleve)
echo "#################"
echo "$choice selected"
echo "#################"
export PATH=/home/sr/cm12.2/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin:$PATH
export ARCH=arm
export SUBARCH=arm
export CROSS_COMPILE=arm-eabi-
mkdir -p kernel_out 
cp -f ./arch/arm/configs/bcm21664_hawaii_ss_kyleve_rev00_defconfig ./kernel_out/.config
make -j8 ARCH=arm KBUILD_OUTPUT=./kernel_out oldnoconfig
make -j8 ARCH=arm KBUILD_OUTPUT=./kernel_out
cp -f ./kernel_out/arch/arm/boot/zImage ./arch/arm/boot/
cp -rf ./kernel_out/arch/arm/boot/compressed/vmlinux ./arch/arm/boot/compressed/
cp -rf ./kernel_out/vmlinux ./
mkdir tmp
cd tmp
cp -f ../arch/arm/boot/zImage ./zImage
cp ../arch/arm/tools/boot.img-ramdisk.cpio.gz-kyleve ./boot.img-ramdisk.cpio.gz
cp ../arch/arm/tools/mkbootimg ./mkbootimg
cp ../arch/arm/tools/META-INF_kyleve.zip ./META-INF.zip
cp ../arch/arm/tools/system.zip ./system.zip
unzip system.zip
rm -rf system.zip
echo "Creating boot.img"
./mkbootimg --kernel zImage --ramdisk boot.img-ramdisk.cpio.gz --board kyleve --pagesize 4096 --base 0x82000000 -o boot.img
echo "boot.img has been created"
unzip META-INF.zip
cp ../kernel_out/crypto/ansi_cprng.ko system/lib/modules/ansi_cprng.ko 
cp ../kernel_out/fs/nfs/blocklayout/blocklayoutdriver.ko system/lib/modules/blocklayoutdriver.ko
cp ../kernel_out/drivers/net/wireless/bcmdhd/dhd.ko system/lib/modules/dhd.kocp
cp ../kernel_out/drivers/exfat/exfat_core.ko system/lib/modules/exfat_core.ko 
cp ../kernel_out/drivers/exfat/exfat_fs.ko system/lib/modules/exfat_fs.ko
cp ../kernel_out/drivers/scsi/scsi_wait_scan.ko system/lib/modules/scsi_wait_scan.ko
echo "#################"
echo "Creating flashable zip"
echo "#################"
zip -r tmp META-INF boot.img system
echo "#################"
echo "flashable zip has been created for $choice"
echo "#################"
mv -f tmp.zip ../flash_kyleve.zip
mv -f boot.img ../boot.img
echo "#################"
echo "flashble zip is ready you can under"
cd ..
rm -rf tmp
pwd
echo "#################"
cd ../
exit

;;

kylevess)
echo "#################"
echo " $choice selected"
echo "#################"
export PATH=/home/sr/cm12.1/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin:$PATH
export ARCH=arm
export SUBARCH=arm
export CROSS_COMPILE=arm-eabi-
mkdir -p kernel_out
cp -f ./arch/arm/configs/bcm21664_hawaii_ss_kylevess_rev00_defconfig ./kernel_out/.config
make -j8 ARCH=arm KBUILD_OUTPUT=./kernel_out oldnoconfig
make -j8 ARCH=arm KBUILD_OUTPUT=./kernel_out
cp -f ./kernel_out/arch/arm/boot/zImage ./arch/arm/boot/
cp -rf ./kernel_out/arch/arm/boot/compressed/vmlinux ./arch/arm/boot/compressed/
cp -rf ./kernel_out/vmlinux ./
mkdir tmp
cd tmp
cp -f ../arch/arm/boot/zImage ./zImage
cp ../arch/arm/tools/boot.img-ramdisk.cpio.gz-kylevess ./boot.img-ramdisk.cpio.gz
cp ../arch/arm/tools/mkbootimg ./mkbootimg
cp ../arch/arm/tools/META-INF_kylevess.zip ./META-INF.zip
cp ../arch/arm/tools/system.zip ./system.zip
unzip system.zip
rm -rf system.zip
echo "#################"
echo "Creating boot.img"
echo "#################"
./mkbootimg --kernel zImage --ramdisk boot.img-ramdisk.cpio.gz --board kylevess --pagesize 4096 --base 0x82000000 -o boot.img
echo "#################"
echo "boot.img has been created"
echo "#################"
unzip META-INF.zip
cp ../kernel_out/crypto/ansi_cprng.ko system/lib/modules/ansi_cprng.ko 
cp ../kernel_out/fs/nfs/blocklayout/blocklayoutdriver.ko system/lib/modules/blocklayoutdriver.ko
cp ../kernel_out/drivers/net/wireless/bcmdhd/dhd.ko system/lib/modules/dhd.kocp
cp ../kernel_out/drivers/exfat/exfat_core.ko system/lib/modules/exfat_core.ko 
cp ../kernel_out/drivers/exfat/exfat_fs.ko system/lib/modules/exfat_fs.ko
cp ../kernel_out/drivers/scsi/scsi_wait_scan.ko system/lib/modules/scsi_wait_scan.ko
echo "#################"
echo "Creating flashable zip"
echo "#################"
zip -r tmp META-INF boot.img system
echo "#################"
echo "flashable zip has been created for $choice."
echo "#################"
mv -f tmp.zip ../flash_kylevess.zip
mv -f boot.img ../boot.img
echo "#################"
echo "flashble zip is ready you can under"
cd ..
rm -rf tmp
pwd
echo "#################"
cd ../
exit
esac
done
