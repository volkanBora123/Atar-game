#!/bin/bash

DISK_IMAGE="storage_vgc.img"
MOUNT_DIR="./mount"
DEVICE_FILE="/dev/mydevice"

if mount | grep "$MOUNT_DIR" > /dev/null; then
    echo "Unmounting $MOUNT_DIR..."
    sudo umount "$MOUNT_DIR"
    
    if [ $? -eq 0 ]; then
        echo "$MOUNT_DIR unmounted successfully."
    else
        echo "Error: Failed to unmount $MOUNT_DIR."
        exit 1
    fi
else
    echo "$MOUNT_DIR is not mounted."
fi

if [ -d "$MOUNT_DIR" ]; then
    sudo rm -rf "$MOUNT_DIR"
    echo "Mount directory $MOUNT_DIR removed."
else
    echo "Mount directory $MOUNT_DIR does not exist."
fi

if [ -e "$DEVICE_FILE" ]; then
    sudo rm -f "$DEVICE_FILE"
    echo "Device file $DEVICE_FILE removed."
else
    echo "Device file $DEVICE_FILE does not exist."
fi

if [ -f "$DISK_IMAGE" ]; then
    sudo rm -f "$DISK_IMAGE"
    echo "Disk image $DISK_IMAGE removed."
else
    echo "Disk image $DISK_IMAGE does not exist."
fi

echo "Purge completed successfully."
