#!/bin/bash
source "/opt/sigil/src/scripts/sigil.sh"

# Check if the configuration is loaded
if [ -z "$DMAIN_USER" ] || [ -z "$DMAIN_HOST" ] || [ -z "$DMAIN_DIR" ] || [ -z "$ST_DIR" ]; then
    echo "Error: Missing configuration in sigil.meta.sh."
    exit 1
fi

# Sync from DMain to ST
echo "Syncing from DMain to ST..."
rsync -avz --progress "${DMAIN_USER}@${DMAIN_HOST}:${DMAIN_DIR}/" "${ST_DIR}/"

echo "Sync completed!"
