from __future__ import annotations

from flask import Blueprint, jsonify, request

from ..metrics import sampler

metrics_bp = Blueprint("metrics", __name__)


@metrics_bp.get("/history")
def metrics_history():
    sampler.ensure_started()
    try:
        window = int(request.args.get("window", 0)) or None
    except ValueError:
        window = None
    return jsonify(sampler.history(window))
