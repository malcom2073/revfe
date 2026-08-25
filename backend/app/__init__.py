from __future__ import annotations

from flask import Flask, jsonify, send_from_directory
from flask_cors import CORS
from flask_sock import Sock

from . import config
from .providers.base import ProviderError

sock = Sock()


def create_app() -> Flask:
    app = Flask(__name__, static_folder=None)
    CORS(app)
    sock.init_app(app)

    from .api.events import events_bp
    from .api.instances import instance_exec, instances_bp
    from .api.metrics import metrics_bp
    from .api.storage import storage_bp
    from .api.system import system_bp

    prefix = config.API_PREFIX
    app.register_blueprint(system_bp, url_prefix=f"{prefix}")
    app.register_blueprint(instances_bp, url_prefix=f"{prefix}/instances")
    app.register_blueprint(events_bp, url_prefix=f"{prefix}/events")
    app.register_blueprint(storage_bp, url_prefix=f"{prefix}/storage")
    app.register_blueprint(metrics_bp, url_prefix=f"{prefix}/metrics")
    sock.route(f"{prefix}/instances/<name>/exec")(instance_exec)

    @app.errorhandler(ProviderError)
    def handle_provider_error(exc: ProviderError):
        return jsonify({"error": exc.message}), exc.status_code

    @app.errorhandler(404)
    def handle_404(_):
        return jsonify({"error": "Not found"}), 404

    if config.FRONTEND_DIST:

        @app.get("/")
        @app.get("/<path:path>")
        def frontend(path: str = ""):
            full = f"{config.FRONTEND_DIST}/{path}"
            import os

            if path and os.path.isfile(full):
                return send_from_directory(config.FRONTEND_DIST, path)
            return send_from_directory(config.FRONTEND_DIST, "index.html")

    return app
