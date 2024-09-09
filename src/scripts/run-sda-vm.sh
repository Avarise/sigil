#!/bin/bash
if [[ "$EUID" -ne 0 ]]; then
  echo "Error: This script must be run as root. Please use sudo or log in as root."
  exit 1
fi

source /opt/sigil/src/scripts/sigil.sh
echo "Using ${META_FILE_PATH}"

# Variables for VM configuration
BIOS_DIR="/opt/sigil/assets/blobs"
BIOS_PATH="/opt/sigil/assets/blobs/tianocore.bin"
DRIVE_PATH="/dev/sda"
RAM_SIZE="4G"
CPU_CORES=4

# Ensure libvirtd is active
ensure_libvirtd_active

# Run QEMU to launch the VM
echo "Launching the virtual machine..."

qemu-system-x86_64 \
    -enable-kvm \
    -m "$RAM_SIZE" \
    -smp "$CPU_CORES" \
    -drive file=/dev/sda,format=raw,media=disk \
    -L "$BIOS_DIR" --bios "$BIOS_PATH" \
    -cpu host \
    -net nic -net user \
    -vga qxl \
    -boot c

if [ $? -eq 0 ]; then
    echo "Virtual machine launched successfully."
else
    echo "Failed to launch virtual machine."
    exit 1
fi
