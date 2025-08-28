#!/bin/bash

set -e

if [ $# -eq 0 ]; then
    echo "Usage: $0 <input-file> [input-file] ..."
    echo "Example: $0 installer.exe setup.exe"
    echo "Output: Overwrites each input file with signed version"
    exit 1
fi

if ! curl -s http://studio.local:50051/health > /dev/null 2>&1; then
    echo "Error: Workrave signing service is not running"
    exit 1
fi

for INPUT_FILE in "$@"; do
    echo "Processing: $INPUT_FILE"

    if [ ! -f "$INPUT_FILE" ]; then
        echo "❌ Error: Input file '$INPUT_FILE' does not exist"
        exit 1
    fi

    OUTPUT_FILE="$INPUT_FILE"
    TEMP_OUTPUT="${OUTPUT_FILE}.tmp"

    if curl -X POST http://studio.local:50051/sign/authenticode \
        -F "file=@${INPUT_FILE}" \
        --output "$TEMP_OUTPUT" \
        --silent \
        --show-error \
        --fail; then

        if [ -s "$TEMP_OUTPUT" ]; then
            mv "$TEMP_OUTPUT" "$OUTPUT_FILE"
            echo "✅ File '$INPUT_FILE' successfully signed and saved"
        else
            echo "❌ Warning: Output file '$INPUT_FILE' is empty"
            rm -f "$TEMP_OUTPUT"
            exit 1
        fi

    else
        echo "❌ Signing failed for '$INPUT_FILE':"
        if [ -f "$TEMP_OUTPUT" ] && ERROR_MSG=$(jq -r '.error // "Unknown error"' "$TEMP_OUTPUT" 2>/dev/null); then
            echo "$ERROR_MSG"
        else
            echo "HTTP request failed"
        fi
        rm -f "$TEMP_OUTPUT"
        exit 1
    fi
done
exit 0
