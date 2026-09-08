"""Independent real-wire checks for native reference observation protocol v3."""
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

from verify_native_transport_liveness import exact, frame, receive


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("executable", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with socket.socket() as reservation:
        reservation.bind(("127.0.0.1", 0))
        port = reservation.getsockname()[1]
    checks = []
    log_path = args.output.with_suffix(".server.log")
    with log_path.open("wb") as log:
        process = subprocess.Popen([str(args.executable.resolve()), "7", str(port)],
            env=dict(os.environ, FTD_FORCE_CPU="1"), stdout=log, stderr=subprocess.STDOUT,
            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        sock = None
        try:
            deadline = time.monotonic() + 30
            while True:
                try:
                    sock = socket.create_connection(("127.0.0.1", port), timeout=3)
                    break
                except ConnectionRefusedError:
                    if process.poll() is not None or time.monotonic() >= deadline:
                        raise RuntimeError(f"startup failed: {log_path}")
                    time.sleep(.05)
            key = base64.b64encode(b"ftd-v3-observe!!?").decode()
            sock.sendall((f"GET / HTTP/1.1\r\nHost: 127.0.0.1:{port}\r\nUpgrade: websocket\r\n"
                f"Connection: Upgrade\r\nSec-WebSocket-Key: {key}\r\nSec-WebSocket-Version: 13\r\n\r\n").encode())
            header = bytearray()
            while not header.endswith(b"\r\n\r\n"):
                header.extend(exact(sock, 1))
                assert len(header) < 8192
            assert b"101 Switching Protocols" in header
            request_id = 0

            def response():
                for _ in range(16):
                    opcode, payload = receive(sock)
                    if opcode == 1:
                        value = json.loads(payload)
                        if value.get("type") in {"telemetry_snapshot", "telemetry_invalidated"}:
                            continue
                        return value
                    assert opcode == 2
                    return payload
                raise AssertionError("unexpected publication loop")

            def request(command, wide=False):
                nonlocal request_id
                request_id += 1
                identity = 9007199254740991 if wide else request_id
                sock.sendall(frame(dict(command, _requestId=identity)))
                while True:
                    value = response()
                    if isinstance(value, bytes):
                        assert command.get("_binaryVersion") == 3
                        assert struct.unpack_from("<Q", value, 8)[0] == identity
                        return value
                    assert value["_requestId"] == identity, (command, value)
                    if value.get("type") == "operation_progress":
                        continue
                    assert not value.get("error"), (command, value)
                    return value

            info = request({"cmd": "info"})
            assert info["backend"] == "cpu" and info["nativeProtocolVersion"] == 3
            assert info["nativeBinaryVersions"] == [2, 3]
            assert info["exactIntegerEncoding"] == "safe-number-or-decimal-string"
            instance = info["nativeInstanceId"]
            assert len(instance) == 32 and int(instance, 16) != 0
            checks.append("capabilities_and_instance_namespace")
            request({"cmd": "inject_particle", "x": 3, "y": 3, "z": 3,
                     "state": 1, "fx": .125, "fy": -.0625, "fz": .03125})
            before = request({"cmd": "get_dynamical_state_digest"})
            source = before["sourceEpoch"]
            header_format = "<IIQQQQQddIIQQ"
            assert struct.calcsize(header_format) == 88
            samples = [
                {"cmd": "get_particles"},
                {"cmd": "get_flux_volume", "axisSamples": 7},
                {"cmd": "get_field_sample", "kind": "e", "stride": 1, "token": 123},
                {"cmd": "get_field_sample", "kind": "divJ", "stride": 1, "token": 124},
                {"cmd": "get_field_slices", "kind": "fluxVector", "stride": 1, "token": 125, "mid": 3},
            ]
            epochs = set()
            for command in samples:
                sock.sendall(frame(command))
                legacy = response()
                assert isinstance(legacy, bytes)
                sample = request(dict(command, _binaryVersion=3), wide=True)
                values = struct.unpack_from(header_format, sample)
                magic, size, rid, tick, sampled_source, epoch, length, physical_time, dt, lattice, flags, lo, hi = values
                assert magic == 0x334e5446 and size == 88 and rid == 9007199254740991
                assert tick == before["tick"] and sampled_source == source and lattice == 7 and flags == 1
                assert length == len(legacy) and sample[88:] == legacy
                assert f"{hi:016x}{lo:016x}" == instance and physical_time == 0 and dt > 0
                epochs.add(epoch)
                checks.append("exact_legacy_payload_and_provenance_" + command["cmd"] + "_" + command.get("kind", ""))
            assert len(epochs) == 1
            assert request({"cmd": "get_dynamical_state_digest"})["hashLo"] == before["hashLo"]
            checks.append("observation_preserves_scoped_dynamical_digest")

            # Observe the same tick after a direct edit: the sample epoch must change.
            request({"cmd": "inject_flux", "x": 2, "y": 3, "z": 3,
                     "fx": .25, "fy": 0, "fz": 0})
            edited = request({"cmd": "get_particles", "_binaryVersion": 3})
            assert struct.unpack_from("<Q", edited, 16)[0] == before["tick"]
            assert struct.unpack_from("<Q", edited, 32)[0] > next(iter(epochs))
            checks.append("same_tick_direct_edit_changes_sample_epoch")

            # The default reference step intentionally clamps dt below one.
            # Select its existing fractional-step integrator before this probe.
            request({"cmd": "set_toggle", "name": "symplectic_leapfrog", "value": True})
            request({"cmd": "set_param", "name": "dt", "value": .125})
            for command, kind in [({"cmd": "tick"}, "tick_complete"),
                                  ({"cmd": "run", "n": 1}, "run_complete")]:
                answer = request(command, wide=True)
                assert answer["type"] == kind
                assert answer["sourceEpoch"] == source and answer["nativeInstanceId"] == instance
                checks.append("correlated_" + kind)
            advanced = request({"cmd": "get_particles", "_binaryVersion": 3})
            assert struct.unpack_from("<Q", advanced, 16)[0] == 2
            assert struct.unpack_from("<Q", advanced, 24)[0] == source
            assert struct.unpack_from("<dd", advanced, 48) == (.25, .125), struct.unpack_from("<dd", advanced, 48)
            advanced_info = request({"cmd": "info"})
            assert struct.unpack_from("<Q", advanced, 32)[0] == advanced_info["telemetryEpoch"]
            assert advanced_info["nativeInstanceId"] == instance
            checks.append("advanced_tick_time_and_dt_are_sampled_from_actual_state")
            for command in [{"cmd": "get_flux_slice", "axis": 0, "index": 3},
                            {"cmd": "inspect_voxel", "x": 3, "y": 3, "z": 3},
                            {"cmd": "get_force_at", "x": 3, "y": 3, "z": 3}]:
                answer = request(command, wide=True)
                assert answer["sampleTick"] == 2 and answer["sourceEpoch"] == source
                assert answer["nativeInstanceId"] == instance
                checks.append("correlated_observation_" + command["cmd"])

            for bad in [{"cmd": "tick", "_binaryVersion": 4, "_requestId": 801},
                        {"cmd": "tick", "_binaryVersion": 2.5, "_requestId": 802},
                        {"cmd": "get_particles", "_binaryVersion": 3},
                        {"cmd": "get_particles", "_requestId": 803},
                        {"cmd": "get_flux_volume", "_binaryVersion": 2, "_requestId": 804},
                        {"cmd": "tick", "_requestId": 9007199254740992}]:
                sock.sendall(frame(bad))
                answer = response()
                assert isinstance(answer, dict) and answer.get("error")
                if bad.get("_requestId", 0) in {801, 802, 803, 804}:
                    assert answer["_requestId"] == bad["_requestId"]
            assert request({"cmd": "get_dynamical_state_digest"})["tick"] == 2
            checks.append("invalid_version_or_identity_rejects_before_tick")

            replacement = request({"cmd": "resize", "size": 7})
            assert replacement["sourceEpoch"] > source
            new_info = request({"cmd": "info"})
            assert new_info["nativeInstanceId"] == instance
            replaced = request({"cmd": "get_particles", "_binaryVersion": 3})
            assert struct.unpack_from("<Q", replaced, 16)[0] == 0
            assert struct.unpack_from("<Q", replaced, 24)[0] == replacement["sourceEpoch"]
            checks.append("correlated_progress_and_equal_tick_source_replacement")
        finally:
            if sock:
                sock.close()
            if process.poll() is None:
                process.terminate()
            process.wait(timeout=10)
    result = {"status": "passed", "case_count": len(checks), "cases": checks,
              "backend": "forced_cpu", "lattice_size": 7,
              "executable_sha256": hashlib.sha256(args.executable.read_bytes()).hexdigest(),
              "scope": "independent native wire layout, sampled state provenance, precision and correlation; not full replay or physics"}
    args.output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result))


if __name__ == "__main__":
    main()
