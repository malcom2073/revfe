import os
from pathlib import Path

BACKEND_DIR = Path(__file__).resolve().parent.parent

INCUS_URL = os.environ.get("INCUS_URL", "https://10.0.0.161:8443")
CLIENT_CERT = os.environ.get(
    "INCUS_CLIENT_CERT", str(BACKEND_DIR / "certs" / "client.crt")
)
CLIENT_KEY = os.environ.get("INCUS_CLIENT_KEY", str(BACKEND_DIR / "certs" / "client.key"))
VERIFY_TLS = os.environ.get("INCUS_VERIFY_TLS", "false").lower() == "true"

FRONTEND_DIST = os.environ.get(
    "FRONTEND_DIST", str(BACKEND_DIR.parent / "frontend" / "dist")
)

API_PREFIX = "/api/v1"
