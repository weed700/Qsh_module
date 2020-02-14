#!/bin/bash

dir="/root/Q_sh/qsh_host_dir"

echo "make directory"
mkdir $dir

path=$1
size=$2
size='+'$size

(echo n; echo l; echo ""; echo $size; echo w;) | fdisk $path

path=`fdisk -l $path | tail -n 1 | cut -d ' ' -f1`

echo "spare disk format"
(echo y;) | mkfs.ext4 $path

echo "mount"
mount $path ./sh_0820_a
