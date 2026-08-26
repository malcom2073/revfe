from __future__ import annotations

from flask import Blueprint, jsonify, request

from ..providers import get_provider
from ..providers.base import ProviderError

system_bp = Blueprint("system", __name__)


@system_bp.get("/server")
def server_info():
    provider = get_provider()
    info = provider.server_info()
    info["provider"] = provider.name
    return jsonify(info)


@system_bp.get("/images")
def list_images():
    provider = get_provider()
    return jsonify(
        [
            {
                "fingerprint": img.fingerprint,
                "shortFingerprint": img.fingerprint[:12],
                "description": img.description,
                "os": img.os,
                "release": img.release,
                "sizeBytes": img.size_bytes,
                "aliases": img.aliases,
                "uploadedAt": img.uploaded_at,
                "instanceType": img.instance_type or None,
            }
            for img in provider.list_images()
        ]
    )


@system_bp.post("/images/pull")
def pull_image():
    provider = get_provider()
    body = request.get_json(force=True) or {}
    ref = body.get("image", "")
    if not ref:
        return jsonify({"error": "'image' reference is required"}), 400
    result = provider.pull_image(ref)
    return jsonify({"ok": True, "operation": result.get("operation")}), 202


@system_bp.get("/remote-images")
def remote_images():
    cache = get_provider().remote_images()
    return jsonify(_catalog_payload(cache))


@system_bp.post("/remote-images/refresh")
def refresh_remote_images():
    try:
        cache = get_provider().refresh_remote_images()
    except ProviderError as exc:
        return jsonify({"error": exc.message}), exc.status_code
    except Exception as exc:  # upstream fetch failures
        return jsonify({"error": f"Catalog refresh failed: {exc}"}), 502
    return jsonify(_catalog_payload(cache))


def _catalog_payload(cache: dict) -> dict:
    return {
        "fetchedAt": cache.get("fetched_at"),
        "images": [
            {
                "ref": img["ref"],
                "os": img["os"],
                "release": img["release"],
                "title": img["title"],
                "version": img["version"],
                "arch": img["arch"],
                "variant": img["variant"],
                "aliases": img["aliases"],
                "sizes": img["sizes"],
                "published": img["published"],
            }
            for img in cache.get("images", [])
        ],
    }


@system_bp.get("/operations")
def running_operations():
    return jsonify(get_provider().running_operations())


@system_bp.delete("/images/<fingerprint>")
def delete_image(fingerprint: str):
    provider = get_provider()
    metadata = provider.delete_image(fingerprint)
    return jsonify({"ok": True, "metadata": metadata})


@system_bp.get("/profiles")
def list_profiles():
    return jsonify(get_provider().list_profiles())


@system_bp.post("/profiles")
def create_profile():
    provider = get_provider()
    spec = request.get_json(force=True) or {}
    result = provider.create_profile(spec)
    name = (spec.get("name") or "").strip()
    return jsonify({"ok": True, "name": name, "metadata": result}), 201


@system_bp.put("/profiles/<name>")
def update_profile(name: str):
    provider = get_provider()
    spec = request.get_json(force=True) or {}
    result = provider.update_profile(name, spec)
    return jsonify({"ok": True, "metadata": result})


@system_bp.delete("/profiles/<name>")
def delete_profile(name: str):
    provider = get_provider()
    result = provider.delete_profile(name)
    return jsonify({"ok": True, "metadata": result})


@system_bp.get("/storage-pools")
def list_storage_pools():
    return jsonify(get_provider().list_storage_pools())


@system_bp.get("/networks")
def list_networks():
    return jsonify(get_provider().list_networks())


@system_bp.get("/health")
def health():
    try:
        get_provider().server_info()
        return jsonify({"ok": True})
    except ProviderError as exc:
        return jsonify({"ok": False, "error": exc.message}), exc.status_code
