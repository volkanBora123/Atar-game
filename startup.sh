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

sudo mkdir -p "$MOUNT_DIR"

if [ ! -f "$DISK_IMAGE" ]; then
    echo "Error: Disk image $DISK_IMAGE is missing. Exiting."
    exit 1
fi

echo "Mounting $DISK_IMAGE to $MOUNT_DIR..."
sudo mount -o loop "$DISK_IMAGE" "$MOUNT_DIR"

if [ $? -eq 0 ]; then
    echo "Disk image $DISK_IMAGE mounted successfully to $MOUNT_DIR."
else
    echo "Error: Failed to mount $DISK_IMAGE. Exiting."
    exit 1
fi

echo "Creating device file $DEVICE_FILE..."
sudo mknod $DEVICE_FILE b 7 0

if [ $? -eq 0 ]; then
    echo "Device file $DEVICE_FILE created successfully."
else
    echo "Error: Failed to create device file $DEVICE_FILE. Exiting."
    exit 1
fi

sudo ls -l $DEVICE_FILE

echo "Here are the contents of the mounted disk:"
sudo ls "$MOUNT_DIR"
