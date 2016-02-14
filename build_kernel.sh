#!/bin/bash

cd SiniTurk
   echo "Cleaning old stuff"
	rm -f zImage
	rm -f *.zip
	rm -f META-INF/com/google/android/updater-script
	rm -f META-INF/com/android/metadata
   echo "Copying new zImage"
	cp ../arch/arm/boot/zImage zImage
   echo "Creating boot.img"
   echo "#################"
   echo "Choose the device"
	choice=""
	while [ "$choice" != "q" ]
	  do
  echo "  kyleve  "
  echo "  kylevess   " &&   echo -n ""
	   read choice

	case $choice in

kyleve)
  
  echo "$choice selected"
  echo "################"
  echo "Creating boot.img"
	./mkbootimg --kernel zImage --ramdisk kyleve-ramdisk.cpio.gz --board kyleve --pagesize 4096 --base 0x82000000 -o boot.img
  echo "boot.img has been created"
  echo "Setting-up updater-script for $choice"
echo 'assert(getprop("ro.product.device") == "kyleve" ||
getprop("ro.build.product") == "kyleve" || 
getprop("ro.product.device") == "S7392" ||
getprop("ro.build.product") == "S7392" || 
getprop("ro.product.device") == "GT-S7392" ||
getprop("ro.build.product") == "GT-S7392" || 
getprop("ro.product.device") == "hawaii" ||
getprop("ro.build.product") == "hawaii" ||
abort("This package is for device: kyleve,S7392,GT-S7392,hawaii; this device is " + getprop("ro.product.device") + "."););
package_extract_file("boot.img", "/dev/block/platform/sdhci.1/by-name/KERNEL");
show_progress(0.100000, 0);' > META-INF/com/google/android/updater-script
  echo "Setting-up metadata for $choice"
echo 'post-build=samsung/cm_kyleve/kyleve:4.4.4/KTU84Q/4d4a8ba998:userdebug/test-keys
post-timestamp=1454518601
pre-device=kyleve' > META-INF/com/android/metadata
  echo "Creating flashable zip"
	if [ -f boot.img ];
	then
		zip -r SiniTurk_kernel_$choice-`date +%Y-%m-%d`.zip META-INF boot.img
	else
		echo "boot.img is not existed"
	fi
   echo "flashable zip has been created for $choice"
exit

;;

kylevess)
  
  echo " $choice selected"
  echo "################"
  echo "Creating boot.img"
	./mkbootimg --kernel zImage --ramdisk kylevess-ramdisk.gz --board kylevess --pagesize 4096 --base 0x82000000 -o boot.img
  echo "boot.img has been created"
  echo "Setting-up updater-script for $choice"
echo 'assert(getprop("ro.product.device") == "kylevess" ||
getprop("ro.build.product") == "kylevess" || 
getprop("ro.product.device") == "S73920" ||
getprop("ro.build.product") == "S7390" || 
getprop("ro.product.device") == "GT-S7390" ||
getprop("ro.build.product") == "GT-S7390" || 
getprop("ro.product.device") == "hawaii" ||
getprop("ro.build.product") == "hawaii" ||
abort("This package is for device: kylevess,S7390,GT-S7390,hawaii; this device is " + getprop("ro.product.device") + "."););
package_extract_file("boot.img", "/dev/block/platform/sdhci.1/by-name/KERNEL");
show_progress(0.100000, 0);' > META-INF/com/google/android/updater-script
  echo "Setting-up metadata for $choice"
echo 'post-build=samsung/cm_kylevess/kylevess:4.4.4/KTU84Q/4d4a8ba998:userdebug/test-keys
post-timestamp=1454518601
pre-device=kylevess' > META-INF/com/android/metadata
  echo "Creating flashable zip"
	if [ -f boot.img ];
	then
		zip -r SiniTurk_kernel_$choice-`date +%Y-%m-%d`.zip META-INF boot.img
	else
		echo "boot.img is not existed"
	fi
   echo "flashable zip has been created for $choice."
exit

esac
done
