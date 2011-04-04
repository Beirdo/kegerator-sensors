#!/bin/bash
# Copyright (c) 2009 Harald Klein <hari@vt100.at>
# Licensed under the terms of the GNU Public license version 3 (see www.fsf.org for details)

# settings
# set -x
DEST=/usr/src/mini2440
UBOOTREPO=git://repo.or.cz/u-boot-openmoko/mini2440.git
KERNELREPO=git://repo.or.cz/linux-2.6/mini2440.git
QEMUREPO=git://repo.or.cz/qemu/mini2440.git
TOOLCHAIN=http://www.codesourcery.com/sgpp/lite/arm/portal/package3696/public/arm-none-linux-gnueabi/arm-2008q3-72-arm-none-linux-gnueabi-i686-pc-linux-gnu.tar.bz2
TOOLCHAINARCHIVE=`basename $TOOLCHAIN`
TOOLCHAINDEST=arm-2008q3
BUSYBOXSRC=http://www.busybox.net/downloads/busybox-1.18.4.tar.bz2
BUSYBOXARCHIVE=`basename $BUSYBOXSRC`
BUSYBOXDIR=`echo $BUSYBOXARCHIVE | sed 's/.tar.bz2//g'`
DROPBEARSRC=http://matt.ucc.asn.au/dropbear/releases/dropbear-0.52.tar.gz
DROPBEARARCHIVE=`basename $DROPBEARSRC`
DROPBEARDIR=`echo $DROPBEARARCHIVE | sed 's/.tar.gz//g'`

CROSS_COMPILE=arm-none-linux-gnueabi-
CC=${CROSS_COMPILE}"gcc -march=armv4t -mtune=arm920t"
PATH=$PATH:${DEST}/${TOOLCHAINDEST}/bin
NMAKE=4

export CROSS_COMPILE
export CC
pushd .
mkdir -p ${DEST}/{uboot,kernel,kernel-bin,qemu,rootfs,output,download}

if 0 ; then

# download bits
cd ${DEST}/download
test -e ${TOOLCHAINARCHIVE} || wget ${TOOLCHAIN} -O ${TOOLCHAINARCHIVE}
test -e ${BUSYBOXARCHIVE} || wget ${BUSYBOXSRC} -O ${BUSYBOXARCHIVE}
test -e ${DROPBEARARCHIVE} || wget ${DROPBEARSRC} -O ${DROPBEARARCHIVE}
cd ${DEST}
test -e ${TOOLCHAINDEST} || tar xjf download/${TOOLCHAINARCHIVE}
tar xjf download/${BUSYBOXARCHIVE}
tar xzf download/${DROPBEARARCHIVE}

test -e ${DEST}/uboot/mini2440 || (
	echo Initial uboot git clone...
	cd ${DEST}/uboot
	git clone ${UBOOTREPO}
) && (
	echo Doing uboot git pull...
	cd ${DEST}/uboot/mini2440
	git pull
)

test -e ${DEST}/kernel/mini2440 || (
	echo Initial kernel git clone...
	cd ${DEST}/kernel
	git clone ${KERNELREPO}
) && (
	echo Doing kernel git pull...
	cd ${DEST}/kernel/mini2440
	git pull
)

test -e ${DEST}/qemu/mini2440 || (
	echo Initial qemu git clone...
	cd ${DEST}/qemu
	git clone ${QEMUREPO}
) && (
	echo Doing qemu git pull...
	cd ${DEST}/qemu/mini2440
	git pull
)

# compile bits
cd ${DEST}/uboot/mini2440
make mini2440_config
make clean all
fi

cd ${DEST}/kernel/mini2440

make ARCH=arm O=../../kernel-bin/ mrproper
make ARCH=arm O=../../kernel-bin/ mini2440_defconfig
make ARCH=arm O=../../kernel-bin/ -j${NMAKE} 
make ARCH=arm O=../../kernel-bin/ -j${NMAKE} modules modules_install

if 0 ; then

cd ${DEST}/qemu/mini2440
./configure --disable-kvm --target-list=arm-softmmu
make clean
make -j${NMAKE}

cd ${DEST}/${BUSYBOXDIR}
make defconfig
make clean
make

cd ${DEST}/${DROPBEARDIR}
./configure --disable-zlib -host arm
make clean
make
make scp

fi


cd ${DEST}
cp uboot/mini2440/u-boot.bin output
./uboot/mini2440/tools/mkimage -A arm -O linux -T kernel -C none -a 0x30008000 -e 0x30008000 -d kernel-bin/arch/arm/boot/zImage output/uImage
cp ${BUSYBOXDIR}/busybox output
cp ${DROPBEARDIR}/{dbclient,dropbear,dropbearkey,scp} output
popd
