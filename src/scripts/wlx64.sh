#!/bin/bash

WINLINUX64_DIR="/opt/winlinux64"
WINEPREFIX="$WINLINUX64_DIR/prefix"
VFS_DIR="$WINLINUX64_DIR/vfs"
PROTON_GE_DIR="$HOME/.steam/root/compatibilitytools.d"

function deploy_environment() {
    echo "Deploying WinLinux64 environment..."
    mkdir -p "$WINLINUX64_DIR" "$WINEPREFIX" "$VFS_DIR" || { echo "Failed to create directories"; exit 1; }
    echo "Initializing wine64 prefix..."
    WINEPREFIX="$WINEPREFIX" wine64 wineboot || { echo "Failed to initialize wine prefix"; exit 1; }
    echo "Environment deployed successfully at $WINLINUX64_DIR."
}

function run_with_wine64() {
    local exe_path="$1"
    echo "Running $exe_path with wine64..."
    WINEPREFIX="$WINEPREFIX" wine64 "$exe_path" || { echo "Failed to run $exe_path with wine64"; exit 1; }
}

function run_with_proton_ge() {
    local exe_path="$1"
    local proton_ge_path
    proton_ge_path=$(find "$PROTON_GE_DIR" -maxdepth 1 -type d | grep "Proton" | sort | tail -n 1)

    if [[ -z "$proton_ge_path" ]]; then
        echo "Proton GE not found in $PROTON_GE_DIR."
        exit 1
    fi

    echo "Using Proton GE from $proton_ge_path to run $exe_path..."
    "$proton_ge_path/proton" run "$exe_path" || { echo "Failed to run $exe_path with Proton GE"; exit 1; }
}

function usage() {
    echo "Usage: $0 --deploy"
    echo "       $0 /path/to/file.exe"
    echo "       $0 /path/to/file.exe --proton-ge"
    exit 1
}

# Main script logic
if [[ $# -lt 1 ]]; then
    usage
fi

case "$1" in
    --deploy)
        deploy_environment
        ;;
    /*.exe)
        exe_path="$1"
        if [[ "$2" == "--proton-ge" ]]; then
            run_with_proton_ge "$exe_path"
        else
            run_with_wine64 "$exe_path"
        fi
        ;;
    *)
        usage
        ;;
esac
