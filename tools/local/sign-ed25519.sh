#!/bin/bash

set -e

if [ $# -eq 0 ]; then
    echo "Usage: $0 <input-file> [input-file]"
    echo "Example: $0 document.pdf"
    echo "Output: Base64-encoded ED25519 signature for each file"
    exit 1
fi

if ! curl -s http://studio.local:50051/health > /dev/null 2>&1; then
    echo "Error: Workrave signing service is not running" >&2
    exit 1
fi

INPUT_FILE="$1"

if [ ! -f "$INPUT_FILE" ]; then
    echo "Error: Input file '$INPUT_FILE' does not exist" >&2
    exit 1
fi

RESPONSE=$(curl -X POST http://studio.local:50051/sign/ed25519 \
    -F "file=@${INPUT_FILE}" \
    --silent \
    --show-error \
    --fail 2>/dev/null)

if [ $? -ne 0 ]; then
    echo "Error: Failed to sign '$INPUT_FILE'" >&2
    exit 1
fi

SIGNATURE=$(echo "$RESPONSE" | jq -r '.signature' 2>/dev/null)

if [ $? -eq 0 ] && [ "$SIGNATURE" != "null" ] && [ -n "$SIGNATURE" ]; then
    echo "$SIGNATURE"
else
    echo "Error: Invalid signature response for '$INPUT_FILE'" >&2
    exit 1
fi

exit 0
