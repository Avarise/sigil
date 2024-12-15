#!/bin/bash

# Generic backup script, that archives a specified path to a tar.gz
# and places it within user's home directory

set -e

# Check if a directory is provided as the first argument
if [ -z "$1" ]; then
    echo "Usage: $0 <directory_to_backup>"
    exit 1
fi

DIRECTORY=$(realpath "$1")

if [ ! -d "$DIRECTORY" ]; then
    echo "Error: $DIRECTORY is not a valid directory."
    exit 1
fi

TOP_DIR=$(basename "$DIRECTORY")
TIMESTAMP=$(date +"%Y-%m-%d_%H-%M-%S")
ARCHIVE_NAME="${TOP_DIR}-${TIMESTAMP}.tar.gz"
DEST_PATH="${HOME}/${ARCHIVE_NAME}"

tar -czf "$DEST_PATH" -C "$(dirname "$DIRECTORY")" "$TOP_DIR"
echo "Backup completed: $DEST_PATH"
