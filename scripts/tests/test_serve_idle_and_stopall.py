"""Idle auto-stop and "stop all engine servers" — engine/web/server_controls.py.

Spec: docs/superpowers/specs/2026-09-16-idle-shutdown-and-kill-all.md,
sections 2-6. `server_controls.py` is a new module (serve.py itself is mid
owner-edit and out of scope for this suite) and is designed to be importable
and fully testable without binding a socket or starting an HTTP server: every
route handler here is driven through a small fake standing in for
`http.server.BaseHTTPRequestHandler`, and the watchdog is driven with an
injected clock/killer so no test ever sleeps for minutes or touches a real
process.
"""
import importlib.util
import io
import json
import threading
from pathlib import Path

import pytest


_SPEC = importlib.util.spec_from_file_location(
    "ftd_server_controls", Path(__file__).resolve().parents[2] / "engine" / "web" / "server_controls.py")
server_controls = importlib.util.module_from_spec(_SPEC)
_SPEC.loader.exec_module(server_controls)


class FakeHandler:
    """Just enough of BaseHTTPRequestHandler for server_controls' route
    handlers: `command`, `client_address`, `headers.get(...)`, `rfile.read`,
    and the send_response/send_header/end_headers/wfile.write sequence."""

    def __init__(self, method="POST", body=b"", client_ip="127.0.0.1", content_type="application/json"):
        self.command = method
        self.client_address = (client_ip, 54321) if client_ip is not None else None
        self.headers = {"Content-Length": str(len(body)), "Content-Type": content_type}
        self.rfile = io.BytesIO(body)
        self.wfile = io.BytesIO()
        self.status = None

    def send_response(self, code, message=None):
        self.status = code

    def send_header(self, key, value):
        pass

    def end_headers(self):
        pass

    def json(self):
        return json.loads(self.wfile.getvalue().decode("utf-8"))


def json_body(obj):
    return json.dumps(obj).encode("utf-8")


# ── Activity clock ────────────────────────────────────────────────────

def test_touch_records_activity_on_the_injected_clock():
    clock = {"t": 100.0}
    controls = server_controls.ServerControls(clock=lambda: clock["t"])
    clock["t"] = 150.0
    controls.touch()
    clock["t"] = 170.0
    assert controls.idle_seconds() == pytest.approx(20.0)


def test_handle_refreshes_activity_even_for_a_route_it_does_not_own():
    clock = {"t": 0.0}
    controls = server_controls.ServerControls(clock=lambda: clock["t"])
    clock["t"] = 500.0
    owned = controls.handle(FakeHandler(method="GET", body=b""), "/index.html")
    assert owned is False
    clock["t"] = 500.0  # idle_seconds measured from the touch() above
    assert controls.idle_seconds() == pytest.approx(0.0)


# ── /api/heartbeat ────────────────────────────────────────────────────

def test_heartbeat_returns_ok_and_updates_the_activity_stamp():
    clock = {"t": 0.0}
    controls = server_controls.ServerControls(clock=lambda: clock["t"])
    clock["t"] = 1000.0
    handler = FakeHandler(method="POST", body=b"")
    owned = controls.handle(handler, "/api/heartbeat")
    assert owned is True
    assert handler.status == 200
    assert handler.json() == {"ok": True}
    assert controls.idle_seconds() == pytest.approx(0.0)


def test_heartbeat_is_loopback_only():
    controls = server_controls.ServerControls()
    handler = FakeHandler(method="POST", body=b"", client_ip="203.0.113.5")
    controls.handle(handler, "/api/heartbeat")
    assert handler.status == 403


# ── /api/idle-policy ──────────────────────────────────────────────────

@pytest.mark.parametrize("minutes", [1, 30, 1440, 720])
def test_idle_policy_accepts_the_full_valid_range(minutes):
    controls = server_controls.ServerControls()
    handler = FakeHandler(body=json_body({"enabled": True, "minutes": minutes}))
    controls.handle(handler, "/api/idle-policy")
    assert handler.status == 200
    assert handler.json() == {"enabled": True, "minutes": minutes}
    assert controls.policy.enabled is True and controls.policy.minutes == minutes


@pytest.mark.parametrize("bad_body", [
    {"enabled": True, "minutes": 0},          # below range
    {"enabled": True, "minutes": 1441},       # above range
    {"enabled": True, "minutes": -5},
    {"enabled": True, "minutes": "30"},       # not an int
    {"enabled": True, "minutes": 30.5},       # not an int
    {"enabled": True, "minutes": True},       # bool, not an int
    {"enabled": "yes", "minutes": 30},        # not a bool
    {"enabled": True},                        # missing minutes
    {"minutes": 30},                          # missing enabled
    {},
])
def test_idle_policy_rejects_junk_with_400_and_does_not_change_the_policy(bad_body):
    controls = server_controls.ServerControls()
    before = controls.policy.as_dict()
    handler = FakeHandler(body=json_body(bad_body))
    controls.handle(handler, "/api/idle-policy")
    assert handler.status == 400
    assert controls.policy.as_dict() == before


def test_idle_policy_is_loopback_only():
    controls = server_controls.ServerControls()
    handler = FakeHandler(body=json_body({"enabled": True, "minutes": 30}), client_ip="10.0.0.9")
    controls.handle(handler, "/api/idle-policy")
    assert handler.status == 403


# ── /api/gpu-server/stop-all ──────────────────────────────────────────

def test_stop_all_invokes_the_injected_killer_and_returns_its_pids_and_errors():
    calls = []

    def fake_killer():
        calls.append(True)
        return [111, 222], []

    controls = server_controls.ServerControls(killer=fake_killer)
    handler = FakeHandler(body=b"")
    controls.handle(handler, "/api/gpu-server/stop-all")
    assert handler.status == 200
    assert handler.json() == {"stopped": [111, 222], "errors": []}
    assert len(calls) == 1


def test_stop_all_never_lets_a_body_supplied_target_reach_the_killer():
    """The killer takes zero arguments and the target is a module constant;
    a body trying to smuggle a process name/path/pattern must be completely
    ignored, never forwarded."""
    received = []

    def recording_killer():
        # A zero-arg signature is itself the proof no body content can flow
        # in — this would raise TypeError if the caller tried to pass args.
        received.append(server_controls.WS_SERVER_PROCESS_NAME)
        return [], []

    controls = server_controls.ServerControls(killer=recording_killer)
    malicious_body = json_body({
        "name": "explorer.exe",
        "path": "C:\\Windows\\System32\\notepad.exe",
        "pattern": "*",
        "cmd": "rm -rf /",
    })
    handler = FakeHandler(body=malicious_body)
    controls.handle(handler, "/api/gpu-server/stop-all")
    assert handler.status == 200
    assert received == [server_controls.WS_SERVER_PROCESS_NAME]


def test_stop_all_is_loopback_only():
    controls = server_controls.ServerControls(killer=lambda: ([], []))
    handler = FakeHandler(body=b"", client_ip="192.168.1.50")
    controls.handle(handler, "/api/gpu-server/stop-all")
    assert handler.status == 403


# ── idle_timeout_args (the `_gpu_start` extension) ────────────────────

@pytest.mark.parametrize("body,expected", [
    ({}, []),
    ({"lattice": 65}, []),
    ({"idleMinutes": 15}, ["--idle-timeout-min", "15"]),
    ({"idleMinutes": 0}, ["--idle-timeout-min", "0"]),
    ({"idleMinutes": 1440}, ["--idle-timeout-min", "1440"]),
    ({"idleMinutes": 1441}, []),           # out of range -> ignored, use default
    ({"idleMinutes": -1}, []),
    ({"idleMinutes": "30"}, []),           # not an int -> ignored
    ({"idleMinutes": True}, []),           # bool -> ignored
    ({"autoStop": False}, ["--idle-timeout-min", "0"]),
    ({"autoStop": False, "idleMinutes": 45}, ["--idle-timeout-min", "0"]),  # autoStop:false always wins
    ({"autoStop": True, "idleMinutes": 45}, ["--idle-timeout-min", "45"]),
    ({"autoStop": True}, []),              # no minutes given -> server default
    (None, []),
])
def test_idle_timeout_args_translates_at_most_one_validated_argument(body, expected):
    assert server_controls.idle_timeout_args(body) == expected


def test_idle_timeout_args_never_forwards_any_other_flag_regardless_of_extra_body_content():
    malicious = {
        "idleMinutes": 20, "autoStop": True,
        "exe": "evil.exe", "args": ["--dangerous"], "flag": "--anything",
    }
    assert server_controls.idle_timeout_args(malicious) == ["--idle-timeout-min", "20"]


# ── CLI parsing ────────────────────────────────────────────────────────

def test_parse_idle_timeout_arg_extracts_and_strips_the_pair():
    minutes, remaining = server_controls.parse_idle_timeout_arg(["--idle-timeout-min", "15", "9090"])
    assert minutes == 15
    assert remaining == ["9090"]


def test_parse_idle_timeout_arg_defaults_when_absent():
    minutes, remaining = server_controls.parse_idle_timeout_arg(["9090"])
    assert minutes == server_controls.DEFAULT_IDLE_MINUTES
    assert remaining == ["9090"]


@pytest.mark.parametrize("bad_value", ["not-a-number", "-1", "1441"])
def test_parse_idle_timeout_arg_falls_back_to_default_on_bad_value(bad_value):
    minutes, remaining = server_controls.parse_idle_timeout_arg(["--idle-timeout-min", bad_value])
    assert minutes == server_controls.DEFAULT_IDLE_MINUTES
    assert remaining == []


def test_parse_idle_timeout_arg_defaults_when_value_missing_entirely():
    # The flag itself is still consumed (there is no value to leave behind
    # for a later positional argument to misread).
    minutes, remaining = server_controls.parse_idle_timeout_arg(["--idle-timeout-min"])
    assert minutes == server_controls.DEFAULT_IDLE_MINUTES
    assert remaining == []


# ── validate_idle_minutes ──────────────────────────────────────────────

@pytest.mark.parametrize("value,expected", [
    (1, 1), (30, 30), (1440, 1440),
    (0, None), (1441, None), (-1, None),
    ("30", None), (30.0, None), (True, None), (None, None),
])
def test_validate_idle_minutes(value, expected):
    assert server_controls.validate_idle_minutes(value) == expected


# ── Idle watchdog decision (check_idle_once) ───────────────────────────

def test_check_idle_once_never_fires_when_disabled():
    clock = {"t": 0.0}
    controls = server_controls.ServerControls(clock=lambda: clock["t"])
    controls.policy = server_controls.IdlePolicy(enabled=False, minutes=1)
    clock["t"] = 10_000.0
    assert controls.check_idle_once() is False


def test_check_idle_once_fires_at_the_boundary_and_not_before():
    clock = {"t": 0.0}
    fired = []
    controls = server_controls.ServerControls(clock=lambda: clock["t"])
    controls._shutdown_cb = lambda: fired.append(True)
    controls.policy = server_controls.IdlePolicy(enabled=True, minutes=1)  # 60s
    clock["t"] = 59.0
    assert controls.check_idle_once() is False
    assert fired == []
    clock["t"] = 60.0
    assert controls.check_idle_once() is True
    assert fired == [True]


def test_touch_resets_the_idle_clock_so_activity_prevents_shutdown():
    clock = {"t": 0.0}
    controls = server_controls.ServerControls(clock=lambda: clock["t"])
    controls.policy = server_controls.IdlePolicy(enabled=True, minutes=1)
    clock["t"] = 55.0
    controls.touch()  # e.g. a heartbeat lands just before the timeout
    clock["t"] = 60.0
    assert controls.check_idle_once() is False  # only 5s idle since the touch


def test_watchdog_thread_shuts_down_past_the_timeout_using_an_injected_clock_and_never_sleeps_for_minutes():
    clock = {"t": 0.0}
    shutdown_called = threading.Event()
    controls = server_controls.ServerControls(
        clock=lambda: clock["t"], killer=lambda: ([], []), poll_seconds=0.01)
    controls.start_watchdog(1, lambda: shutdown_called.set())  # 1 minute == 60s
    try:
        clock["t"] = 10.0  # well under the timeout
        assert not shutdown_called.wait(0.2), "must not fire before the timeout"
        clock["t"] = 61.0  # past the timeout
        assert shutdown_called.wait(2.0), "must fire once the timeout has passed"
    finally:
        controls.stop_watchdog(join_timeout=2)
    assert not controls._thread.is_alive()


def test_start_watchdog_with_zero_minutes_disables_auto_stop():
    controls = server_controls.ServerControls(poll_seconds=0.01)
    controls.start_watchdog(0, lambda: pytest.fail("must never be called when disabled"))
    try:
        assert controls.policy.enabled is False
        assert controls.check_idle_once() is False
    finally:
        controls.stop_watchdog(join_timeout=2)
