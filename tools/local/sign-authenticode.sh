#!/bin/bash

set -e

curl_with_dns_retry() {
    local attempt=1
    local max_attempts=4
    local curl_status

    while true; do
        if curl "$@"; then
            return 0
        else
            curl_status=$?
        fi

        # A failed hostname lookup has not sent the signing request.
        if [ "$curl_status" -ne 6 ] || [ "$attempt" -ge "$max_attempts" ]; then
            return "$curl_status"
        fi

        attempt=$((attempt + 1))
        echo "Hostname lookup failed; retrying in 1 second (attempt $attempt/$max_attempts)" >&2
        sleep 1
    done
}

if [ $# -eq 0 ]; then
    echo "Usage: $0 <input-file> [input-file] ..."
    echo "Example: $0 installer.exe setup.exe"
    echo "Output: Overwrites each input file with signed version"
    exit 1
fi

if ! curl_with_dns_retry -sk --show-error --fail https://studio.local:50051/health > /dev/null; then
    echo "Error: Cannot reach Workrave signing service at https://studio.local:50051/health" >&2
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

    if curl_with_dns_retry -k -X POST https://studio.local:50051/sign/authenticode \
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
        curl_status=$?
        echo "❌ Signing failed for '$INPUT_FILE' (curl exit status $curl_status):"
        if [ -f "$TEMP_OUTPUT" ] && ERROR_MSG=$(jq -r '.error // "Unknown error"' "$TEMP_OUTPUT" 2>/dev/null); then
            echo "$ERROR_MSG"
        else
            echo "Request failed"
        fi
        rm -f "$TEMP_OUTPUT"
        exit 1
    fi
done
exit 0
