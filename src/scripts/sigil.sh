#!/bin/bash
# This file defines common behavior for few scripts of SigilVM
# This file must not be run, and can only be sourced.
if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
  echo "This script must be sourced and cannot be executed directly."
  exit 1
fi

# Sigil common variables
PROJ_DIR="/opt/sigil"
BIN_DIR="/opt/sigil/bin"
BUILD_DIR="/opt/sigil/build"
IMGUI_DIR="/opt/sigil/src/extern/imgui"
META_FILE_NAME="sigil.meta.sh"
APPS=("xorit" "sigil-tools" "gbgen")

# Function to check if libvirtd is running and start it if necessary
ensure_libvirtd_active() {
    if ! systemctl is-active --quiet libvirtd; then
        echo "Starting libvirtd service..."
        systemctl start libvirtd
        if [ $? -ne 0 ]; then
            echo "Failed to start libvirtd. Exiting."
            exit 1
        fi
    else
        echo "libvirtd is already active."
    fi
}

create_meta_file() {
  # Check if PROJ_DIR is defined
  if [[ -z "$PROJ_DIR" ]]; then
    echo "Error: PROJ_DIR variable is not defined. Exiting."
    return 1
  fi

  # Ensure the target directory exists
  mkdir -p "$PROJ_DIR"

  # Define the path for sigil.meta.sh
  local meta_file="$PROJ_DIR/sigil.meta.sh"

  # Write the content to sigil.meta.sh
  {
    echo "#!/bin/bash"
    echo ""
    echo "# Default user"
    echo "DEFAULT_USER=\"$(whoami)\""
  } > "$meta_file"

  # Make the file executable
  chmod +x "$meta_file"

  echo "sigil.meta.sh has been created at $meta_file with DEFAULT_USER=$(whoami)"
  return 0
}

find_meta_file() {
  local dir="$PWD"
  while [[ "$dir" != "/" ]]; do
    if [[ -f "$dir/$META_FILE_NAME" ]]; then
      echo "$dir/$META_FILE_NAME"
      return 0
    fi
    dir=$(dirname "$dir")
  done

  # Check root directory as a last resort
  if [[ -f "/$META_FILE_NAME" ]]; then
    echo "/$META_FILE_NAME"
    return 0
  fi

  # If not found, return error
  return 1
}

# As a last step, we automate meta file inclusion
META_FILE_PATH=$(find_meta_file)

if [[ $? -ne 0 ]]; then
  echo "Meta file '$META_FILE_NAME' not found in current directory or any parent directory up to root."
  echo "Creating local metafile"
  create_meta_file
fi

# Source the meta file
source "$META_FILE_PATH"