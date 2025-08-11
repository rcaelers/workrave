#!/bin/bash

set -e

if [ $# -eq 0 ]; then
    echo "Usage: $0 <input-file> [input-file] ..."
    echo "Example: $0 myfile.tar archive.tar"
    echo "Output: Creates myfile.tar.sigstore and archive.tar.sigstore"
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

    OUTPUT_FILE="${INPUT_FILE}.sigstore"

    if curl -X POST http://studio.local:50051/sign/cosign \
        -F "file=@${INPUT_FILE}" \
        --output "$OUTPUT_FILE" \
        --silent \
        --show-error \
        --fail; then

        echo "✅ Successfully created: $OUTPUT_FILE"

        if [ -s "$OUTPUT_FILE" ]; then
            echo "✅ Output file '$OUTPUT_FILE' is valid (non-empty)"
        else
            echo "❌ Warning: Output file '$OUTPUT_FILE' is empty"
            rm -f "$OUTPUT_FILE"
        fi

    else
        echo "❌ Failed to sign file '$INPUT_FILE'"
        [ -f "$OUTPUT_FILE" ] && rm -f "$OUTPUT_FILE"
    fi
    echo
done
