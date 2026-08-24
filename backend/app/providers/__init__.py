from __future__ import annotations

import threading

from .base import Image, Instance, Provider, ProviderError
from .incus import IncusProvider

_providers: dict[str, Provider] = {}
_lock = threading.Lock()

DEFAULT_PROVIDER = "incus"


def _create(name: str) -> Provider:
    if name != "incus":
        raise ProviderError(f"Unknown backend provider: {name}", 400)
    return IncusProvider()


def get_provider(name: str | None = None) -> Provider:
    key = name or DEFAULT_PROVIDER
    with _lock:
        if key not in _providers:
            _providers[key] = _create(key)
        return _providers[key]
