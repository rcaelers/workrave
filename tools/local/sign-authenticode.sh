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
        continue
    fi

    OUTPUT_FILE="$INPUT_FILE"

    if curl -X POST http://studio.local:50051/sign/authenticode \
        -F "file=@${INPUT_FILE}" \
        --output "$OUTPUT_FILE" \
        --silent \
        --show-error \
        --fail; then

        if [ -s "$OUTPUT_FILE" ]; then
            echo "✅ File '$INPUT_FILE' successfully signed and saved"
        else
            echo "❌ Warning: Output file '$INPUT_FILE' is empty"
        fi

    else
        echo "❌ Failed to sign file '$INPUT_FILE'"
        [ -f "$OUTPUT_FILE" ] && rm -f "$OUTPUT_FILE"
    fi
    echo
done
