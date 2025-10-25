#!/bin/bash
export BR2_EXTERNAL=$(pwd)/buildroot_ext
export BR2_DEFCONFIG=$(pwd)/buildroot_ext/configs/hlk7621_defconfig
#export BR2_LINUX_KERNEL_CUSTOM_CONFIG_FILE=$(pwd)/buildroot_ext/hilink/mt7621/linux.config


pushd buildroot > /dev/null
"$@"
popd > /dev/null
