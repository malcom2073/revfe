from __future__ import annotations

from flask import Blueprint, jsonify

from ..providers import get_provider
from ..providers.base import ProviderError

storage_bp = Blueprint("storage", __name__)


@storage_bp.get("")
def storage_overview():
    try:
        return jsonify(get_provider().storage_overview())
    except ProviderError as exc:
        return jsonify({"error": exc.message}), exc.status_code
