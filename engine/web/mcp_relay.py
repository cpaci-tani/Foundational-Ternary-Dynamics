"""Bounded, process-local mailbox for one explicitly paired browser session.

The relay cannot execute simulation commands itself. Browser owners consume
allowlisted data requests and return their own acknowledged results. Credentials
never appear in URLs, logs, disk, or cross-origin responses.
"""
from __future__ import annotations

from dataclasses import dataclass, field
import hashlib
import hmac
import ipaddress
import json
from pathlib import Path
import re
import secrets
import threading
import time
from urllib.parse import urlsplit

METHODS = frozenset(("observe", "measure_flux_sectors", "list_scenarios", "describe_scenario", "execute_plan",
                     "run_scenario", "run_status", "search_docs", "stop"))
READ_METHODS = frozenset(("observe", "measure_flux_sectors", "list_scenarios", "describe_scenario", "run_status", "search_docs"))
MAX_REQUEST_BYTES = 256 * 1024
MAX_REPLY_BYTES = 1024 * 1024
MAX_SESSIONS = 8
MAX_PENDING = 8
MAX_HISTORY = 128
MAX_RESULT_BYTES = 8 * MAX_REPLY_BYTES
MAX_READ_HISTORY = 32
MAX_READ_RESULT_BYTES = 2 * MAX_REPLY_BYTES
# Reserve one additional control slot for Stop, including when normal work or
# retained mutation receipts fill their budgets. Cancellation notices are also
# bounded; admission reserves space for every still-pending operation.
MAX_CANCELLED = MAX_HISTORY + MAX_READ_HISTORY + MAX_PENDING + 1
SERVER_PATH = str(Path(__file__).resolve().parents[1] / "mcp" / "server.mjs")
_ID = re.compile(r"[A-Za-z0-9._:-]{1,128}\Z")


class RelayError(Exception):
    def __init__(self, status, code, message, outcome=None):
        super().__init__(message)
        self.status = status
        self.body = {"error": {"code": code, "message": message}}
        if outcome is not None:
            self.body["error"]["outcome"] = outcome


def _error(status, code, message, outcome=None):
    error = RelayError(status, code, message, outcome)
    return error.status, error.body


def _json_bytes(value):
    try:
        return json.dumps(value, allow_nan=False, ensure_ascii=True,
                          separators=(",", ":"), sort_keys=True).encode("utf-8")
    except (TypeError, ValueError, RecursionError):
        raise RelayError(400, "invalid_json", "Plain finite JSON data required") from None


def _keys(value, required, optional=()):
    if not isinstance(value, dict) or not set(required) <= set(value) or set(value) - set(required) - set(optional):
        raise RelayError(400, "invalid_request", "Unexpected or missing request fields")


def _identifier(value):
    if not isinstance(value, str) or not _ID.fullmatch(value):
        raise RelayError(400, "invalid_id", "A bounded request identifier is required")
    return value


@dataclass
class Request:
    id: str
    fingerprint: str | None
    method: str
    args: dict
    deadline: float
    deadline_ms: int
    delivered: bool = False
    response: tuple | None = None
    reply_fingerprint: str | None = None
    response_bytes: int = 0


@dataclass
class Session:
    id: str
    browser_token: str
    client_token: str
    label: str
    last_poll: float
    calls: dict = field(default_factory=dict)
    cancelled: list = field(default_factory=list)
    stop_cancelled: str | None = None
    result_bytes: int = 0
    read_result_bytes: int = 0
    waiters: int = 0
    stop_waiters: int = 0
    polling: bool = False
    closed: bool = False


class Relay:
    def __init__(self, *, clock=time.monotonic, wall_clock=time.time,
                 heartbeat_seconds=45, poll_seconds=20, call_seconds=60):
        self.clock, self.wall_clock = clock, wall_clock
        self.heartbeat_seconds = min(45, max(1, heartbeat_seconds))
        self.poll_seconds = min(20, max(0, poll_seconds))
        self.call_seconds = min(60, max(0, call_seconds))
        self.sessions = {}
        self.condition = threading.Condition()

    def wake(self):
        """Wake bounded waits after an injected clock advances in tests."""
        with self.condition:
            self.condition.notify_all()

    def _finish(self, session, request, response, *, cancel=False):
        if request.response is not None:
            return
        request.response = response
        request.args = {}
        request.response_bytes = len(_json_bytes(response[1]))
        if request.method in READ_METHODS:
            session.read_result_bytes += request.response_bytes
            self._prune_reads(session)
        elif request.method != "stop":
            session.result_bytes += request.response_bytes
        if cancel:
            if request.method == "stop":
                # Stop is idempotent control. One latest cancellation notice
                # is sufficient and cannot turn a Stop/cancel flood into an
                # unbounded queue of identifiers.
                session.stop_cancelled = request.id
            elif request.id not in session.cancelled:
                session.cancelled.append(request.id)
        self.condition.notify_all()

    def _prune_reads(self, session):
        """Evict completed reads only. Mutations and early cancel tombstones
        retain their original receipts/IDs for the entire paired session.
        Waiting callers hold their Request directly, so eviction cannot erase
        a reply already awaited by an admitted HTTP call.
        """
        completed = [row for row in session.calls.values()
                     if row.method in READ_METHODS and row.response is not None]
        for request in completed:
            if len(completed) <= MAX_READ_HISTORY and session.read_result_bytes <= MAX_READ_RESULT_BYTES:
                break
            session.calls.pop(request.id, None)
            session.read_result_bytes -= request.response_bytes
            completed = completed[1:]

    @staticmethod
    def _durable(request):
        # Empty method names are cancel-before-call tombstones, whose future
        # payload could be a mutation. Never demote or evict them.
        return request.method not in READ_METHODS and request.method != "stop"

    @staticmethod
    def _pending(session):
        return sum(row.response is None and row.method != "stop" for row in session.calls.values())

    def _close(self, session, code):
        session.closed = True
        self.sessions.pop(session.id, None)
        for request in tuple(session.calls.values()):
            self._finish(session, request, _error(410, code, "Browser session is no longer connected",
                         "unknown" if request.delivered else "not_dispatched"))
        self.condition.notify_all()

    def _sweep(self):
        now = self.clock()
        for session in list(self.sessions.values()):
            if now - session.last_poll >= self.heartbeat_seconds:
                self._close(session, "session_expired")
                continue
            for request in tuple(session.calls.values()):
                if request.response is None and now >= request.deadline:
                    self._finish(session, request, _error(504, "request_timeout", "Browser reply deadline elapsed",
                                 "unknown" if request.delivered else "not_dispatched"), cancel=True)

    def _session(self, session_id, token, browser=False):
        self._sweep()
        session = self.sessions.get(session_id) if isinstance(session_id, str) else None
        if session is None:
            raise RelayError(410, "session_unavailable", "Browser session is unavailable; pair again")
        expected = session.browser_token if browser else session.client_token
        if not isinstance(token, str) or len(token) > 128 or not token.isascii() or not hmac.compare_digest(token, expected):
            raise RelayError(401, "unauthorized", "Invalid session credential")
        return session

    def register(self, label):
        if not isinstance(label, str) or not label.strip() or len(label) > 80:
            raise RelayError(400, "invalid_label", "Session label must contain 1 to 80 characters")
        with self.condition:
            self._sweep()
            if len(self.sessions) >= MAX_SESSIONS:
                raise RelayError(429, "session_limit", "Eight browser sessions are already paired")
            session = Session(secrets.token_urlsafe(24), secrets.token_urlsafe(32),
                              secrets.token_urlsafe(32), label.strip(), self.clock())
            self.sessions[session.id] = session
            return {"sessionId": session.id, "browserToken": session.browser_token,
                    "clientToken": session.client_token, "serverPath": SERVER_PATH}

    def poll(self, session_id, token):
        with self.condition:
            session = self._session(session_id, token, browser=True)
            if session.polling:
                raise RelayError(409, "poll_in_progress", "One browser poll may be active per session")
            session.last_poll = self.clock()
            session.polling = True
            self.condition.notify_all()
            deadline = self.clock() + self.poll_seconds
            try:
                while True:
                    self._sweep()
                    if session.closed:
                        raise RelayError(410, "session_unavailable", "Browser session was disconnected")
                    requests = []
                    for request in sorted(session.calls.values(), key=lambda row: row.method != "stop"):
                        if request.response is None and not request.delivered:
                            request.delivered = True
                            requests.append({"id": request.id, "method": request.method,
                                             "args": request.args, "deadline": request.deadline_ms})
                    cancelled, session.cancelled = session.cancelled, []
                    if session.stop_cancelled is not None:
                        cancelled.append(session.stop_cancelled)
                        session.stop_cancelled = None
                    remaining = deadline - self.clock()
                    if requests or cancelled or remaining <= 0:
                        session.last_poll = self.clock()
                        return {"requests": requests, "cancelled": cancelled}
                    self.condition.wait(min(remaining, self.heartbeat_seconds))
            finally:
                session.polling = False

    def call(self, session_id, token, request_id, method, args):
        request_id = _identifier(request_id)
        if not isinstance(method, str) or method not in METHODS or not isinstance(args, dict):
            raise RelayError(400, "invalid_method", "An allowlisted method and argument object are required")
        if method == "stop" and args:
            raise RelayError(400, "invalid_request", "Stop takes no arguments")
        payload = _json_bytes({"method": method, "args": args})
        if len(payload) > MAX_REQUEST_BYTES:
            raise RelayError(413, "request_too_large", "Request exceeds 256 KiB")
        fingerprint = hashlib.sha256(payload).hexdigest()
        with self.condition:
            session = self._session(session_id, token)
            request = session.calls.get(request_id)
            # An authenticated cancellation may overtake its original HTTP
            # call. Bind its tombstone to the first arriving payload without
            # ever delivering the cancelled operation to the browser.
            if request is not None and request.fingerprint is None:
                request.fingerprint = fingerprint
            if request is not None and request.fingerprint != fingerprint:
                raise RelayError(409, "id_conflict", "Request ID already belongs to a different payload")
            if request is not None and request.response is not None:
                return request.response
            if method == "stop":
                stopping = next((row for row in session.calls.values()
                                 if row.method == "stop" and row.response is None), None)
                if stopping is not None:
                    # One admitted control command is sufficient. Do not let
                    # stop floods allocate unbounded requests or HTTP waiters.
                    return 202, {"result": {"status": "stop_requested", "requestId": stopping.id,
                                             "coalesced": True}}
            elif session.waiters >= MAX_PENDING:
                raise RelayError(429, "waiter_limit", "At most eight calls may wait per session")
            if request is None:
                pending = self._pending(session)
                if method != "stop" and pending >= MAX_PENDING:
                    raise RelayError(429, "pending_limit", "At most eight requests may be pending")
                if method != "stop" and len(session.cancelled) + pending + 1 >= MAX_CANCELLED:
                    raise RelayError(429, "cancellation_backlog", "Browser must receive pending cancellation notices before more requests")
                if method not in READ_METHODS and method != "stop":
                    durable = [row for row in session.calls.values() if self._durable(row)]
                    durable_pending = sum(row.response is None for row in durable)
                    if len(durable) >= MAX_HISTORY or session.result_bytes + (durable_pending + 1) * MAX_REPLY_BYTES > MAX_RESULT_BYTES:
                        raise RelayError(429, "history_limit", "Mutation receipt budget exhausted; disconnect and pair again")
                if method == "stop":
                    # Stop is idempotent control, not an experimental mutation.
                    # Retain its latest receipt only, separately from durable
                    # simulation-write receipts. Never drop a pending Stop.
                    for prior in tuple(session.calls.values()):
                        if prior.method == "stop" and prior.response is not None:
                            session.calls.pop(prior.id, None)
                request = Request(request_id, fingerprint, method, json.loads(payload)["args"],
                                  self.clock() + self.call_seconds,
                                  int((self.wall_clock() + self.call_seconds) * 1000))
                session.calls[request_id] = request
                self.condition.notify_all()
            if method == "stop" and session.stop_waiters:
                # A completed prior Stop may still be returning through its
                # original HTTP waiter. Admit this control request without
                # allocating a second blocking waiter in that short window.
                return 202, {"result": {"status": "stop_requested", "requestId": request.id,
                                         "coalesced": False}}
            if method == "stop":
                session.stop_waiters += 1
            else:
                session.waiters += 1
            try:
                while request.response is None:
                    self._sweep()
                    if request.response is not None:
                        break
                    remaining = min(request.deadline - self.clock(),
                                    self.heartbeat_seconds - (self.clock() - session.last_poll))
                    self.condition.wait(max(0, remaining))
                return request.response
            finally:
                if method == "stop":
                    session.stop_waiters -= 1
                else:
                    session.waiters -= 1

    def reply(self, session_id, token, request_id, response):
        request_id = _identifier(request_id)
        if not isinstance(response, dict) or set(response) not in ({"result"}, {"error"}):
            raise RelayError(400, "invalid_reply", "Reply must contain exactly one result or error")
        body = _json_bytes(response)
        if len(body) > MAX_REPLY_BYTES:
            raise RelayError(413, "reply_too_large", "Reply exceeds 1 MiB")
        fingerprint = hashlib.sha256(body).hexdigest()
        with self.condition:
            session = self._session(session_id, token, browser=True)
            request = session.calls.get(request_id)
            if request is None or not request.delivered:
                raise RelayError(409, "request_not_delivered", "No delivered request matches this reply")
            if request.response is not None:
                if request.reply_fingerprint == fingerprint:
                    return {"accepted": True}
                raise RelayError(409, "request_terminal", "Request already has a terminal outcome")
            request.reply_fingerprint = fingerprint
            self._finish(session, request, (200, json.loads(body)))
            return {"accepted": True}

    def cancel(self, session_id, token, request_id):
        request_id = _identifier(request_id)
        with self.condition:
            session = self._session(session_id, token)
            request = session.calls.get(request_id)
            if request is None:
                durable = [row for row in session.calls.values() if self._durable(row)]
                pending = sum(row.response is None for row in durable)
                response = _error(409, "request_cancelled", "Cancellation requested before dispatch", "not_dispatched")
                if (len(durable) >= MAX_HISTORY
                        or session.result_bytes + pending * MAX_REPLY_BYTES + len(_json_bytes(response[1])) > MAX_RESULT_BYTES):
                    raise RelayError(429, "history_limit", "Mutation receipt budget exhausted; disconnect and pair again")
                if len(session.cancelled) + self._pending(session) + 1 >= MAX_CANCELLED:
                    raise RelayError(429, "cancellation_backlog", "Browser must receive pending cancellation notices before more requests")
                request = Request(request_id, None, "", {}, self.clock(), int(self.wall_clock() * 1000))
                session.calls[request_id] = request
                self._finish(session, request, response, cancel=True)
            if request.response is None:
                self._finish(session, request, _error(409, "request_cancelled", "Cancellation requested",
                             "unknown" if request.delivered else "not_dispatched"), cancel=True)
            return {"cancelled": request_id, "outcome": request.response[1]}

    def disconnect(self, session_id, token):
        with self.condition:
            session = self._session(session_id, token, browser=True)
            self._close(session, "session_disconnected")
            return {"disconnected": True}


RELAY = Relay()


def _same_origin_loopback(handler):
    try:
        if not ipaddress.ip_address(handler.client_address[0]).is_loopback:
            return False
        hosts = handler.headers.get_all("Host", [])
        if len(hosts) != 1:
            return False
        host = hosts[0]
        parsed = urlsplit("http://" + host)
        if (parsed.hostname not in ("localhost", "127.0.0.1", "::1")
                or parsed.username or parsed.password or parsed.path or parsed.query or parsed.fragment
                or (parsed.port or 80) != handler.server.server_port
                or any(character.isspace() for character in host)):
            return False
        origins = handler.headers.get_all("Origin", [])
        if len(origins) > 1 or origins and origins[0] != "http://" + host:
            return False
        sites = handler.headers.get_all("Sec-Fetch-Site", [])
        return len(sites) <= 1 and (not sites or sites[0] in ("same-origin", "none"))
    except (ValueError, IndexError, AttributeError):
        return False


def _read(handler, limit):
    if handler.headers.get_content_type() != "application/json":
        raise RelayError(415, "json_required", "Content-Type application/json required")
    lengths = handler.headers.get_all("Content-Length", [])
    if handler.headers.get("Transfer-Encoding") or len(lengths) != 1:
        raise RelayError(400, "length_required", "One Content-Length header is required")
    try:
        length = int(lengths[0])
    except ValueError:
        raise RelayError(400, "invalid_length", "Invalid Content-Length") from None
    if not 0 < length <= limit:
        raise RelayError(413, "body_too_large", "Request body exceeds the endpoint limit")
    def pairs(values):
        result = {}
        for key, value in values:
            if key in result:
                raise ValueError("duplicate key")
            result[key] = value
        return result
    try:
        raw = handler.rfile.read(length)
        if len(raw) != length:
            raise ValueError("incomplete body")
        value = json.loads(raw, object_pairs_hook=pairs)
        _json_bytes(value)
        return value
    except (ValueError, TypeError, RecursionError, UnicodeError):
        raise RelayError(400, "invalid_json", "Bounded plain JSON object required") from None


def _send(handler, value, status=200):
    body = _json_bytes(value)
    handler.send_response(status)
    handler.send_header("Content-Type", "application/json")
    handler.send_header("Content-Length", str(len(body)))
    handler.send_header("Cache-Control", "no-store")
    handler.send_header("X-Content-Type-Options", "nosniff")
    handler.end_headers()
    handler.wfile.write(body)


def handle(handler, route):
    if not route.startswith("/api/mcp/"):
        return False
    handler._mcp_request = True
    try:
        if not _same_origin_loopback(handler):
            raise RelayError(403, "origin_forbidden", "Loopback same-origin relay required")
        if urlsplit(handler.path).query or urlsplit(handler.path).fragment:
            raise RelayError(400, "url_credentials_forbidden", "Relay URLs do not accept query parameters")
        if handler.command != "POST":
            raise RelayError(405, "method_not_allowed", "Relay endpoints require POST")
        operation = route.removeprefix("/api/mcp/")
        if operation not in ("register", "poll", "reply", "disconnect", "call", "cancel"):
            raise RelayError(404, "route_unknown", "Unknown relay endpoint")
        browser = operation in ("register", "poll", "reply", "disconnect")
        if browser and handler.headers.get_all("X-FTD-MCP", []) != ["1"]:
            raise RelayError(403, "browser_header_required", "X-FTD-MCP: 1 required")
        token = ""
        if not browser:
            authorization = handler.headers.get_all("Authorization", [])
            if len(authorization) != 1 or not authorization[0].startswith("Bearer "):
                raise RelayError(401, "unauthorized", "A paired client bearer token is required")
            token = authorization[0][7:]
        body = _read(handler, MAX_REPLY_BYTES if operation == "reply" else MAX_REQUEST_BYTES)
        if operation == "register":
            _keys(body, ("label",))
            result = RELAY.register(body["label"])
        elif browser:
            fields = ("sessionId", "browserToken", "id") if operation == "reply" else ("sessionId", "browserToken")
            _keys(body, fields, ("result", "error") if operation == "reply" else ())
            if operation == "reply":
                result = RELAY.reply(body["sessionId"], body["browserToken"], body["id"],
                                     {key: body[key] for key in ("result", "error") if key in body})
            else:
                result = getattr(RELAY, operation)(body["sessionId"], body["browserToken"])
        elif operation == "call":
            _keys(body, ("sessionId", "id", "method", "args"))
            status, result = RELAY.call(body["sessionId"], token, body["id"], body["method"], body["args"])
            _send(handler, result, status)
            return True
        else:
            _keys(body, ("sessionId", "id"))
            result = RELAY.cancel(body["sessionId"], token, body["id"])
        _send(handler, result)
    except RelayError as error:
        _send(handler, error.body, error.status)
    return True
