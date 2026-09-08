"""Bounded, forced-CPU wire oracle for native transport retirement and ordering.

Owns its server and ephemeral port. Timings test cooperative network deadlines,
not preemption of engine work. Digests below are the existing scoped readout,
not a complete checkpoint or a replay certificate.
"""
import argparse
import base64
import hashlib
import json
import os
from pathlib import Path
import socket
import struct
import subprocess
import time


def exact(sock, count):
    out = bytearray()
    while len(out) < count:
        part = sock.recv(count - len(out))
        if not part:
            raise EOFError("server disconnected")
        out.extend(part)
    return bytes(out)


def frame(payload=b"", opcode=1):
    if isinstance(payload, dict):
        payload = json.dumps(payload, separators=(",", ":")).encode()
    mask = b"\x12\x34\x56\x78"
    size = len(payload)
    prefix = bytes([0x80 | opcode, 0x80 | size]) if size < 126 else (
        bytes([0x80 | opcode, 0xfe]) + struct.pack("!H", size))
    return prefix + mask + bytes(v ^ mask[i % 4] for i, v in enumerate(payload))


def receive(sock):
    first, second = exact(sock, 2)
    assert first & 0xf0 == 0x80 and not second & 0x80, "invalid/interleaved server frame"
    size = second & 127
    if size == 126:
        size = struct.unpack("!H", exact(sock, 2))[0]
    elif size == 127:
        size = struct.unpack("!Q", exact(sock, 8))[0]
    assert size <= 4_000_000, "unexpected response bound"
    return first & 15, exact(sock, size)


def reply(sock, command):
    sock.sendall(frame(command))
    for _ in range(16):
        opcode, raw = receive(sock)
        assert opcode == 1
        value = json.loads(raw, parse_constant=lambda s: (_ for _ in ()).throw(ValueError(s)))
        if value.get("type") not in {"telemetry_invalidated", "telemetry_snapshot"}:
            return value
    raise AssertionError("too many unsolicited publications")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("executable", type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    timeout_ms = 400
    checks = []
    exe = str(args.executable.resolve())
    env = dict(os.environ, FTD_FORCE_CPU="1")
    for bad in [[], ["0"], ["99"], ["60001"], ["100x"], ["999999999999999999999"]]:
        result = subprocess.run([exe, "4", "0", "--client-timeout-ms", *bad],
            env=env, capture_output=True, timeout=15,
            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        assert result.returncode == 2, (bad, result.returncode)
    checks.append({"case": "six_invalid_timeout_arguments", "status": "passed"})
    with socket.socket() as reservation:
        reservation.bind(("127.0.0.1", 0))
        port = reservation.getsockname()[1]
    log_path = args.output.with_suffix(".server.log")
    timings = {}
    with log_path.open("wb") as log:
        process = subprocess.Popen([exe, "17", str(port), "--client-timeout-ms", str(timeout_ms)],
            env=env, stdout=log, stderr=subprocess.STDOUT,
            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        clients = []

        def connect(upgrade=True, small_buffer=False):
            sock = socket.socket()
            clients.append(sock)
            sock.settimeout(3)
            if small_buffer:
                sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 1024)
            sock.connect(("127.0.0.1", port))
            if upgrade:
                key = base64.b64encode(b"ftd-wire-audit!!?").decode()
                sock.sendall((f"GET / HTTP/1.1\r\nHost: 127.0.0.1:{port}\r\nUpgrade: websocket\r\n"
                    f"Connection: Upgrade\r\nSec-WebSocket-Key: {key}\r\n"
                    "Sec-WebSocket-Version: 13\r\n\r\n").encode())
                header = bytearray()
                while not header.endswith(b"\r\n\r\n"):
                    header.extend(exact(sock, 1))
                    assert len(header) < 8192
                assert b"101 Switching Protocols" in header
            return sock

        def physical(sock):
            info = reply(sock, {"cmd": "info"})
            assert info["backend"] == "cpu" and not info["restartRequired"]
            value = reply(sock, {"cmd": "get_dynamical_state_digest"})
            assert info["sourceEpoch"] == value["sourceEpoch"]
            return {key: value[key] for key in ["sourceEpoch", "tick", "stateVersion", "hashLo", "hashHi"]}

        def retired(sock, start, name, upper=2.0):
            try:
                data = sock.recv(1)
                assert not data, (name, "unexpected output", data)
            except (ConnectionResetError, ConnectionAbortedError):
                pass
            elapsed = time.monotonic() - start
            assert elapsed < upper, (name, elapsed)
            timings[name] = elapsed
            sock.close()

        def retained(expected, name):
            with connect() as sock:
                assert physical(sock) == expected, name
            checks.append({"case": name, "status": "passed"})

        try:
            deadline = time.monotonic() + 30
            while True:
                try:
                    sock = connect()
                    break
                except ConnectionRefusedError:
                    if process.poll() is not None or time.monotonic() >= deadline:
                        raise RuntimeError(f"server startup failed: {log_path}")
                    time.sleep(.05)
            with sock:
                sock.sendall(frame({"cmd": "inject_flux", "x": 3, "y": 3, "z": 3,
                                    "fx": .125, "fy": -.0625, "fz": .03125}))
                initial = physical(sock)
                time.sleep(timeout_ms / 1000 * 1.4)
                assert physical(sock) == initial
                checks.append({"case": "idle_upgraded_client_has_no_transaction_deadline", "status": "passed"})

            for name, prefix in [("silent_handshake", b""), ("partial_handshake", b"GET / HTTP/1.1\r\nHost:")]:
                sock = connect(False)
                start = time.monotonic()
                if prefix:
                    sock.sendall(prefix)
                retired(sock, start, name)
                retained(initial, name + "_retains_state")

            sock = connect(False)
            start = time.monotonic()
            for byte in b"GET / HT":
                try:
                    sock.sendall(bytes([byte]))
                except OSError:
                    break
                time.sleep(.07)
            retired(sock, start, "slow_drip_handshake", upper=.7)
            retained(initial, "slow_drip_handshake_retains_state")

            for name, data, half_close in [
                ("one_header_byte_eof", b"\x81", True),
                ("partial_command_eof", frame({"cmd": "tick"})[:-1], True),
                ("partial_command_timeout", frame({"cmd": "tick"})[:-1], False)]:
                sock = connect()
                start = time.monotonic()
                sock.sendall(data)
                if half_close:
                    sock.shutdown(socket.SHUT_WR)
                retired(sock, start, name)
                retained(initial, name + "_retains_state")

            sock = connect()
            start = time.monotonic()
            for byte in frame({"cmd": "tick"}):
                try:
                    sock.sendall(bytes([byte]))
                except OSError:
                    break
                time.sleep(.07)
                if time.monotonic() - start > .7:
                    break
            retired(sock, start, "slow_drip_frame", upper=.85)
            retained(initial, "slow_drip_frame_retains_state")

            # These prefixes must be rejected without waiting for a mask or body.
            bad_prefixes = {
                "oversized_ping": b"\x89\xfe\x00\x7e",
                "fragmented_control": b"\x09\x80",
                "fragmented_text": b"\x01\x80",
                "reserved_bits": b"\xc1\x80",
                "unmasked": b"\x81\x00",
                "unsupported_opcode": b"\x83\x80",
                "reserved_control": b"\x8b\x80",
                "nonminimal_16": b"\x81\xfe\x00\x01",
                "nonminimal_64": b"\x81\xff" + struct.pack("!Q", 126),
                "length_high_bit": b"\x81\xff" + struct.pack("!Q", 1 << 63),
                "over_input_cap": b"\x81\xff" + struct.pack("!Q", 65537),
                "one_byte_close": b"\x88\x81",
            }
            for name, prefix in bad_prefixes.items():
                sock = connect()
                start = time.monotonic()
                sock.sendall(prefix)
                retired(sock, start, name)
                # Prefix rejection is distinguished from the configured timeout.
                assert timings[name] < timeout_ms / 1000 * .8, (name, timings[name])
                retained(initial, name + "_retains_state")

            with connect() as sock:
                for body in [b"", bytes(range(125))]:
                    sock.sendall(frame(body, 9))
                    assert receive(sock) == (10, body)
                sock.sendall(frame(b"unsolicited", 10) + frame(b"ignored", 2))
                assert physical(sock) == initial
                raw = frame({"cmd": "info"})
                sock.sendall(raw[:1])
                time.sleep(.07)
                sock.sendall(raw[1:])
                assert json.loads(receive(sock)[1])["latticeSize"] == 17
                sock.sendall(frame({"cmd": "info"}) * 2)
                for _ in range(2):
                    assert json.loads(receive(sock)[1])["latticeSize"] == 17
                checks.append({"case": "valid_controls_tcp_chunks_and_ordered_coalesced_frames", "status": "passed"})

            # Completed command survives disconnect; trailing incomplete command never executes.
            with connect() as sock:
                ack = reply(sock, {"cmd": "tick"})
                assert ack["type"] == "tick_complete" and ack["tick"] == initial["tick"] + 1
                after_tick = physical(sock)
                sock.sendall(frame({"cmd": "tick"})[:-1])
                sock.shutdown(socket.SHUT_WR)
                retired(sock, time.monotonic(), "completed_then_partial")
            retained(after_tick, "completed_tick_retained_exactly_once")

            # Deliberately fill output, then queue a mutation that cannot be read
            # until every earlier response succeeds. A fresh handshake while the
            # old socket remains open proves the owner was released.
            old = connect(small_buffer=True)
            command = frame({"cmd": "get_flux_volume", "axisSamples": 17})
            log_before = log_path.read_bytes().count(b"Client network operation timed out")
            old.sendall(command * 192 + frame({"cmd": "tick"}))
            start = time.monotonic()
            first, length = exact(old, 2)
            assert first == 0x82 and length == 126
            payload_size = struct.unpack("!H", exact(old, 2))[0]
            assert payload_size > 16_000, payload_size
            time.sleep(.8)
            retained(after_tick, "blocked_output_releases_owner_and_does_not_execute_queued_tick")
            timings["backpressure_reconnect"] = time.monotonic() - start
            assert timings["backpressure_reconnect"] < 2.5
            assert log_path.read_bytes().count(b"Client network operation timed out") > log_before, "must observe actual send timeout"
            old.close()

            with connect() as sock:
                sock.sendall(frame({"cmd": "get_flux_volume", "axisSamples": 17}) + frame({"cmd": "info"}))
                opcode, payload = receive(sock)
                assert opcode == 2 and len(payload) == payload_size
                assert json.loads(receive(sock)[1])["latticeSize"] == 17
                assert physical(sock) == after_tick
                checks.append({"case": "complete_binary_response_precedes_next_json_frame", "status": "passed"})
                sock.sendall(frame(b"", 8))
                assert receive(sock) == (8, b"")
                retired(sock, time.monotonic(), "normal_close")

            # Warm every group, then arm a debounced edit and start an incomplete
            # request. Observation work becomes due during the frame read itself.
            with connect() as sock:
                assert reply(sock, {"cmd": "set_telemetry_demand", "mask": 15})["enabledMask"] == 15
                published = 0
                for _ in range(8):
                    opcode, raw = receive(sock)
                    value = json.loads(raw)
                    assert opcode == 1 and value["type"] == "telemetry_snapshot"
                    published |= value["publishedMask"]
                    if published == 15:
                        break
                assert published == 15
                time.sleep(.55)  # clear the slowest existing producer QoS interval
                query = frame({"cmd": "get_telemetry", "diagnostics": True, "audit": True,
                               "gravity": True, "lagrangian": True})
                sock.sendall(frame({"cmd": "inject_flux", "x": 3, "y": 3, "z": 3,
                                   "fx": .25, "fy": -.125, "fz": .0625}) + query[:1])
                opcode, raw = receive(sock)
                invalidation = json.loads(raw)
                assert opcode == 1 and invalidation["type"] == "telemetry_invalidated"
                time.sleep(.23)
                sock.sendall(query[1:])
                opcode, raw = receive(sock)
                cached = json.loads(raw)
                assert opcode == 1 and cached["type"] == "telemetry"
                assert cached["freshMask"] == 15 and cached["pendingMask"] == 0
                assert cached["sourceEpoch"] == after_tick["sourceEpoch"] and cached["tick"] == after_tick["tick"]
                assert cached["epoch"] == invalidation["epoch"]
                # Fresh cache before dispatch proves the wait hook ran. Queued
                # deltas follow the completed response and each group appears once.
                published = 0
                for _ in range(4):
                    opcode, raw = receive(sock)
                    value = json.loads(raw)
                    assert opcode == 1 and value["type"] == "telemetry_snapshot"
                    assert value["epoch"] == cached["epoch"] and value["sourceEpoch"] == cached["sourceEpoch"]
                    assert value["tick"] == cached["tick"] and not published & value["publishedMask"]
                    published |= value["publishedMask"]
                    if published == 15:
                        break
                assert published == 15
                after_observation = physical(sock)
                assert after_observation["tick"] == after_tick["tick"]
                checks.append({"case": "all_group_wait_pump_publishes_without_tick_or_interleaving", "status": "passed"})
                sock.sendall(b"\x81")
                retired(sock, time.monotonic(), "subscribed_partial_input_timeout")
            with connect() as sock:
                assert physical(sock) == after_observation
                demand = reply(sock, {"cmd": "set_telemetry_demand"})
                assert demand["enabledMask"] == 0
                cached = reply(sock, {"cmd": "get_telemetry"})
                assert cached["availableMask"] == 0 and cached["pendingMask"] == 0
                assert cached["epoch"] > invalidation["epoch"]
                checks.append({"case": "timeout_retires_all_old_demand_and_cache", "status": "passed"})
        finally:
            for sock in clients:
                sock.close()
            if process.poll() is None:
                process.terminate()
            process.wait(timeout=10)
    result = {"status": "passed", "backend": "forced_cpu", "lattice_size": 17,
              "configured_timeout_ms": timeout_ms, "checks": checks, "case_count": len(checks),
              "timings_seconds": timings,
              "scope": "actual TCP liveness, framing, scoped state retention and command ordering; no full replay, GPU or performance certification",
              "executable_sha256": hashlib.sha256(args.executable.read_bytes()).hexdigest()}
    args.output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result))


if __name__ == "__main__":
    main()
