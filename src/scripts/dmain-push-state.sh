#!/bin/bash
source "/opt/sigil/src/scripts/sigil.sh"
echo "Using ${META_FILE_PATH}"

# Check if the configuration is loaded
if [ -z "$DMAIN_USER" ] || [ -z "$DMAIN_HOST" ] || [ -z "$DMAIN_DIR" ] || [ -z "$ST_DIR" ]; then
    echo "Error: Missing configuration in sigil.meta.sh."
    exit 1
fi

# Sync from ST to DMain
echo "Syncing from ST to DMain..."
rsync -avz --progress "${ST_DIR}/" "${DMAIN_USER}@${DMAIN_HOST}:${DMAIN_DIR}/"

echo "Sync completed!"
