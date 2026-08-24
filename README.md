# RevFe

**Rev**olutionary **F**ront**E**nd (for Incus) — a slick, Cockpit-style web
frontend for managing [Incus](https://linuxcontainers.org/incus/)
servers. Python/Flask backend with a pluggable provider layer (Incus today,
more backends later), React + TypeScript + PatternFly v6 frontend.

## Architecture

```
frontend (React 18 + Vite + PatternFly v6 + xterm.js)
   │  /api/v1/* (HTTP, SSE, WebSocket)
backend (Flask)
   │  provider abstraction
incus provider ── TLS client cert ──► https://10.0.0.161:8443
```

- **Backend** (`backend/app`): Flask app. `providers/base.py` defines the
  backend-agnostic `Provider` protocol; `providers/incus.py` implements it via
  the Incus REST API. Long-running Incus operations are awaited server-side.
  Instance events stream to the browser as Server-Sent Events; interactive
  exec is bridged from a browser WebSocket to Incus's exec websockets.
- **Frontend** (`frontend/src`): Dashboard, Instances table with live status +
  start/stop/restart/delete, create-instance wizard, instance detail with
  Overview and Console (xterm.js) tabs.

## Setup

```bash
make install        # python venv + npm deps

# One-time: generate a client cert and get instructions to trust it:
make setup-cert
#   -> scp backend/certs/client.crt root@10.0.0.161:/tmp/
#   -> on the host: incus config trust add-certificate /tmp/client.crt
```

Configuration via environment vars (see `backend/.env.example`):

| Variable           | Default                        |
| ------------------ | ------------------------------ |
| `INCUS_URL`        | `https://10.0.0.161:8443`      |
| `INCUS_CLIENT_CERT`| `backend/certs/client.crt`     |
| `INCUS_CLIENT_KEY` | `backend/certs/client.key`     |
| `INCUS_VERIFY_TLS` | `false`                        |

## Running

Development (Vite dev server on :5173 proxying to Flask on :8555):

```bash
make dev-backend    # terminal 1
make dev-frontend   # terminal 2
```

Production-style (Flask serves the built frontend on :8555):

```bash
make serve
```

## Testing

UI tests use [Playwright](https://playwright.dev) with the Incus API fully
mocked at the network layer, so they run hermetically — no Flask or Incus
needed:

```bash
make test-ui        # runs vite + chromium headless (frontend/tests/e2e/)
```

Tests cover: shell/nav rendering, dashboard cards, instances table (incl.
formatted stats regression), instance actions hitting the right endpoints,
the full create-instance wizard flow, and the console terminal over a mocked
WebSocket.

## Roadmap

- [x] Instances: list, live events, actions, create, console/exec
- [ ] Snapshots, file browser
- [ ] Images management page
- [ ] Storage pools & volumes
- [ ] Networks & ACLs
- [ ] Profiles & projects switcher
- [ ] Metrics dashboard (CPU/RAM/disk graphs from `/1.0/metrics`)
- [ ] Additional providers (libvirt, Docker, raw CLI tooling)
