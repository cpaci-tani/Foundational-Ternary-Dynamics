"""Loopback AI transport and allowlisted immutable local artifacts.

Keys exist only in request memory or TYPESAFE_API_KEY. No request bodies,
authorization headers, upstream bodies or credentials are logged.
"""
from __future__ import annotations

from collections import deque
import hashlib
import json
import math
import mimetypes
import os
from pathlib import Path
import threading
import time
from urllib.parse import unquote, urlsplit
import urllib.error
import urllib.request

ROOT = Path(__file__).resolve().parents[2]
MODEL_MANIFEST = ROOT / "engine/web/js/assistant/model-manifest.json"
MODEL_ROOT = ROOT / "engine/.cache/local-ai/models"
KNOWLEDGE_ROOT = ROOT / "engine/.cache/local-ai/knowledge"
CONTRACT = json.loads((ROOT / "engine/ai-proxy/contract.json").read_text(encoding="utf-8"))
_slots = threading.BoundedSemaphore(2)
_rate_lock = threading.Lock()
_requests = deque()
_verified = {}
_verify_lock = threading.Lock()


def _send(handler, value, status=200):
    body = json.dumps(value, allow_nan=False).encode("utf-8")
    handler.send_response(status)
    handler.send_header("Content-Type", "application/json")
    handler.send_header("Content-Length", str(len(body)))
    handler.end_headers()
    handler.wfile.write(body)


def _local(handler):
    if not handler.client_address or handler.client_address[0] not in ("127.0.0.1", "::1"):
        return False
    try:
        host = urlsplit("http://" + handler.headers.get("Host", ""))
        if host.hostname not in ("localhost", "127.0.0.1", "::1") or host.username or host.password:
            return False
        if host.port != handler.server.server_port:
            return False
    except ValueError:
        return False
    origin = handler.headers.get("Origin")
    return (not origin or origin == "http://" + handler.headers.get("Host", "")) and handler.headers.get("Sec-Fetch-Site") != "cross-site"


def validate_input(value):
    if not isinstance(value, dict) or set(value) != {"intent", "observation", "plan"}:
        raise ValueError("Expected intent, observation and plan")
    if not isinstance(value["intent"], str) or not 1 <= len(value["intent"].strip()) <= 8192:
        raise ValueError("Intent must be a nonempty bounded string")
    if not isinstance(value["observation"], dict) or not isinstance(value["plan"], dict):
        raise ValueError("Observation and plan must be objects")
    # Reject nonfinite JSON extensions accepted by Python's default parser.
    json.dumps(value, allow_nan=False)
    return value


def normalize_answer(value):
    answers = value.get("answers") if isinstance(value, dict) else None
    answer = answers.get("decision") if isinstance(answers, dict) else None
    if not isinstance(answer, dict):
        raise ValueError("Invalid decision envelope")
    confidence = answer.get("confidence")
    if answer.get("type") != "choice" or answer.get("choice") not in ("execute", "clarify", "reject"):
        raise ValueError("Invalid decision")
    if isinstance(confidence, bool) or not isinstance(confidence, (int, float)) or not math.isfinite(confidence) or not 0 <= confidence <= 1:
        raise ValueError("Invalid confidence")
    model = value.get("model")
    if not isinstance(model, str) or not 1 <= len(model) <= 128:
        raise ValueError("Missing model identity")
    usage = value.get("usage", {})
    usage = {key: val for key, val in usage.items() if key in ("input_tokens", "output_tokens") and type(val) is int and val >= 0} if isinstance(usage, dict) else {}
    return {"decision": answer["choice"], "confidence": confidence, "model": model, "usage": usage}


def _upstream(value, key):
    payload = {"state": json.dumps(value, allow_nan=False), "model": CONTRACT["model"], "questions": CONTRACT["questions"]}
    request = urllib.request.Request(CONTRACT["upstream"], data=json.dumps(payload).encode(),
        headers={"Content-Type": "application/json", "Authorization": "Bearer " + key}, method="POST")
    # Redirects must not forward a visitor's credentials to a second host.
    class NoRedirect(urllib.request.HTTPRedirectHandler):
        def redirect_request(self, *args, **kwargs):
            return None
    with urllib.request.build_opener(NoRedirect).open(request, timeout=CONTRACT["timeoutSeconds"]) as response:
        data = response.read(CONTRACT["maxResponseBytes"] + 1)
        if len(data) > CONTRACT["maxResponseBytes"]:
            raise ValueError("Oversized upstream response")
        return normalize_answer(json.loads(data))


def _jev(handler):
    if handler.headers.get_content_type() != "application/json":
        return _send(handler, {"error": "JSON required"}, 415)
    if handler.headers.get("Transfer-Encoding"):
        return _send(handler, {"error": "Content-Length required"}, 400)
    try:
        length = int(handler.headers.get("Content-Length", "0"))
    except ValueError:
        length = 0
    if not 0 < length <= CONTRACT["maxBodyBytes"]:
        return _send(handler, {"error": "Request exceeds 64 KiB or is empty"}, 413)
    try:
        value = validate_input(json.loads(handler.rfile.read(length)))
    except (ValueError, TypeError, RecursionError):
        return _send(handler, {"error": "Invalid decision request"}, 400)
    header = handler.headers.get("Authorization", "")
    key = header.removeprefix("Bearer ") if header.startswith("Bearer ") else ""
    if header and not key:
        return _send(handler, {"error": "Bearer key required"}, 401)
    key = key or os.environ.get("TYPESAFE_API_KEY", "")
    if not key or len(key) > 4096 or any(ord(c) < 33 or ord(c) > 126 for c in key):
        return _send(handler, {"error": "Supply a TypeSafe API key"}, 401)
    with _rate_lock:
        now = time.monotonic()
        while _requests and _requests[0] < now - 60:
            _requests.popleft()
        if len(_requests) >= 20:
            return _send(handler, {"error": "Decision request limit reached"}, 429)
        _requests.append(now)
    if not _slots.acquire(blocking=False):
        return _send(handler, {"error": "Decision service busy"}, 429)
    try:
        return _send(handler, _upstream(value, key))
    except urllib.error.HTTPError as error:
        return _send(handler, {"error": "TypeSafe rejected the request"}, 401 if error.code in (401, 403) else 502)
    except (OSError, ValueError, TypeError, RecursionError):
        return _send(handler, {"error": "Decision service unavailable or returned an invalid answer"}, 502)
    finally:
        key = ""
        _slots.release()


def _asset(handler, root, relative, entries):
    try:
        relative = unquote(relative, errors="strict")
        entry = entries.get(relative)
        if entry is None or any(x in relative for x in ("\\", "\x00", ":")):
            raise ValueError("Not allowlisted")
        target = (root / relative).resolve()
        if not target.is_relative_to(root.resolve()) or not target.is_file():
            raise ValueError("Missing asset")
        if entry.get("size") is not None and target.stat().st_size != entry["size"]:
            raise ValueError("Changed asset size")
        if entry.get("sha256"):
            stat = target.stat()
            stamp = (stat.st_mtime_ns, stat.st_size, entry["sha256"])
            with _verify_lock:
                if _verified.get(target) != stamp:
                    h = hashlib.sha256()
                    with target.open("rb") as source:
                        for block in iter(lambda: source.read(1024 * 1024), b""): h.update(block)
                    if h.hexdigest() != entry["sha256"]: raise ValueError("Changed asset hash")
                    _verified[target] = stamp
        handler.send_response(200)
        source_document = entry.get("kind") == "source-document"
        handler.send_header("Content-Type", "text/html; charset=utf-8" if source_document else mimetypes.guess_type(target.name)[0] or "application/octet-stream")
        if source_document:
            handler.send_header("Content-Security-Policy", "default-src 'none'; base-uri 'none'; form-action 'none'; sandbox")
            handler.send_header("X-Content-Type-Options", "nosniff")
        handler.send_header("Content-Length", str(target.stat().st_size))
        if entry.get("sha256"):
            handler.send_header("ETag", '"' + entry["sha256"] + '"')
        # Only immutable hash-addressed artifacts escape the dev no-store policy.
        handler._ai_immutable_asset = bool(entry.get("sha256"))
        handler.end_headers()
        with target.open("rb") as source:
            for block in iter(lambda: source.read(1024 * 1024), b""):
                handler.wfile.write(block)
    except (ValueError, OSError, UnicodeError):
        return _send(handler, {"error": "Asset unavailable; prepare local AI assets first"}, 404)


def handle(handler, route):
    if not route.startswith("/api/ai/"):
        return False
    if not _local(handler):
        _send(handler, {"error": "Same-origin loopback AI service required"}, 403)
        return True
    if handler.command == "POST" and route == "/api/ai/jev":
        _jev(handler)
    elif handler.command == "GET" and route == "/api/ai/status":
        _send(handler, {"available": True, "jevConfigured": bool(os.environ.get("TYPESAFE_API_KEY")), "byok": True,
            "knowledgeManifest": "/api/ai/knowledge/manifest.json", "modelBase": "/api/ai/assets/",
            "knowledgeReady": (KNOWLEDGE_ROOT / "manifest.json").is_file()})
    elif handler.command == "GET" and route.startswith("/api/ai/assets/"):
        manifest = json.loads(MODEL_MANIFEST.read_text(encoding="utf-8"))
        entries = {e["path"]: e for variant in manifest["variants"] for e in variant["files"]}
        _asset(handler, MODEL_ROOT, route[len("/api/ai/assets/"):], entries)
    elif handler.command == "GET" and route.startswith("/api/ai/knowledge/"):
        try:
            manifest = json.loads((KNOWLEDGE_ROOT / "manifest.json").read_text(encoding="utf-8"))
            entries = {e["path"]: e for e in manifest["files"]}
            entries["manifest.json"] = {}
            _asset(handler, KNOWLEDGE_ROOT, route[len("/api/ai/knowledge/"):], entries)
        except (ValueError, OSError, KeyError):
            _send(handler, {"error": "Local documentation index not built"}, 404)
    else:
        _send(handler, {"error": "Unknown AI route"}, 404)
    return True
