#!/bin/bash

DISK_IMAGE="storage_vgc.img"
DISK_SIZE="64M"

MKFS_EXT4_PATH=$(command -v mkfs.ext4 || echo "/sbin/mkfs.ext4")

if [ ! -x "$MKFS_EXT4_PATH" ]; then
    echo "Error: mkfs.ext4 command not found. Please ensure it is installed and available in your PATH."
    exit 1
fi

if [ -f "$DISK_IMAGE" ]; then
    echo "Disk image $DISK_IMAGE already exists. Removing it..."
    sudo rm -f "$DISK_IMAGE"
    echo "Disk image $DISK_IMAGE removed successfully."
fi

echo "Creating a new disk image $DISK_IMAGE of size $DISK_SIZE..."
sudo dd if=/dev/zero of=$DISK_IMAGE bs=1M count=64 status=progress

if [ $? -eq 0 ]; then
    echo "Disk image $DISK_IMAGE created successfully."
else
    echo "Error: Failed to create the disk image."
    exit 1
fi

echo "Formatting the disk image with ext4 filesystem..."
sudo $MKFS_EXT4_PATH $DISK_IMAGE

if [ $? -eq 0 ]; then
    echo "Disk image $DISK_IMAGE formatted successfully."
else
    echo "Error: Failed to format the disk image."
    exit 1
fi

echo "Disk image initialization completed successfully!"
