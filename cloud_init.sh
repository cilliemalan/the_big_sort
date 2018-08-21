#!/bin/bash
# This is the init script to mount and configure the nvme drives on an m5d.4xlarge server

# make /tmp be a ram filesystem
cat <<EOF >> /etc/fstab
tmpfs   /tmp       tmpfs   defaults,noatime,mode=1777,size=10g    0  0
tmpfs   /var/spool tmpfs   defaults,noatime,mode=1777,size=10m   0  0
tmpfs   /var/tmp   tmpfs   defaults,noatime,mode=1777,size=10m   0  0
EOF

mount -a

# we're going to need some deps
apt update
apt install -y docker.io build-essential

mkdir -p /tmp/build
cd /tmp/build

git clone https://github.com/cilliemalan/the_big_sort
cd the_big_sort/src
make
make install

cd /
rm -rf /tmp/build


# create and mount NVME partitions and filesystems

function initialize {
local drive=$1
local mountpoint=$2
local part=${drive}p1
sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk ${drive}
  o # clear the in memory partition table
  n # new partition
  p # primary partition
  1 # partition number 1
    # default - start at beginning of disk
  +215G  # default - use whole disk
  w # write the partition table
  q # quit just in case
EOF

sleep 1

mkfs.ext4 ${part}
local uuid=$(blkid ${part} | sed -r 's/^.+UUID="(.+?)" TYPE.+/\1/')
echo "UUID=${uuid}  ${mountpoint}     ext4    noatime,discard,errors=remount-ro  0  0" \
    >> /etc/fstab
mkdir -p ${mountpoint}

tune2fs -O ^has_journal ${part}
tune2fs -o discard ${part}
}

initialize /dev/nvme1n1 /src
initialize /dev/nvme2n1 /dst
mount -a
