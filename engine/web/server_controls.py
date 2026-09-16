"""Idle auto-stop and "stop all engine servers" support for the FTD dev server.

New module per the 2026-09-16 idle-shutdown-and-kill-all spec
(`docs/superpowers/specs/2026-09-16-idle-shutdown-and-kill-all.md`, sections
2-5). `engine/web/serve.py` was mid-edit by its owner when this was written
and must not be committed by this work, so every new behaviour lives here
instead: the activity clock, the idle watchdog thread, the stop-all killer,
and the three new route handlers (`/api/heartbeat`, `/api/idle-policy`,
`/api/gpu-server/stop-all`). serve.py needs only a small, uncommitted hook —
see the accompanying report for the exact inserted lines.

Usage from serve.py::

    import server_controls
    CONTROLS = server_controls.ServerControls()

    # in NoCacheHandler.do_GET / do_POST, before existing routing:
    def do_GET(self):
        route = self.path.split("?", 1)[0]
        if CONTROLS.handle(self, route):
            return
        ...

    # in main(), after parsing CLI args:
    idle_minutes, raw = server_controls.parse_idle_timeout_arg(raw)
    ...
    CONTROLS.start_watchdog(idle_minutes, server.shutdown)
    try:
        server.serve_forever()
    finally:
        CONTROLS.stop_watchdog()

This module is deliberately importable and fully testable without binding a
socket or starting an HTTP server: `ServerControls` takes an injectable clock
and killer, and its route handlers accept any object shaped like a
`BaseHTTPRequestHandler` (a fake is enough — see
`scripts/tests/test_serve_idle_and_stopall.py`).
"""

import json
import os
import subprocess
import sys
import threading
import time


DEFAULT_IDLE_MINUTES = 30
MIN_IDLE_MINUTES = 1
MAX_IDLE_MINUTES = 1440
WATCHDOG_POLL_SECONDS = 30
MAX_BODY_BYTES = 65536

# Hard-coded stop-all target. NEVER derive this from a request body — see
# `idle_timeout_args` and `ServerControls._stop_all` for the same rule
# applied to `_gpu_start`'s single validated integer argument.
WS_SERVER_PROCESS_NAME = "ws_server.exe" if os.name == "nt" else "ws_server"


# ── Small HTTP helpers shared by every route below ───────────────────────

def client_is_local(handler):
    """Loopback-only check, identical in spirit to serve.py's existing
    `NoCacheHandler._client_is_local`."""
    return bool(getattr(handler, "client_address", None)) and handler.client_address[0] in ("127.0.0.1", "::1")


def send_json(handler, obj, code=200):
    body = json.dumps(obj).encode("utf-8")
    handler.send_response(code)
    handler.send_header("Content-Type", "application/json")
    handler.send_header("Content-Length", str(len(body)))
    handler.end_headers()
    handler.wfile.write(body)


def read_json_body(handler, max_bytes=MAX_BODY_BYTES):
    """Best-effort JSON object body parse.

    Returns {} for an empty, oversized, non-JSON, or non-object body —
    callers validate whichever fields they actually need explicitly, so a
    malformed body degrades to "as if the field were absent" rather than
    surfacing a decode error of its own.
    """
    try:
        length = int(handler.headers.get("Content-Length", "0") or "0")
    except (TypeError, ValueError):
        return {}
    if length <= 0 or length > max_bytes:
        return {}
    try:
        raw = handler.rfile.read(length)
        data = json.loads(raw or b"{}")
    except (ValueError, TypeError, OSError):
        return {}
    return data if isinstance(data, dict) else {}


# ── Idle policy ────────────────────────────────────────────────────────

class IdlePolicy:
    """An immutable enabled/minutes snapshot. `ServerControls.policy` is
    replaced wholesale on every update (never mutated in place) so a
    concurrent watchdog poll and an /api/idle-policy POST can never observe
    a half-written value."""

    __slots__ = ("enabled", "minutes")

    def __init__(self, enabled=True, minutes=DEFAULT_IDLE_MINUTES):
        self.enabled = bool(enabled)
        self.minutes = int(minutes)

    def as_dict(self):
        return {"enabled": self.enabled, "minutes": self.minutes}

    @property
    def seconds(self):
        return self.minutes * 60

    def __repr__(self):
        return f"IdlePolicy(enabled={self.enabled!r}, minutes={self.minutes!r})"


def validate_idle_minutes(value):
    """Returns `value` as an int if it is a valid minutes count in
    [MIN_IDLE_MINUTES, MAX_IDLE_MINUTES], else None. Rejects bool (Python's
    bool is an int subclass) and anything non-integral."""
    if isinstance(value, bool) or not isinstance(value, int):
        return None
    if MIN_IDLE_MINUTES <= value <= MAX_IDLE_MINUTES:
        return value
    return None


def idle_timeout_args(body):
    """`_gpu_start`'s validated extension (spec section 3): translates the
    optional `idleMinutes`/`autoStop` body fields into at most one extra
    argument, `--idle-timeout-min <N>`.

    `autoStop: false` always wins and sends 0 (disabled), regardless of
    `idleMinutes`. Otherwise a valid `idleMinutes` (0..1440) is forwarded.
    Anything out of range, missing, or malformed is ignored in favour of the
    server's own default — no other flag is ever produced from body content.
    """
    if not isinstance(body, dict):
        return []
    if body.get("autoStop") is False:
        return ["--idle-timeout-min", "0"]
    minutes = body.get("idleMinutes")
    if isinstance(minutes, int) and not isinstance(minutes, bool) and 0 <= minutes <= MAX_IDLE_MINUTES:
        return ["--idle-timeout-min", str(minutes)]
    return []


def parse_idle_timeout_arg(argv):
    """Parses and strips a `--idle-timeout-min <N>` pair from a CLI argv
    list, mirroring serve.py's existing `--host` handling. A missing value,
    a non-integer, or one out of 0..1440 falls back to the default rather
    than raising, since the Python dev server's CLI has no other hard-exit
    validation to match — but the flag and (if present) its value token are
    always both consumed, so an invalid value can never fall through and be
    misread as the positional port argument.

    Returns (idle_minutes, remaining_argv).
    """
    argv = list(argv)
    if "--idle-timeout-min" not in argv:
        return DEFAULT_IDLE_MINUTES, argv
    idx = argv.index("--idle-timeout-min")
    minutes = DEFAULT_IDLE_MINUTES
    has_value = idx + 1 < len(argv)
    if has_value:
        try:
            candidate = int(argv[idx + 1])
        except ValueError:
            candidate = None
        if candidate is not None and 0 <= candidate <= MAX_IDLE_MINUTES:
            minutes = candidate
    consumed = 2 if has_value else 1
    return minutes, argv[:idx] + argv[idx + consumed:]


def idle_policy_banner(idle_minutes):
    """Startup-banner fragment for the effective CLI idle policy."""
    return "auto-stop disabled" if idle_minutes == 0 else f"stops after {idle_minutes} min idle"


# ── Stop-all killer ────────────────────────────────────────────────────

def _run(args, timeout=10):
    return subprocess.run(args, capture_output=True, text=True, timeout=timeout)


def _kill_windows():
    pids = []
    errors = []
    try:
        listing = _run(["tasklist", "/FI", f"IMAGENAME eq {WS_SERVER_PROCESS_NAME}", "/FO", "CSV", "/NH"])
        for line in listing.stdout.splitlines():
            fields = [f.strip('"') for f in line.strip().split(",")]
            if len(fields) >= 2 and fields[0].lower() == WS_SERVER_PROCESS_NAME.lower():
                try:
                    pids.append(int(fields[1]))
                except ValueError:
                    pass
    except (OSError, subprocess.SubprocessError) as exc:
        errors.append(str(exc))
    try:
        _run(["taskkill", "/IM", WS_SERVER_PROCESS_NAME, "/F"])
    except (OSError, subprocess.SubprocessError) as exc:
        errors.append(str(exc))
    return pids, errors


def _kill_posix():
    pids = []
    errors = []
    try:
        listing = _run(["pgrep", "-f", WS_SERVER_PROCESS_NAME])
        pids = [int(p) for p in listing.stdout.split() if p.isdigit()]
    except (OSError, subprocess.SubprocessError) as exc:
        errors.append(str(exc))
    try:
        _run(["pkill", "-f", WS_SERVER_PROCESS_NAME])
    except (OSError, subprocess.SubprocessError) as exc:
        errors.append(str(exc))
    return pids, errors


def default_kill_ws_server():
    """The hard-coded stop-all target: every `ws_server(.exe)` process on the
    machine, found and killed by a fixed name only — never a name, path, or
    pattern taken from a request. Returns (pids, errors); never raises."""
    try:
        return _kill_windows() if os.name == "nt" else _kill_posix()
    except Exception as exc:  # pragma: no cover - defensive: a route must always answer
        return [], [str(exc)]


# ── The controller itself ─────────────────────────────────────────────

class ServerControls:
    """Owns the activity clock, the idle watchdog thread, the stop-all
    killer, and the three new route handlers. One instance lives at
    serve.py module scope for the process lifetime.
    """

    POST_ROUTES = ("/api/heartbeat", "/api/idle-policy", "/api/gpu-server/stop-all")

    def __init__(self, clock=time.monotonic, killer=None, poll_seconds=WATCHDOG_POLL_SECONDS, log=None):
        self._clock = clock
        self._killer = killer or default_kill_ws_server
        self._poll_seconds = poll_seconds
        self._log = log or (lambda msg: print(msg, flush=True))
        self._lock = threading.Lock()
        self._last_activity = clock()
        self.policy = IdlePolicy()
        self._stop_event = threading.Event()
        self._thread = None
        self._shutdown_cb = None

    # ---- activity clock ----

    def touch(self):
        """Record activity now. Called unconditionally for every HTTP
        request `handle()` sees, whether or not it owns the route — that is
        what makes "any HTTP request handled" (spec §2) true."""
        with self._lock:
            self._last_activity = self._clock()

    def idle_seconds(self):
        with self._lock:
            return self._clock() - self._last_activity

    # ---- HTTP hook ----

    def handle(self, handler, route):
        """Call at the top of do_GET/do_POST with the handler instance and
        its query-stripped path. Always refreshes the activity clock first;
        returns True only when this call fully answered the request."""
        self.touch()
        method = getattr(handler, "command", None)
        if method != "POST" or route not in self.POST_ROUTES:
            return False
        if route == "/api/heartbeat":
            return self._heartbeat(handler)
        if route == "/api/idle-policy":
            return self._idle_policy(handler)
        return self._stop_all(handler)

    def _heartbeat(self, handler):
        if not client_is_local(handler):
            send_json(handler, {"error": "forbidden (loopback only)"}, 403)
            return True
        send_json(handler, {"ok": True})
        return True

    def _idle_policy(self, handler):
        if not client_is_local(handler):
            send_json(handler, {"error": "forbidden (loopback only)"}, 403)
            return True
        body = read_json_body(handler)
        enabled = body.get("enabled")
        minutes = validate_idle_minutes(body.get("minutes"))
        if not isinstance(enabled, bool) or minutes is None:
            send_json(handler, {"error": "enabled must be a bool, minutes an integer 1..1440"}, 400)
            return True
        self.policy = IdlePolicy(enabled=enabled, minutes=minutes)
        send_json(handler, self.policy.as_dict())
        return True

    def _stop_all(self, handler):
        if not client_is_local(handler):
            send_json(handler, {"error": "forbidden (loopback only)"}, 403)
            return True
        # The body is drained so the connection stays well-formed, but its
        # content is never inspected: the killer takes zero arguments and
        # the process target is hard-coded module-wide (WS_SERVER_PROCESS_NAME).
        read_json_body(handler)
        pids, errors = self._killer()
        send_json(handler, {"stopped": pids, "errors": errors})
        return True

    # ---- idle watchdog ----

    def start_watchdog(self, idle_minutes, shutdown_cb):
        """idle_minutes: the CLI value (0 disables auto-stop entirely).
        shutdown_cb: a zero-arg callable invoked from the watchdog thread
        only, never from a request handler (spec §2)."""
        self.policy = IdlePolicy(
            enabled=idle_minutes > 0,
            minutes=idle_minutes if idle_minutes > 0 else DEFAULT_IDLE_MINUTES,
        )
        self._shutdown_cb = shutdown_cb
        self._stop_event.clear()
        self.touch()
        self._thread = threading.Thread(target=self._watchdog_loop, name="idle-watchdog", daemon=True)
        self._thread.start()
        return self._thread

    def stop_watchdog(self, join_timeout=2):
        self._stop_event.set()
        if self._thread is not None:
            self._thread.join(timeout=join_timeout)

    def check_idle_once(self):
        """One watchdog decision, standalone-testable without a thread or a
        real clock: returns True (and invokes shutdown_cb, if any) exactly
        when the current policy is enabled and the idle time has reached or
        passed its timeout. Never fires early, and never fires at all when
        disabled."""
        policy = self.policy
        if not policy.enabled:
            return False
        if self.idle_seconds() < policy.seconds:
            return False
        if self._shutdown_cb is not None:
            self._shutdown_cb()
        return True

    def _watchdog_loop(self):
        while not self._stop_event.is_set():
            if self._stop_event.wait(self._poll_seconds):
                return
            if self.check_idle_once():
                return
