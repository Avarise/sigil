#!/bin/bash
source /opt/sigil/src/scripts/sigil.sh
echo "Using ${META_FILE_PATH}"

# Validate that required variables are defined
if [[ -z "$extensions" ]]; then
  echo "Error: One or more required variables are missing in '$META_FILE_PATH'. Exiting."
  exit 1
fi

# Ensure a prefix is provided
if [ $# -ne 1 ]; then
    echo "Usage: $0 <prefix>"
    exit 1
fi

prefix=$1

# Iterate over files in the current directory
for file in *; do
    # Skip directories
    if [ -f "$file" ]; then
        # Extract file extension and name
        ext="${file##*.}"
        base="${file%.*}"

        # Check if the file extension is in the list
        if [[ " ${extensions[*]} " == *" $ext "* ]]; then
            # Rename the file if it doesn't start with the prefix
            if [[ "$base" != $prefix* ]]; then
                mv "$file" "${prefix}_$file"
            fi
        fi
    fi
done
