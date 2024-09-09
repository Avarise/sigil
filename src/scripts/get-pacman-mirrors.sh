#!/bin/bash

# Ensure the script runs with root privileges
if [ "$EUID" -ne 0 ]; then
    echo "Please run as root."
    exit 1
fi

# Default configuration
COUNTRIES="US,DE,FR,PL"  # Replace with your preferred countries
MAX_MIRRORS=20
PROTOCOLS="https"

# Install reflector if not already installed
if ! command -v reflector &>/dev/null; then
    echo "Reflector not found. Installing it..."
    pacman -S --noconfirm reflector
fi

# Generate the mirrorlist
reflector \
    --latest 20 \
    --number $MAX_MIRRORS \
    --protocol $PROTOCOLS \
    --sort rate \
    --country $COUNTRIES \
    --save /etc/pacman.d/mirrorlist

echo "Updated /etc/pacman.d/mirrorlist"
cat /etc/pacman.d/mirrorlist
pacman -Syy