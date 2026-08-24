from __future__ import annotations

import json

from flask import Blueprint, Response

from ..providers import get_provider
from ..providers.base import ProviderError

events_bp = Blueprint("events", __name__)


@events_bp.get("")
def events_stream():
    provider = get_provider()
    try:
        iterator = provider.subscribe_events()
    except ProviderError as exc:
        return jsonify({"error": exc.message}), exc.status_code

    def generate():
        yield ": connected\n\n"
        for event in iterator:
            data = json.dumps(
                {
                    "type": event.get("type"),
                    "timestamp": event.get("timestamp"),
                    "metadata": event.get("metadata"),
                }
            )
            yield f"data: {data}\n\n"

    return Response(generate(), mimetype="text/event-stream")
