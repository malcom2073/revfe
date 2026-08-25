from __future__ import annotations

import re
import threading
import time
from collections import deque
from typing import Any

SAMPLE_INTERVAL = 5.0
HISTORY_POINTS = 240  # 20 minutes at 5s intervals

_SAMPLE_RE = re.compile(r"^(incus_[a-zA-Z_]+)(?:\{([^}]*)\})?\s+([0-9eE+.+-]+)$")
_LABEL_RE = re.compile(r'(\w+)="([^"]*)"')


def _parse_labels(label_str: str) -> dict[str, str]:
    return dict(_LABEL_RE.findall(label_str))


def parse_prometheus(text: str) -> list[tuple[str, dict[str, str], float]]:
    samples = []
    for line in text.splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        m = _SAMPLE_RE.match(line)
        if not m:
            continue
        metric, label_str, value = m.groups()
        try:
            samples.append((metric, _parse_labels(label_str or ""), float(value)))
        except ValueError:
            continue
    return samples


def summarize(samples: list[tuple[str, dict[str, str], float]]) -> dict[str, Any]:
    instances: dict[str, dict[str, Any]] = {}

    def inst(name: str) -> dict[str, Any]:
        return instances.setdefault(name, {})

    for metric, labels, value in samples:
        name = labels.get("name")
        if not name:
            continue
        entry = inst(name)
        if metric == "incus_cpu_seconds_total":
            # Exclude idle/iowait — otherwise an idle VM's "busy doing
            # nothing" time graphs as 100%+ CPU usage.
            if labels.get("mode") in ("idle", "iowait"):
                continue
            entry["cpuSeconds"] = entry.get("cpuSeconds", 0.0) + value
        elif metric == "incus_memory_MemTotal_bytes":
            entry["memTotal"] = value
        elif metric == "incus_memory_MemAvailable_bytes":
            entry["memAvailable"] = value
        elif metric == "incus_memory_usage_bytes":
            entry["memoryUsed"] = value
        elif metric == "incus_network_receive_bytes_total":
            entry["netRx"] = entry.get("netRx", 0.0) + value
        elif metric == "incus_network_transmit_bytes_total":
            entry["netTx"] = entry.get("netTx", 0.0) + value
        elif metric == "incus_disk_read_bytes_total":
            entry["diskRead"] = entry.get("diskRead", 0.0) + value
        elif metric == "incus_disk_written_bytes_total":
            entry["diskWrite"] = entry.get("diskWrite", 0.0) + value
        elif metric == "incus_procs_total":
            entry["processes"] = value
        elif metric == "incus_filesystem_size_bytes" and labels.get("mountpoint") == "/":
            prev = entry.get("fsSize")
            entry["fsSize"] = value if prev is None else max(prev, value)
        elif (
            metric == "incus_filesystem_used_bytes"
            and labels.get("mountpoint") == "/"
        ):
            prev = entry.get("fsUsed")
            entry["fsUsed"] = value if prev is None else max(prev, value)

    # Containers report MemTotal/MemAvailable instead of usage
    for entry in instances.values():
        if "memoryUsed" not in entry and "memTotal" in entry and "memAvailable" in entry:
            entry["memoryUsed"] = entry["memTotal"] - entry["memAvailable"]
    return {"instances": instances}


class MetricsSampler:
    def __init__(self) -> None:
        self._buffer: deque[dict[str, Any]] = deque(maxlen=HISTORY_POINTS)
        self._lock = threading.Lock()
        self._thread: threading.Thread | None = None

    def ensure_started(self) -> None:
        with self._lock:
            if self._thread is None or not self._thread.is_alive():
                self._thread = threading.Thread(
                    target=self._run, name="metrics-sampler", daemon=True
                )
                self._thread.start()

    def _fetch(self) -> dict[str, Any] | None:
        from .providers import get_provider

        provider = get_provider()
        session = getattr(provider, "session", None)
        base = getattr(provider, "base", None)
        if session is None or base is None:
            return None
        resp = session.get(f"{base}/1.0/metrics", timeout=10)
        if resp.status_code != 200:
            return None
        point = summarize(parse_prometheus(resp.text))
        point["timestamp"] = time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())
        point["epoch"] = time.time()
        return point

    def sample_once(self) -> bool:
        try:
            point = self._fetch()
        except Exception:
            return False
        if point is None:
            return False
        with self._lock:
            self._buffer.append(point)
        return True

    def _run(self) -> None:
        while True:
            self.sample_once()
            time.sleep(SAMPLE_INTERVAL)

    def history(self, window_seconds: int | None = None) -> dict[str, Any]:
        with self._lock:
            points = list(self._buffer)
        cutoff = time.time() - window_seconds if window_seconds else None
        if cutoff is not None:
            points = [p for p in points if p.get("epoch", 0) >= cutoff]
        names: set[str] = set()
        for p in points:
            names.update((p.get("instances") or {}).keys())
        return {
            "interval": SAMPLE_INTERVAL,
            "instances": sorted(names),
            "points": [
                {k: v for k, v in p.items() if k != "epoch"} for p in points
            ],
        }


sampler = MetricsSampler()
