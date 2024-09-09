#!/bin/bash
source /opt/sigil/src/scripts/sigil.sh

# Validate that required variables are defined
if [[ -z "$BSORT_TARGET_DIR" || -z "$BSORT_GROUPS" || -z "$BSORT_KEYWORDS" || -z "$BSORT_EXT" ]]; then
  echo "Error: One or more required variables (BSORT_TARGET_DIR, BSORT_GROUPS, BSORT_KEYWORDS, BSORT_EXT) are missing in '$META_FILE_PATH'. Exiting."
  exit 1
fi

echo "Using ${META_FILE_PATH}"

# Ensure the target base directory exists
mkdir -p "$BSORT_TARGET_DIR"

# Process files based on BSORT_GROUPS and BSORT_KEYWORDS
for keyword_group in "${BSORT_KEYWORDS[@]}"; do
  # Split keyword:group into separate variables
  IFS=":" read -r keyword group <<< "$keyword_group"

  # Check if the group is valid
  if [[ ! " ${BSORT_GROUPS[*]} " =~ " $group " ]]; then
    echo "Error: Group '$group' is not valid. Skipping keyword '$keyword'."
    continue
  fi

  # Ensure the directory for the group exists
  TARGET_DIR="$BSORT_TARGET_DIR/$group"
  mkdir -p "$TARGET_DIR"

  # Move files for each extension
  for ext in "${BSORT_EXT[@]}"; do
    # Find and move files that match the pattern
    files_to_move=("$keyword"*."$ext")
    num_files=0

    # Check if files exist before moving
    if ls "${files_to_move[@]}" >/dev/null 2>&1; then
      for file in "${files_to_move[@]}"; do
        mv "$file" "$TARGET_DIR"
        ((num_files++))
      done
      echo "Moved $num_files file(s) matching $keyword*.$ext to $TARGET_DIR"
    else
      echo "No files found matching $keyword*.$ext"
    fi
  done
done