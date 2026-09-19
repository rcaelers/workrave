#!/bin/bash
#
# GPG signing proxy for debuild.
# Delegates GPG signing to the workrave signing service.
#
# Usage in debuild:
#   debuild -p"/path/to/gpg-sign-client.sh" -d -S -sa -kKEYID
#
# The signing service must be running at SIGNING_SERVICE_URL.
#
# debuild/dpkg-buildpackage calls the signing program with gpg-compatible arguments:
#   <program> --utf8-strings --textmode --armor --status-fd N -u KEYID --output OUTPUT [--detach-sign] INPUT
#

set -euo pipefail

if [[ -z "${SIGNING_SERVICE_URL:-}" ]]; then
    echo "Error: SIGNING_SERVICE_URL is not set" >&2
    exit 1
fi

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

# Send signing request to the service.
# -S: still report curl's own errors (unreachable host, TLS, ...) despite -s.
# The exit status is captured explicitly so that a failed connection produces
# a useful message instead of silently aborting under set -e.
echo "gpg-sign-client: signing $(basename "$INPUT_FILE") (${MODE}) via ${SIGNING_SERVICE_URL}" >&2

CURL_ERR=$(mktemp)
trap 'rm -f "$CURL_ERR"' EXIT

set +e
HTTP_CODE=$(curl -sSk -o "$OUTPUT_FILE" -w "%{http_code}" \
    --connect-timeout 10 \
    -X POST "${SIGNING_SERVICE_URL}/sign/gpg" \
    -F "file=@${INPUT_FILE}" \
    -F "mode=${MODE}" 2>"$CURL_ERR")
CURL_STATUS=$?
set -e

if [[ $CURL_STATUS -ne 0 ]]; then
    echo "Error: cannot reach signing service at ${SIGNING_SERVICE_URL} (curl exit ${CURL_STATUS}): $(cat "$CURL_ERR")" >&2
    echo "Is the signing service running, and reachable from where this build runs" >&2
    echo "(inside the build container, possibly on a remote podman host)?" >&2
    rm -f "$OUTPUT_FILE"
    exit 1
fi

if [[ "$HTTP_CODE" != "200" ]]; then
    echo "Error: signing service at ${SIGNING_SERVICE_URL} returned HTTP ${HTTP_CODE}" >&2
    if [[ -s "$OUTPUT_FILE" ]]; then
        echo "Response:" >&2
        cat "$OUTPUT_FILE" >&2
        echo >&2
    fi
    rm -f "$OUTPUT_FILE"
    exit 1
fi

if [[ ! -s "$OUTPUT_FILE" ]]; then
    echo "Error: signing service at ${SIGNING_SERVICE_URL} returned an empty signature for $(basename "$INPUT_FILE")" >&2
    rm -f "$OUTPUT_FILE"
    exit 1
fi

# Emit GPG status messages on the status FD if requested.
# dpkg-buildpackage checks for SIG_CREATED to confirm signing succeeded.
if [[ -n "$STATUS_FD" ]]; then
    echo "[GNUPG:] SIG_CREATED D 1 8 00 $(date +%s) 0 4 0 1 ${KEY_ID}" >&${STATUS_FD}
fi
