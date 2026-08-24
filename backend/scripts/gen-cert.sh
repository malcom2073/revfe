#!/usr/bin/env bash
set -euo pipefail

CERT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)/certs"
CERT="$CERT_DIR/client.crt"
KEY="$CERT_DIR/client.key"

INCUS_HOST="${INCUS_HOST:-10.0.0.161}"
INCUS_URL="${INCUS_URL:-https://$INCUS_HOST:8443}"

mkdir -p "$CERT_DIR"

if [[ -f "$CERT" && -f "$KEY" ]]; then
    echo "==> Client certificate already exists at $CERT"
else
    echo "==> Generating self-signed client certificate..."
    openssl req -x509 -newkey ec \
        -pkeyopt ec_paramgen_curve:prime256v1 \
        -keyout "$KEY" -out "$CERT" \
        -days 825 -nodes \
        -subj "/CN=revfe-o=$(hostname)-$(date +%s)" \
        -addext "subjectAltName=DNS:revfe.local"
    chmod 600 "$KEY"
    echo "==> Generated $CERT and $KEY"
fi

FINGERPRINT="$(openssl x509 -in "$CERT" -noout -fingerprint -sha256 | cut -d= -f2 | tr -d ':' | tr 'A-F' 'a-f')"
echo
echo "==============================================================="
echo " Certificate fingerprint (SHA-256, no colons):"
echo "   $FINGERPRINT"
echo
echo " Next steps — run ON the Incus host ($INCUS_HOST):"
echo
echo "   1. Copy the certificate over:"
echo "        scp $CERT ${REMOTE_USER:-root}@$INCUS_HOST:/tmp/revfe-client.crt"
echo
echo "   2. Trust it with Incus:"
echo "        incus config trust add-certificate /tmp/revfe-client.crt"
echo
echo "   3. Verify from this machine:"
echo "        curl --cert $CERT --key $KEY \\"
echo "             --insecure $INCUS_URL/1.0/instances?recursion=1"
echo "==============================================================="
