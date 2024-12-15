#!/bin/sh
# Fetch newest Glorious Eggroll Proton from GitHub
echo "Updating Proton-GE..."
mkdir /tmp/proton-ge-custom
cd /tmp/proton-ge-custom

curl -LOJ $(curl -s https://api.github.com/repos/GloriousEggroll/proton-ge-custom/releases/latest | grep browser_download_url | cut -d\" -f4 | grep -E .tar.gz)
curl -LOJ $(curl -s https://api.github.com/repos/GloriousEggroll/proton-ge-custom/releases/latest | grep browser_download_url | cut -d\" -f4 | grep -E .sha512sum) 
sha512sum -c *.sha512sum
mkdir -p ~/.steam/root/compatibilitytools.d
tar -xf GE-Proton*.tar.gz -C ~/.steam/root/compatibilitytools.d/
echo "Proton-GE updated successfully."
