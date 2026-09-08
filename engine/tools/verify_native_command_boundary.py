"""Bounded CPU WebSocket integration check using Python's independent JSON parser.

Starts and owns a temporary local server process; never attaches to a user's
running server. This verifies the production command boundary, not physics.
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


def strict_json(raw):
    def invalid_constant(value):
        raise ValueError(f"non-JSON constant: {value}")
    return json.loads(raw, parse_constant=invalid_constant)


def exact(sock, size):
    result = bytearray()
    while len(result) < size:
        part = sock.recv(size - len(result))
        if not part:
            raise RuntimeError("unexpected server disconnect")
        result.extend(part)
    return bytes(result)


def receive(sock):
    first, second = exact(sock, 2)
    assert first == 0x81 and not second & 128, "expected unmasked text response"
    size = second & 127
    if size == 126:
        size = struct.unpack("!H", exact(sock, 2))[0]
    elif size == 127:
        size = struct.unpack("!Q", exact(sock, 8))[0]
    assert size <= 1_000_000, "unexpected response size"
    return strict_json(exact(sock, size))


def send(sock, value):
    raw = value.encode() if isinstance(value, str) else json.dumps(value).encode()
    mask = b"\x12\x34\x56\x78"
    header = bytes([0x81, 0x80 | len(raw)]) if len(raw) < 126 else b"\x81\xfe" + struct.pack("!H", len(raw))
    sock.sendall(header + mask + bytes(v ^ mask[i % 4] for i, v in enumerate(raw)))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("executable", type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--telemetry-jsonl", type=Path)
    args = parser.parse_args()
    checks = 0
    if args.telemetry_jsonl:
        for raw in args.telemetry_jsonl.read_text(encoding="utf-8").splitlines():
            assert isinstance(strict_json(raw), dict)
            checks += 1
    with socket.socket() as reservation:
        reservation.bind(("127.0.0.1", 0))
        port = reservation.getsockname()[1]
    log_path = args.output.with_suffix(".server.log")
    env = dict(os.environ, FTD_FORCE_CPU="1")
    with log_path.open("wb") as log:
        process = subprocess.Popen([str(args.executable.resolve()), "4", str(port), "--once"],
            env=env, stdout=log, stderr=subprocess.STDOUT,
            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        try:
            deadline = time.monotonic() + 30
            while True:
                try:
                    sock = socket.create_connection(("127.0.0.1", port), timeout=2)
                    break
                except OSError:
                    if process.poll() is not None or time.monotonic() >= deadline:
                        raise RuntimeError(f"server startup failed; see {log_path}")
                    time.sleep(0.05)
            with sock:
                sock.settimeout(5)
                key = base64.b64encode(b"ftd-native-audit!").decode()
                sock.sendall((f"GET / HTTP/1.1\r\nHost: 127.0.0.1:{port}\r\nUpgrade: websocket\r\n"
                              f"Connection: Upgrade\r\nSec-WebSocket-Key: {key}\r\nSec-WebSocket-Version: 13\r\n\r\n").encode())
                header = bytearray()
                while not header.endswith(b"\r\n\r\n"):
                    header.extend(exact(sock, 1))
                    assert len(header) < 8192
                assert b"101 Switching Protocols" in header
                accept = base64.b64encode(hashlib.sha1((key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").encode()).digest())
                assert accept in header
                send(sock, {"cmd": "info"})
                info = receive(sock)
                assert info["latticeSize"] == 4 and info["backend"] == "cpu"
                send(sock, {"cmd": "get_dynamical_state_digest"})
                before = receive(sock)
                invalid = [
                    '{"cmd":"tick","cmd":"reset"}',
                    '{"cmd":"tick","_requestId":9007199254740990.9}',
                    '{"cmd":"set_toggle","name":"wave_propagation","value":"false"}',
                    '{"cmd":"inject_flux","x":2147483648,"y":0,"z":0,"fx":1,"fy":0,"fz":0}',
                    '{"cmd":"set_telemetry_demand","mask":1,"diagnostics":false}',
                    '{"cmd":"tick"}trailing',
                    '{"cmd":"invalid\\u0000\\ncommand"}',
                ]
                for raw in invalid:
                    send(sock, raw)
                    assert "error" in receive(sock)
                    send(sock, {"cmd": "get_dynamical_state_digest"})
                    assert receive(sock) == before, "rejection changed observed state/provenance"
                    checks += 1
                name = 'quote" newline\n nul\0 slash\\'
                send(sock, {"cmd": "apply_profile", "name": name, "_requestId": 9007199254740991})
                ack = receive(sock)
                assert ack["scenario"] == name and ack["_requestId"] == 9007199254740991
                checks += 1
                send(sock, {"cmd": "tick"})
                for _ in range(8):
                    reply = receive(sock)
                    if reply.get("type") == "tick_complete":
                        assert reply["tick"] == 1
                        break
                    assert reply.get("type") == "telemetry_invalidated"
                else:
                    raise AssertionError("missing completed tick response")
                checks += 1
        finally:
            if process.poll() is None:
                process.terminate()
            process.wait(timeout=10)
    result = {"status": "passed", "checks": checks, "backend": "forced_cpu",
              "scope": "independent JSON oracle and actual WebSocket command rejection; digest is scoped",
              "executable_sha256": hashlib.sha256(args.executable.read_bytes()).hexdigest()}
    args.output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result))


if __name__ == "__main__":
    main()
