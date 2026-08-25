from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any, Iterator, Protocol


@dataclass
class Instance:
    name: str
    status: str
    status_code: int
    type: str
    profiles: list[str]
    config: dict[str, Any] = field(default_factory=dict)
    devices: dict[str, Any] = field(default_factory=dict)
    state: dict[str, Any] = field(default_factory=dict)
    created_at: str = ""
    raw: dict[str, Any] = field(default_factory=dict)


@dataclass
class Image:
    fingerprint: str
    public: bool
    description: str
    os: str
    release: str
    size_bytes: int
    aliases: list[str] = field(default_factory=list)
    uploaded_at: str = ""
    instance_type: str = ""


@dataclass
class ProviderError(Exception):
    message: str
    status_code: int = 500

    def __init__(self, message: str, status_code: int = 500):
        super().__init__(message)
        self.message = message
        self.status_code = status_code


class Provider(Protocol):
    name: str

    def server_info(self) -> dict[str, Any]: ...

    def list_instances(self) -> list[Instance]: ...

    def get_instance(self, name: str) -> Instance: ...

    def instance_action(self, name: str, action: str) -> dict[str, Any]: ...

    def delete_instance(self, name: str) -> dict[str, Any]: ...

    def create_instance(self, spec: dict[str, Any]) -> dict[str, Any]: ...

    def list_images(self) -> list[Image]: ...

    def delete_image(self, fingerprint: str) -> dict[str, Any]: ...

    def pull_image(self, ref: str) -> dict[str, Any]: ...

    def running_operations(self) -> list[dict[str, Any]]: ...

    def remote_images(self) -> dict[str, Any]: ...

    def refresh_remote_images(self) -> dict[str, Any]: ...

    def list_snapshots(self, instance: str) -> list[dict[str, Any]]: ...

    def create_snapshot(
        self, instance: str, name: str, stateful: bool = False
    ) -> dict[str, Any]: ...

    def restore_snapshot(self, instance: str, snapshot: str) -> dict[str, Any]: ...

    def delete_snapshot(self, instance: str, snapshot: str) -> dict[str, Any]: ...

    def list_profiles(self) -> list[str]: ...

    def list_storage_pools(self) -> list[dict[str, Any]]: ...

    def list_networks(self) -> list[dict[str, Any]]: ...


    def exec_bridge(self, name: str, browser_ws: Any, shell: str) -> None: ...

    def subscribe_events(self) -> Iterator[dict[str, Any]]: ...
