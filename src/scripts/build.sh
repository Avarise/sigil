#!/bin/bash
source /opt/sigil/src/scripts/sigil.sh

echo "Using ${META_FILE_PATH}"

# Ensure directories exist
if [ ! -d "$BIN_DIR" ]; then
echo "Creating directory: $BIN_DIR"
mkdir -p "$BIN_DIR"
fi

if [ ! -d "$BUILD_DIR" ]; then
echo "Creating directory: $BUILD_DIR"
mkdir -p "$BUILD_DIR"
fi

# Ensure docking branch for Dear ImGui
git submodule update --init --recursive
cd "$IMGUI_DIR" && git checkout docking

# Prepare build directory
cmake -B "$BUILD_DIR" -S "$PROJ_DIR"

# Build project
cd "$BUILD_DIR"
make -j$(nproc) && make test

# Move apps from build dir to binary dir
for app in "${APPS[@]}"; do
source_file="$BUILD_DIR/$app"
if [ -f "$source_file" ]; then
    echo "Copying $source_file to $BIN_DIR"
    cp "$source_file" "$BIN_DIR"
    chmod +x "$BIN_DIR/$app"
else
    echo "Warning: $source_file not found, skipping."
fi
done

# Copy scripts and config for clangd
cp /opt/sigil/build/compile_commands.json /opt/sigil/compile_commands.json
cp /opt/sigil/src/scripts/bsort.sh /opt/sigil/bin/bsort
cp /opt/sigil/src/scripts/pfstamp.sh /opt/sigil/bin/pfstamp
chmod +x /opt/sigil/bin/pfstamp
chmod +x /opt/sigil/bin/bsort

# Add /opt/sigil/bin to PATH if not already present
if [[ ":$PATH:" != *":$BIN_DIR:"* ]]; then
    echo "Adding $BIN_DIR to PATH"
    echo "export PATH=\$PATH:$BIN_DIR" >> ~/.bashrc
    export PATH="$PATH:$BIN_DIR"
else
    echo "$BIN_DIR already defined for this env"
fi