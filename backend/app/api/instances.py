from __future__ import annotations

import time

from flask import Blueprint, jsonify, request

from ..providers import get_provider
from ..providers.base import ProviderError

instances_bp = Blueprint("instances", __name__, url_prefix="/instances")


@instances_bp.get("")
def list_instances():
    provider = get_provider()
    result = [
        {
            "name": i.name,
            "status": i.status,
            "statusCode": i.status_code,
            "type": i.type,
            "profiles": i.profiles,
            "createdAt": i.created_at,
            "state": i.state or None,
        }
        for i in provider.list_instances()
    ]
    return jsonify(result)


@instances_bp.get("/<name>")
def get_instance(name: str):
    provider = get_provider()
    inst = provider.get_instance(name)
    state = inst.state or {}
    return jsonify(
        {
            "name": inst.name,
            "status": inst.status,
            "statusCode": inst.status_code,
            "type": inst.type,
            "profiles": inst.profiles,
            "config": inst.config,
            "devices": inst.devices,
            "createdAt": inst.created_at,
            "state": {
                "pid": state.get("pid"),
                "processes": state.get("processes"),
                "memoryUsed": state.get("memory_used"),
                "memoryUsagePeak": state.get("memory_usage_peak"),
                "cpuSeconds": state.get("cpu_seconds"),
                "interfaces": state.get("interfaces") or [],
                "disks": state.get("disks") or [],
            },
        }
    )


@instances_bp.patch("/<name>")
def update_instance(name: str):
    provider = get_provider()
    spec = request.get_json(force=True) or {}
    metadata = provider.update_instance(name, spec)
    return jsonify({"ok": True, "metadata": metadata})


@instances_bp.post("/<name>/rename")
def rename_instance(name: str):
    provider = get_provider()
    body = request.get_json(force=True) or {}
    new_name = (body.get("name") or "").strip()
    metadata = provider.rename_instance(name, new_name)
    return jsonify({"ok": True, "name": new_name, "metadata": metadata})


@instances_bp.post("/<name>/<action>")
def instance_action(name: str, action: str):
    if action == "delete":
        return delete_instance(name)
    provider = get_provider()
    metadata = provider.instance_action(name, action)
    return jsonify({"ok": True, "metadata": metadata})


def delete_instance(name: str):
    provider = get_provider()
    metadata = provider.delete_instance(name)
    return jsonify({"ok": True, "metadata": metadata})


@instances_bp.post("")
def create_instance():
    provider = get_provider()
    spec = request.get_json(force=True) or {}
    metadata = provider.create_instance(spec)
    return jsonify({"ok": True, "metadata": metadata}), 201


@instances_bp.get("/<name>/snapshots")
def list_snapshots(name: str):
    return jsonify(get_provider().list_snapshots(name))


@instances_bp.post("/<name>/snapshots")
def create_snapshot(name: str):
    provider = get_provider()
    body = request.get_json(force=True) or {}
    snap_name = body.get("name") or f"snap{time.strftime('%Y%m%d%H%M%S')}"
    metadata = provider.create_snapshot(
        name, snap_name, stateful=bool(body.get("stateful", False))
    )
    return jsonify({"ok": True, "name": snap_name, "metadata": metadata}), 201


@instances_bp.post("/<name>/snapshots/<snapshot>/restore")
def restore_snapshot(name: str, snapshot: str):
    provider = get_provider()
    metadata = provider.restore_snapshot(name, snapshot)
    return jsonify({"ok": True, "metadata": metadata})


@instances_bp.post("/<name>/snapshots/<snapshot>/delete")
def delete_snapshot(name: str, snapshot: str):
    provider = get_provider()
    metadata = provider.delete_snapshot(name, snapshot)
    return jsonify({"ok": True, "metadata": metadata})


def instance_exec(ws, name: str):
    shell = request.args.get("shell", "bash")
    provider = get_provider()
    try:
        provider.exec_bridge(name, ws, shell)
    except ProviderError as exc:
        ws.send(f"\r\n[RevFe] {exc.message}\r\n")
        ws.close()
