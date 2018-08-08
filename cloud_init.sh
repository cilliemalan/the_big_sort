#!/bin/bash
# This is the init script to mount and configure the nvme drives on an m5d.4xlarge server

# we're going to need some deps
apt update
apt install docker.io build-essential

mkdir -p /tmp/build
cd /tmp/build

git clone https://github.com/cilliemalan/the_big_sort
cd the_big_sort
make

cp generate /usr/bin/


# create and mount NVME drives

function initialize(drive,mountpoint) {
sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk ${drive}
  o # clear the in memory partition table
  n # new partition
  p # primary partition
  1 # partition number 1
    # default - start at beginning of disk 
    # default - use whole disk
  w # write the partition table
  q # quit just in case
EOF

mkfs.ext4 ${drive}p1
mkdir -p ${mountpoint}
mount ${drive}p1 ${mountpoint}
}

initialize /dev/nvme1n1 /src
initialize /dev/nvme2n1 /dst