#!/bin/bash

# SigilVM installation and cleanup script
# This sript will not be installed as part of
# Project installation, unlike backup, bsort,
# pfstamp and others

set -e

PROJ_DIR="/sigil/vm"
BUILD_DIR="/sigil/vm/build"
IMGUI_DIR="/sigil/vm/src/extern/imgui"
SHADER_DIR="/sigil/vm/src/shaders"
SCRIPTS_DIR="/sigil/vm/src/scripts"
SHADER_CACHE_DIR="/sigil/shader-cache"

add_sigil_to_bashrc() {
    local SIGILRC="$HOME/.config/sigilrc"
    local BASHRC="$HOME/.bashrc"
    local MARKER="# Sigil"
    local SOURCE_CMD="[ -f \"$SIGILRC\" ] && source \"$SIGILRC\""

    # Check if ~/.config/sigilrc exists
    if [[ ! -f "$SIGILRC" ]]; then
        echo "Sigil configuration file not found: $SIGILRC"
        return 1
    fi

    # Check if ~/.bashrc already contains "# Sigil"
    if grep -qxF "$MARKER" "$BASHRC"; then
        echo "Sigil configuration already added to ~/.bashrc"
        return 0
    fi

    # Append to ~/.bashrc
    {
        echo ""
        echo "$MARKER"
        echo "$SOURCE_CMD"
    } >> "$BASHRC"

    echo "Added Sigil configuration to ~/.bashrc"
}

build_project() {
    # Ensure directories exist
    if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating directory: $BUILD_DIR"
    mkdir -p "$BUILD_DIR"
    fi

    # Ensure docking branch for Dear ImGui
    cd $PROJ_DIR
    git submodule update --init --recursive
    cd $IMGUI_DIR && git checkout docking

    # Prepare build directory
    cmake -B "$BUILD_DIR" -S "$PROJ_DIR"

    # Build project
    cd "$BUILD_DIR"
    make -j$(nproc)
}

install_project() {
    if [ ! -d "$BUILD_DIR" ]; then
    echo "Error, build project first"
    return 1
    fi

    # Build project
    cd "$BUILD_DIR"
    make install_config

    # Copy cache for LSP support
    cp $BUILD_DIR/compile_commands.json $PROJ_DIR/compile_commands.json
    add_sigil_to_bashrc
}

clean_project() {
    if [ -d "$BUILD_DIR" ]; then
    echo "Removing directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
    fi

    if [ -d "$SHADER_CACHE_DIR" ]; then
    echo "Removing directory: $SHADER_CACHE_DIR"
    rm -rf "$SHADER_CACHE_DIR"
    fi

    cd /sigil/vm
    echo "Shutting down git submodules"
    git submodule deinit --all --force
}

test_project() {
    cd "$BUILD_DIR"
    make test
}

compile_all_shaders() {
    # Create the cache directory if it doesn't exist
    mkdir -p "$SHADER_CACHE_DIR"

    # Compile each shader in the shader directory
    for shader in "$SHADER_DIR"/*; do
        if [[ -f "$shader" ]]; then
            # Extract the filename without extension
            filename=$(basename -- "$shader")
            extension="${filename##*.}"
            filename="${filename%.*}"
            
            # Determine the appropriate output file extension for SPIR-V binaries (.spv)
            output_file="$SHADER_CACHE_DIR/$filename.$extension.spv"

            # Compile the shader depending on its extension
            case "$extension" in
                vert|frag|comp|tesc|tese|geom)
                    glslangValidator -V "$shader" -o "$output_file"
                    echo "Compiled $shader to $output_file"
                    ;;
                *)
                    echo "Skipping $shader: Unsupported shader type."
                    ;;
            esac
        fi
    done

    echo "All shaders have been compiled."
}

manual_build() {
    echo "Running SigilVM install script"
    while true; do
        read -rp "Build, test and install SigilVM? (Y/N): " response
        case "$response" in
            [Yy])
                build_project
                compile_all_shaders
                test_project
                install_project
                exit 0
                ;;
            [Nn]) 
                echo "Exiting script."
                exit 1
                ;;
            *) 
                echo "Invalid input. Please enter Y or N."
                ;;
        esac
    done
}

# Function to show help
script_help_message() {
    echo "Usage: $0 [OPTION]"
    echo "Run without arguments to launch interactive installer"
    echo "To remove/edit user config, delete/edit ~./config/sigilrc"
    echo "Available options:"
    echo "    --help          Show this help message"
    echo "    --test          Run SigilVM test suite"
    echo "    --build         Compile project in $(pwd)/build"
    echo "    --rebuild       Reset $(pwd)/build and compile again"
    echo "    --install       Install project for $(whoami)"
    echo "    --clean         Remove $(pwd)/build"
    echo "    --shaders       Compile shaders in $(pwd)/src/shaders"
}

if [ $# -eq 0 ]; then
    manual_build
    exit 0
fi

case "$1" in
    --help)
        script_help_message
        ;;
    --shaders)
        compile_all_shaders
        ;;
    --install)
        install_project
        ;;
    --build)
        build_project
        ;;
    --rebuild)
        clean_project
        build_project
        ;;
    --clean)
        clean_project
        ;;
    --test)
        test_project
        ;;
    *)
        echo "Unknown option: $1"
        script_help_message
        exit 1
        ;;
esac
