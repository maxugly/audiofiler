#!/bin/bash

# Target directory
SEARCH_DIR="./Source"

echo "--- Searching for potential Doxygen mismatches in $SEARCH_DIR ---"

# Find all @param tags and show their context
# This helps identify if the param listed actually exists in the code below it
grep -r -n -A 3 "@param" "$SEARCH_DIR" | grep -v "\-\-" 

echo ""
echo "--- Checking for 'naked' Doxygen blocks (potential orphans) ---"
# Finds /** blocks that might not be attached to functions correctly
grep -r -n "/\*\*" "$SEARCH_DIR" -A 1 | grep -E "(\*\/|virtual|void|int|float|double|bool|juce::)"

echo ""
echo "Done. Share this list with Jules."
