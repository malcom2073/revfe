from __future__ import annotations

import json
import queue
import ssl
import threading
import time
from typing import Any, Iterator
from urllib.parse import urlencode, urlparse

import requests
import websocket

from .base import Image, Instance, ProviderError
from .. import config


class IncusProvider:
    name = "incus"

    TERMINAL_STATUS_CODES = {200, 400, 401}

    REMOTE_CATALOG_URL = "https://images.linuxcontainers.org"
    _remote_catalog_cache: dict[str, Any] = {"fetched_at": None, "images": []}

    def __init__(self) -> None:
        self.base = config.INCUS_URL.rstrip("/")
        self.session = requests.Session()
        self.session.cert = (config.CLIENT_CERT, config.CLIENT_KEY)
        self.session.verify = config.VERIFY_TLS
        self._event_queues: list[queue.Queue] = []
        self._event_lock = threading.Lock()
        self._events_thread: threading.Thread | None = None

    def _ssl_context(self) -> ssl.SSLContext:
        ctx = ssl.create_default_context()
        if not config.VERIFY_TLS:
            ctx.check_hostname = False
            ctx.verify_mode = ssl.CERT_NONE
        # Raw websockets must present the client cert themselves;
        # requests.Session.cert only covers REST calls.
        ctx.load_cert_chain(config.CLIENT_CERT, config.CLIENT_KEY)
        return ctx

    def _request_raw(
        self,
        method: str,
        path: str,
        json_body: dict[str, Any] | None = None,
        params: dict[str, Any] | None = None,
    ) -> dict[str, Any]:
        url = f"{self.base}{path}"
        try:
            resp = self.session.request(
                method, url, json=json_body, params=params, timeout=60
            )
        except requests.RequestException as exc:
            raise ProviderError(f"Cannot reach Incus at {self.base}: {exc}", 502) from exc
        try:
            body = resp.json()
        except ValueError as exc:
            raise ProviderError(
                f"Incus returned non-JSON response ({resp.status_code})", 502
            ) from exc

        if resp.status_code == 403:
            raise ProviderError(
                "Incus rejected our certificate. Run 'make setup-cert' and trust "
                "the generated client certificate on the Incus host.",
                502,
            )
        if body.get("type") == "error":
            raise ProviderError(body.get("error", "Unknown Incus error"), 502)
        return body

    def _request(
        self,
        method: str,
        path: str,
        json_body: dict[str, Any] | None = None,
        params: dict[str, Any] | None = None,
        wait: bool = False,
    ) -> dict[str, Any]:
        body = self._request_raw(method, path, json_body, params)
        rtype = body.get("type")
        if rtype == "async" and body.get("operation"):
            op_url = body["operation"]
            if wait:
                return self.wait_operation(op_url)
            return {"operation": op_url.split("/1.0/")[-1], "running": True}
        return body.get("metadata", {})

    def wait_operation(self, op_url: str, timeout: int = 300) -> dict[str, Any]:
        url = f"{self.base}{op_url}/wait?timeout={timeout}"
        try:
            resp = self.session.get(url, timeout=timeout + 10)
        except requests.RequestException as exc:
            raise ProviderError(f"Operation wait failed: {exc}", 502) from exc
        body = resp.json()
        if body.get("type") == "error":
            raise ProviderError(body.get("error", "Operation failed"), 502)
        metadata = body.get("metadata", {})
        if isinstance(metadata, dict) and metadata.get("status_code") not in (
            None,
            200,
            103,
        ):
            raise ProviderError(
                f"Operation failed: {metadata.get('status', 'unknown')}", 502
            )
        return metadata

    def server_info(self) -> dict[str, Any]:
        meta = self._request("GET", "/1.0")
        env = meta.get("environment", {})
        auth = meta.get("auth", "trusted")
        info = {
            "name": env.get("server_name"),
            "version": env.get("server_version"),
            "os": env.get("distribution", "Linux"),
            "kernel": env.get("kernel_version"),
            "architectures": env.get("architectures", []),
            "cpu": env.get("cpu"),
            "memory_total": env.get("memory_total"),
            "project": meta.get("project", "default"),
            "auth": auth,
        }
        if auth == "untrusted":
            info["message"] = (
                "Incus does not trust this client certificate yet. "
                "Run 'make setup-cert' and follow the printed steps."
            )
        return info

    def list_instances(self) -> list[Instance]:
        metas = self._request("GET", "/1.0/instances", params={"recursion": 2})
        return [self._to_instance(m) for m in metas]

    def get_instance(self, name: str) -> Instance:
        meta = self._request("GET", f"/1.0/instances/{name}")
        state = self._request("GET", f"/1.0/instances/{name}/state")
        inst = self._to_instance(meta)
        inst.state = self._normalize_state(state)
        return inst

    def _to_instance(self, meta: dict[str, Any]) -> Instance:
        return Instance(
            name=meta.get("name", ""),
            status=meta.get("status", "Unknown"),
            status_code=meta.get("status_code", 0),
            type=meta.get("type", "container"),
            profiles=meta.get("profiles", []),
            config=meta.get("config", {}),
            devices=meta.get("devices", {}),
            created_at=meta.get("created_at", ""),
            raw=meta,
        )

    @staticmethod
    def _normalize_state(state: dict[str, Any]) -> dict[str, Any]:
        cpu = state.get("cpu") or {}
        memory = state.get("memory") or {}
        disks = [
            {
                "name": name,
                "usage": info.get("usage"),
                "total": info.get("total"),
            }
            for name, info in (state.get("disk") or {}).items()
        ]
        interfaces = []
        for if_name, net in (state.get("network") or {}).items():
            addresses = [
                {
                    "address": a.get("address"),
                    "family": a.get("family"),
                    "netmask": a.get("netmask"),
                    "scope": a.get("scope"),
                }
                for a in net.get("addresses") or []
                if a.get("address")
            ]
            interfaces.append(
                {
                    "name": if_name,
                    "addresses": addresses,
                }
            )
        processes = state.get("processes")
        if isinstance(processes, dict):
            processes = processes.get("total")
        return {
            "pid": state.get("pid"),
            "processes": processes,
            "memory_used": memory.get("usage"),
            "memory_usage_peak": memory.get("usage_peak"),
            "cpu_seconds": cpu.get("usage"),
            "disks": disks,
            "interfaces": interfaces,
        }

    def instance_action(self, name: str, action: str) -> dict[str, Any]:
        if action not in {"start", "stop", "restart", "freeze", "unfreeze"}:
            raise ProviderError(f"Unsupported action: {action}", 400)
        return self._request(
            "PUT",
            f"/1.0/instances/{name}/state",
            json_body={"action": action, "timeout": 120},
            wait=True,
        )

    def delete_instance(self, name: str) -> dict[str, Any]:
        return self._request("DELETE", f"/1.0/instances/{name}", wait=True)

    def create_instance(self, spec: dict[str, Any]) -> dict[str, Any]:
        name = spec.get("name")
        if not name:
            raise ProviderError("'name' is required", 400)
        image_ref = spec.get("image", "ubuntu:24.04")
        config: dict[str, Any] = dict(spec.get("config") or {})
        devices: dict[str, Any] = {}

        limits = spec.get("limits") or {}
        if limits.get("cpu"):
            config["limits.cpu"] = str(limits["cpu"])
        memory = limits.get("memory")
        if memory:
            mem = str(memory)
            config["limits.memory"] = f"{mem}MiB" if mem.isdigit() else mem
        if limits.get("disk_gb") or spec.get("disk_pool"):
            device: dict[str, Any] = {
                "path": "/",
                "type": "disk",
                "pool": spec.get("disk_pool") or "default",
            }
            if limits.get("disk_gb"):
                device["size"] = f"{limits['disk_gb']}GiB"
            devices["root"] = device
        network = spec.get("network")
        if network:
            devices["eth0"] = {"type": "nic", "network": network}

        body: dict[str, Any] = {
            "name": name,
            "source": self._image_source(image_ref),
            "type": spec.get("type", "container"),
            "profiles": spec.get("profiles") or ["default"],
            "config": config,
            "devices": {**devices, **(spec.get("devices") or {})},
        }
        return self._request("POST", "/1.0/instances", json_body=body, wait=True)

    # Remote prefix -> (protocol, server, alias-prefix).
    # NOTE: we deliberately route ubuntu:* through images.linuxcontainers.org
    # instead of cloud-images.ubuntu.com — some daemons fail simplestreams
    # resolution against the latter ("requested image couldn't be found"),
    # while the linuxcontainers mirror carries the same Ubuntu images and works.
    _REMOTES = {
        "ubuntu:": ("simplestreams", "https://images.linuxcontainers.org", "ubuntu/"),
        "images:": ("simplestreams", "https://images.linuxcontainers.org", ""),
    }

    @classmethod
    def _image_source(cls, ref: str) -> dict[str, Any]:
        cleaned = ref.strip()
        if len(cleaned) >= 12 and all(
            c in "0123456789abcdef" for c in cleaned.lower()
        ):
            return {"type": "image", "fingerprint": cleaned}
        for prefix, (protocol, server, alias_prefix) in cls._REMOTES.items():
            if cleaned.startswith(prefix):
                return {
                    "type": "image",
                    "protocol": protocol,
                    "server": server,
                    "alias": f"{alias_prefix}{cleaned[len(prefix):]}",
                }
        return {"type": "image", "alias": cleaned}

    def list_images(self) -> list[Image]:
        metas = self._request("GET", "/1.0/images", params={"recursion": 1})
        images = []
        for m in metas:
            props = m.get("properties") or {}
            aliases = [a.get("name", "") for a in m.get("aliases") or []]
            os_name = props.get("os", "")
            release = props.get("release", "")
            description = m.get("description") or ""
            if not description:
                parts = [p for p in (os_name, release) if p]
                description = " ".join(parts)
            images.append(
                Image(
                    fingerprint=m.get("fingerprint", ""),
                    public=m.get("public", False),
                    description=description,
                    os=os_name,
                    release=release,
                    size_bytes=int(m.get("size", 0)),
                    aliases=[a for a in aliases if a],
                    uploaded_at=m.get("uploaded_at", ""),
                    instance_type=m.get("type", "") or "",
                )
            )
        return images

    def _wait_for_exec_secrets(
        self, operation: str, timeout: float = 15.0
    ) -> dict[str, str] | None:
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            try:
                body = self._request_raw("GET", operation)
            except ProviderError as exc:
                raise ProviderError(f"Exec failed to start: {exc.message}", 502)
            metadata = body.get("metadata") or {}
            inner = metadata.get("metadata") if isinstance(metadata, dict) else None
            fds = (inner or metadata).get("fds")
            if fds:
                return fds
            time.sleep(0.2)
        return None

    def delete_image(self, fingerprint: str) -> dict[str, Any]:
        return self._request(
            "DELETE", f"/1.0/images/{fingerprint}", wait=True
        )

    def remote_images(self) -> dict[str, Any]:
        return self._remote_catalog_cache

    def refresh_remote_images(self) -> dict[str, Any]:
        base = self.REMOTE_CATALOG_URL
        idx = requests.get(f"{base}/streams/v1/index.json", timeout=30)
        idx.raise_for_status()
        index = idx.json()
        download_entry = next(
            (
                v
                for v in index.get("index", {}).values()
                if v.get("datatype") == "image-downloads"
            ),
            None,
        )
        if not download_entry:
            raise ProviderError("Remote catalog has no image-downloads stream", 502)
        doc = requests.get(
            f"{base}/{download_entry['path']}", timeout=60
        )
        doc.raise_for_status()
        products = doc.json().get("products", {})

        images: list[dict[str, Any]] = []
        for p in products.values():
            os_name = p.get("os", "")
            release = p.get("release", "")
            versions = p.get("versions") or {}
            if not (os_name and release and versions):
                continue
            latest = sorted(versions)[-1]
            items = (versions.get(latest) or {}).get("items") or {}
            sizes: dict[str, int] = {}
            for item in items.values():
                ftype = item.get("ftype")
                size = int(item.get("size", 0))
                if ftype == "squashfs":
                    sizes["container"] = max(sizes.get("container", 0), size)
                elif ftype in ("disk-kvm.img", "disk1.img"):
                    sizes["virtual-machine"] = max(
                        sizes.get("virtual-machine", 0), size
                    )
            if not sizes:
                continue
            aliases = [
                a.strip() for a in (p.get("aliases") or "").split(",") if a.strip()
            ]
            images.append(
                {
                    "ref": aliases[0] if aliases else f"{os_name.lower()}/{release}",
                    "os": os_name,
                    "release": release,
                    "title": p.get("release_title") or release,
                    "version": p.get("version", ""),
                    "arch": p.get("arch", ""),
                    "variant": p.get("variant", ""),
                    "aliases": aliases[:4],
                    "sizes": sizes,
                    "published": latest,
                }
            )
        images.sort(key=lambda i: (i["os"].lower(), i["release"], i["arch"]))
        self._remote_catalog_cache["images"] = images
        self._remote_catalog_cache["fetched_at"] = time.strftime(
            "%Y-%m-%dT%H:%M:%SZ", time.gmtime()
        )
        return self._remote_catalog_cache

    def pull_image(self, ref: str) -> dict[str, Any]:
        source = self._image_source(ref.strip())
        if "server" not in source:
            raise ProviderError(
                "Pull requires a remote image reference "
                "(e.g. images:debian/13 or ubuntu:24.04)",
                400,
            )
        return self._request("POST", "/1.0/images", json_body={"source": source})

    def running_operations(self) -> list[dict[str, Any]]:
        metas = self._request("GET", "/1.0/operations", params={"recursion": 1})
        ops: list[dict[str, Any]] = []
        for group in (metas or {}).values() if isinstance(metas, dict) else []:
            for op in group or []:
                if op.get("class") == "task" and op.get("status_code") not in (
                    200,
                    400,
                    401,
                ):
                    ops.append(
                        {
                            "id": op.get("id"),
                            "description": op.get("description"),
                            "status": op.get("status"),
                            "statusCode": op.get("status_code"),
                            "metadata": op.get("metadata") or {},
                        }
                    )
        return ops

    def list_profiles(self) -> list[dict[str, Any]]:
        metas = self._request("GET", "/1.0/profiles", params={"recursion": 1})
        profiles = []
        for m in metas:
            used_by = []
            for ref in m.get("used_by") or []:
                parts = [p for p in ref.split("/") if p]
                if len(parts) >= 3 and parts[0] == "1.0":
                    used_by.append({"kind": parts[1].rstrip("s"), "name": parts[2]})
                else:
                    used_by.append({"kind": "other", "name": ref})
            profiles.append(
                {
                    "name": m.get("name", ""),
                    "description": m.get("description") or "",
                    "config": m.get("config") or {},
                    "devices": m.get("devices") or {},
                    "usedBy": used_by,
                }
            )
        return profiles

    def list_storage_pools(self) -> list[dict[str, Any]]:
        metas = self._request("GET", "/1.0/storage-pools", params={"recursion": 1})
        return [
            {
                "name": m.get("name"),
                "driver": m.get("driver"),
                "description": m.get("description") or "",
                "status": m.get("status"),
            }
            for m in metas
        ]

    def list_networks(self) -> list[dict[str, Any]]:
        metas = self._request("GET", "/1.0/networks", params={"recursion": 1})
        networks = []
        for m in metas:
            if m.get("type") == "loopback":
                continue
            name = m.get("name", "")
            config = m.get("config") or {}
            used_by = []
            for ref in m.get("used_by") or []:
                # "/1.0/instances/foo" -> ("instance", "foo")
                parts = [p for p in ref.split("/") if p]
                if len(parts) >= 3 and parts[0] == "1.0":
                    used_by.append({"kind": parts[1].rstrip("s"), "name": parts[2]})
                else:
                    used_by.append({"kind": "other", "name": ref})
            networks.append(
                {
                    "name": name,
                    "type": m.get("type", ""),
                    "description": m.get("description") or "",
                    "managed": m.get("managed", False),
                    "status": m.get("status", ""),
                    "ipv4": config.get("ipv4.address"),
                    "ipv6": config.get("ipv6.address"),
                    "ipv4Nat": config.get("ipv4.nat") == "true",
                    "ipv6Nat": config.get("ipv6.nat") == "true",
                    "usedBy": used_by,
                }
            )
        return networks

    def storage_overview(self) -> list[dict[str, Any]]:
        pools = self._request("GET", "/1.0/storage-pools", params={"recursion": 1})
        try:
            images = {img.fingerprint: img for img in self.list_images()}
        except ProviderError:
            images = {}

        def resolve_image(volume_name: str) -> str | None:
            for fp, img in images.items():
                if fp.startswith(volume_name) or volume_name.startswith(fp):
                    return img.description or img.fingerprint[:12]
            return None

        overview = []
        for p in pools:
            pool_name = p.get("name", "")
            volumes_meta = self._request(
                "GET",
                f"/1.0/storage-pools/{pool_name}/volumes",
                params={"recursion": 1},
            )
            volumes = []
            for v in volumes_meta or []:
                vtype = v.get("type", "")
                entry = {
                    "name": v.get("name", ""),
                    "type": vtype,
                    "contentType": v.get("content_type", ""),
                }
                if vtype == "image":
                    entry["imageDescription"] = resolve_image(entry["name"])
                volumes.append(entry)
            usage = None
            try:
                resources = self._request(
                    "GET", f"/1.0/storage-pools/{pool_name}/resources"
                )
                space = resources.get("space") or {}
                if space.get("total"):
                    usage = {
                        "used": space.get("used"),
                        "total": space.get("total"),
                    }
            except ProviderError:
                pass  # older daemons without the resources endpoint
            overview.append(
                {
                    "name": pool_name,
                    "driver": p.get("driver", ""),
                    "description": p.get("description") or "",
                    "status": p.get("status", ""),
                    "usedByCount": len(p.get("used_by") or []),
                    "usage": usage,
                    "volumes": volumes,
                }
            )
        return overview

    def list_snapshots(self, instance: str) -> list[dict[str, Any]]:
        metas = self._request(
            "GET", f"/1.0/instances/{instance}/snapshots", params={"recursion": 1}
        )
        snapshots = []
        for m in metas or []:
            full_name = m.get("name", "")
            snap_name = full_name.split("/")[-1]
            expires = m.get("expires_at") or ""
            snapshots.append(
                {
                    "name": snap_name,
                    "createdAt": m.get("created_at", ""),
                    "stateful": bool(m.get("stateful", False)),
                    "expiresAt": expires if not expires.startswith("0001-") else None,
                }
            )
        return snapshots

    def create_snapshot(
        self, instance: str, name: str, stateful: bool = False
    ) -> dict[str, Any]:
        if not name or not all(c.isalnum() or c in "-_." for c in name):
            raise ProviderError(
                "Snapshot names may only contain alphanumerics, dashes, "
                "underscores and dots",
                400,
            )
        return self._request(
            "POST",
            f"/1.0/instances/{instance}/snapshots",
            json_body={"name": name, "stateful": stateful},
            wait=True,
        )

    def restore_snapshot(self, instance: str, snapshot: str) -> dict[str, Any]:
        # NOTE: restore goes through the instance-update endpoint (PUT).
        # POSTing {"restore": ...} hits the rename/migrate handler instead,
        # which fails with a confusing name-validation error.
        return self._request(
            "PUT",
            f"/1.0/instances/{instance}",
            json_body={"restore": snapshot, "stateful": False},
            wait=True,
        )

    def delete_snapshot(self, instance: str, snapshot: str) -> dict[str, Any]:
        return self._request(
            "DELETE",
            f"/1.0/instances/{instance}/snapshots/{snapshot}",
            wait=True,
        )

    def exec_bridge(self, name: str, browser_ws, shell: str) -> None:
        payload = {
            "command": [shell],
            "environment": {"TERM": "xterm-256color"},
            "interactive": True,
            "wait-for-websocket": True,
        }
        result = self._request_raw(
            "POST", f"/1.0/instances/{name}/exec", json_body=payload
        )
        operation = result.get("operation", "")
        fds = self._wait_for_exec_secrets(operation)
        if not fds or "0" not in fds:
            raise ProviderError("Exec did not return websocket secrets", 502)

        def ws_url(secret: str) -> str:
            parsed = urlparse(self.base)
            scheme = "wss" if parsed.scheme == "https" else "ws"
            query = urlencode({"secret": secret})
            return f"{scheme}://{parsed.netloc}{operation}/websocket?{query}"

        ctrl_sock = websocket.create_connection(
            ws_url(fds.get("control", fds["0"])),
            sslopt={"context": self._ssl_context()},
        )
        term_sock = websocket.create_connection(
            ws_url(fds["0"]), sslopt={"context": self._ssl_context()}
        )

        stop = threading.Event()

        def pump_incus_to_browser() -> None:
            try:
                while not stop.is_set():
                    data = term_sock.recv()
                    if data is None or data == "":
                        break
                    browser_ws.send(data)
            except Exception:
                pass
            finally:
                stop.set()
                try:
                    browser_ws.close()
                except Exception:
                    pass

        threads = [
            threading.Thread(target=pump_incus_to_browser, daemon=True),
            threading.Thread(target=self._pump_control, args=(ctrl_sock,), daemon=True),
        ]
        for t in threads:
            t.start()

        try:
            while not stop.is_set():
                message = browser_ws.receive(timeout=1)
                if message is None:
                    continue
                if isinstance(message, str):
                    try:
                        parsed = json.loads(message)
                    except ValueError:
                        continue
                    if "resize" in parsed:
                        cols = parsed["resize"].get("cols", 80)
                        rows = parsed["resize"].get("rows", 24)
                        ctrl_sock.send(
                            json.dumps(
                                {
                                    "command": "signal",
                                    "signal": "SIGWINCH",
                                    "args": [str(rows), str(cols)],
                                }
                            )
                        )
                elif isinstance(message, bytes):
                    term_sock.send_binary(message)
        except Exception:
            pass
        finally:
            stop.set()
            for sock in (term_sock, ctrl_sock):
                try:
                    sock.close()
                except Exception:
                    pass

    @staticmethod
    def _pump_control(ctrl_sock) -> None:
        try:
            while True:
                if ctrl_sock.recv() is None:
                    break
        except Exception:
            pass

    def subscribe_events(self) -> Iterator[dict[str, Any]]:
        q: queue.Queue = queue.Queue()
        with self._event_lock:
            self._event_queues.append(q)
            if self._events_thread is None or not self._events_thread.is_alive():
                self._events_thread = threading.Thread(
                    target=self._consume_events, daemon=True
                )
                self._events_thread.start()
        try:
            while True:
                yield q.get()
        finally:
            with self._event_lock:
                if q in self._event_queues:
                    self._event_queues.remove(q)

    def _consume_events(self) -> None:
        while True:
            try:
                parsed = urlparse(self.base)
                ws = websocket.create_connection(
                    f"wss://{parsed.netloc}/1.0/events",
                    sslopt={"context": self._ssl_context()},
                )
                while True:
                    raw = ws.recv()
                    if not raw:
                        break
                    event = json.loads(raw)
                    etype = event.get("type", "")
                    if etype.startswith(("instance", "operation", "lifecycle")):
                        with self._event_lock:
                            subscribers = list(self._event_queues)
                        for q in subscribers:
                            q.put(event)
            except Exception:
                time.sleep(2)
