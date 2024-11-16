#! /bin/bash

if stat /dev/sda; then
	echo ;
else
	echo "/dev/sda not found";
	exit 1;	
fi

echo "will flash buildroot output to /dev/sda"
read agree

if [ "$agree" = "y" ]; then
	echo "executing";
else
	echo "aborting";
	exit 1;
fi
set -x
/bin/sudo -k dd if=./output/images/sdcard.img of=/dev/sda bs=4M
set +x
