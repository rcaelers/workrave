#!/bin/bash
#
# GPG signing proxy for debuild.
# Delegates GPG signing to the workrave signing service.
#
# Usage in debuild:
#   debuild -p"/path/to/gpg-sign-client.sh" -d -S -sa -kKEYID
#
# The signing service must be running at SIGNING_SERVICE_URL (default: https://127.0.0.1:50051).
#
# debuild/dpkg-buildpackage calls the signing program with gpg-compatible arguments:
#   <program> --utf8-strings --textmode --armor --status-fd N -u KEYID --output OUTPUT [--detach-sign] INPUT
#

set -euo pipefail

SIGNING_SERVICE_URL="${SIGNING_SERVICE_URL:-https://127.0.0.1:50051}"

# Parse gpg-compatible arguments
STATUS_FD=""
OUTPUT_FILE=""
KEY_ID=""
MODE="clearsign"
INPUT_FILE=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --status-fd)
            STATUS_FD="$2"
            shift 2
            ;;
        --output|-o)
            OUTPUT_FILE="$2"
            shift 2
            ;;
        -u)
            KEY_ID="$2"
            shift 2
            ;;
        --detach-sign)
            MODE="detach-sign"
            shift
            ;;
        --clearsign)
            MODE="clearsign"
            shift
            ;;
        # Ignore these gpg options
        --utf8-strings|--textmode|--armor|--batch|--no-tty|--yes)
            shift
            ;;
        --pinentry-mode|--passphrase-file|--passphrase-fd)
            shift 2
            ;;
        -b|-s|-a)
            shift
            ;;
        -*)
            # Skip unknown flags
            shift
            ;;
        *)
            # Positional argument = input file
            INPUT_FILE="$1"
            shift
            ;;
    esac
done

if [[ -z "$INPUT_FILE" ]]; then
    echo "Error: no input file specified" >&2
    exit 1
fi

if [[ -z "$OUTPUT_FILE" ]]; then
    echo "Error: no output file specified (--output)" >&2
    exit 1
fi

# Send signing request to the service
HTTP_CODE=$(curl -sk -o "$OUTPUT_FILE" -w "%{http_code}" \
    -X POST "${SIGNING_SERVICE_URL}/sign/gpg" \
    -F "file=@${INPUT_FILE}" \
    -F "mode=${MODE}")

if [[ "$HTTP_CODE" != "200" ]]; then
    echo "Error: signing service returned HTTP ${HTTP_CODE}" >&2
    if [[ -f "$OUTPUT_FILE" ]]; then
        cat "$OUTPUT_FILE" >&2
    fi
    exit 1
fi

# Emit GPG status messages on the status FD if requested.
# dpkg-buildpackage checks for SIG_CREATED to confirm signing succeeded.
if [[ -n "$STATUS_FD" ]]; then
    echo "[GNUPG:] SIG_CREATED D 1 8 00 $(date +%s) 0 4 0 1 ${KEY_ID}" >&${STATUS_FD}
fi
