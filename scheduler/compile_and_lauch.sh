#!/bin/bash
make -C /home/lando/didattica/aos/tmp-linux/linux-4.16.15  M=`pwd` DEBUG=$1
cp *.ko ../compiled/
cd ../compiled
find . | cpio -H newc -o | gzip > ../filesystem/mod.gz
cd ../filesystem
cat tinyfs.gz mod.gz > fs.gz 
cd ..
qemu-system-x86_64 -kernel bzImage -initrd filesystem/fs.gz 