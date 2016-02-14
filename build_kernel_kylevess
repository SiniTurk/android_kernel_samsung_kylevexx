#!/bin/bash

mkdir -p kernel_out
cp -f ./arch/arm/configs/bcm21664_hawaii_ss_kylevess_rev00_defconfig ./kernel_out/.config
make -j8 ARCH=arm KBUILD_OUTPUT=./kernel_out oldnoconfig
make -j8 ARCH=arm KBUILD_OUTPUT=./kernel_out
cp -f ./kernel_out/arch/arm/boot/zImage ./arch/arm/boot/
cp -rf ./kernel_out/arch/arm/boot/compressed/vmlinux ./arch/arm/boot/compressed/
cp -rf ./kernel_out/vmlinux ./
