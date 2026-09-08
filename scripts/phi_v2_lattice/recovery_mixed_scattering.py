"""Separately registered heterogeneous-probe intervention; previous protocol frozen."""
from __future__ import annotations
from dataclasses import asdict
from pathlib import Path
import json
import os
import subprocess
import time
from . import recovery_mixed_response as M
from . import channels as C, geometry as G, native_codec as N, staged as P

L = 17
HORIZON = 32
ARTIFACT_CAP = 1 << 30
DEFECTS = M.DEFECTS
ROOT = Path(__file__).resolve().parents[2]
RUNNER = ROOT / "engine/build_strict_mixed_scattering/ftd_strict_mixed_scattering_campaign"
_sha, _json, _linux = M._sha, M._json, M._linux
pair_history, verify_transition = M.pair_history, M.verify_transition


def cases() -> tuple[M.MixedCase, ...]:
    return tuple(M.MixedCase(f"scatter_{2*i+int(probe):02d}", defect, probe)
                 for i, defect in enumerate(DEFECTS) for probe in (False, True))


def prepare(case: M.MixedCase) -> P.StagedState:
    if not isinstance(case, M.MixedCase) or case not in cases():
        raise ValueError("unregistered heterogeneous preparation")
    # The previous relation preparation is reused without its common-flag fields.
    base = next(c for c in M.cases() if c.defect == case.defect and not c.probe)
    result = M.prepare(base)
    if case.probe:
        for x in range(7, 10):
            for y in range(7, 10):
                for z in range(7, 10):
                    result.lattice.bank[G.site_index(L,x,y,z),[0,32]] = True
    P.validate(result)
    return result


def registration() -> dict:
    return {"schema":"strict-mixed-scattering-prereg-1", "law_id":P.LAW_ID,
            "collision_sha256":C.COLLISION_HASH, "encoding_sha256":N.HASHES[1].hex(),
            "L":L, "boundary":"periodic", "horizon":HORIZON,
            "cases":[asdict(c) for c in cases()], "initial_s":0, "initial_ell":0,
            "background":"same homogeneous occupied relation background and four defects as mixed response v1",
            "defect_owner":[8,8,8], "sc_axis":0, "fcc_plane_diagonal":[0,1],
            "probe_channels":[0,32], "probe_sites":"{7,8,9}^3", "probe_tokens":54,
            "observables":list(N.NAMES),
            "causality":"all-record owner support, periodic Moore-1 per microtick",
            "boundary_claim":"periodic only; seam diagnostic is not a no-wrap certificate",
            "hypothesis":"different flags and actual collisions may permit remote relation-payload feedback",
            "artifact_cap_bytes":ARTIFACT_CAP, "GPU_timeout_seconds":1800,
            "M1_recovery_certified":False, "M2_recovery_certified":False}


def instrument_paths() -> set[str]:
    return M.instrument_paths() | {
        "scripts/phi_v2_lattice/recovery_mixed_scattering.py",
        "scripts/tests/phi_v2_lattice/test_recovery_mixed_scattering.py",
        "engine/docs/PREREG_STRICT_MIXED_SCATTERING.md",
        "engine/strict/recovery_mixed_scattering/CMakeLists.txt",
        "engine/strict/recovery_mixed_scattering/scattering_main.cpp"}


def prepare_campaign(directory) -> dict:
    directory = Path(directory)
    directory.mkdir(parents=True, exist_ok=True)
    if any(directory.iterdir()):
        raise ValueError("fresh empty campaign directory required")
    if not RUNNER.is_file():
        raise ValueError("build real CUDA mixed runner before locking")
    manifest = []
    lines = []
    for case in cases():
        path = directory / f"{case.case_id}.t00.bin"
        blob = N.encode(prepare(case))
        path.write_bytes(blob)
        manifest.append({"case": asdict(case), "initial_sha256": _sha(blob)})
        lines.append(case.case_id + "\t" + _linux(path))
    reg = registration()
    lock = {"registration": reg, "registration_sha256": _sha(_json(reg).encode()),
            "manifest": manifest, "manifest_sha256": _sha(_json(manifest).encode()),
            "instrument_sha256": {p: _sha((ROOT/p).read_bytes()) for p in sorted(instrument_paths())},
            "runner_sha256": _sha(RUNNER.read_bytes())}
    (directory / "manifest.tsv").write_text("\n".join(lines) + "\n", encoding="utf-8")
    (directory / "lock.json").write_text(_json(lock) + "\n", encoding="utf-8")
    return lock


def validate_lock(directory) -> dict:
    directory = Path(directory)
    lock = json.loads((directory / "lock.json").read_text())
    if set(lock["instrument_sha256"]) != instrument_paths():
        raise ValueError("incomplete imported source closure")
    if any(_sha((ROOT/p).read_bytes()) != h for p, h in lock["instrument_sha256"].items()):
        raise ValueError("frozen instrument source changed")
    if _sha(RUNNER.read_bytes()) != lock["runner_sha256"]:
        raise ValueError("frozen executable changed")
    if lock["registration"] != registration() or _sha(_json(lock["registration"]).encode()) != lock["registration_sha256"]:
        raise ValueError("scientific registration changed")
    if _sha(_json(lock["manifest"]).encode()) != lock["manifest_sha256"]:
        raise ValueError("preparation manifest changed")
    if [row["case"] for row in lock["manifest"]] != [asdict(c) for c in cases()]:
        raise ValueError("preparation matrix differs from registration")
    lines = []
    for case, row in zip(cases(), lock["manifest"], strict=True):
        path = directory / f"{case.case_id}.t00.bin"
        blob = path.read_bytes()
        if _sha(blob) != row["initial_sha256"] or blob != N.encode(prepare(case)):
            raise ValueError("preparation bytes differ from deterministic registered input")
        lines.append(case.case_id + "\t" + _linux(path))
    if (directory / "manifest.tsv").read_text() != "\n".join(lines) + "\n":
        raise ValueError("executable manifest path or order changed")
    if sum(p.stat().st_size for p in directory.iterdir() if p.is_file()) > ARTIFACT_CAP:
        raise ValueError("campaign artifact resource cap exceeded")
    return lock


def run_campaign(directory) -> dict:
    directory = Path(directory).resolve()
    lock = validate_lock(directory)
    if any((directory/name).exists() for name in ("trace.jsonl", "preflight.json", "execution.json")):
        raise ValueError("refuse to overwrite a started campaign")
    preflight = {"lock_sha256": _sha((directory / "lock.json").read_bytes()),
                 **{k: lock[k] for k in ("registration_sha256", "manifest_sha256", "instrument_sha256", "runner_sha256")}}
    (directory / "preflight.json").write_text(_json(preflight)+"\n", encoding="utf-8")
    args = [RUNNER, directory / "manifest.tsv", directory / "trace.jsonl"]
    command = (["wsl", "-d", "Ubuntu-22.04", "--"] + [_linux(p) for p in args]
               if os.name == "nt" else [str(p) for p in args])
    started = time.perf_counter()
    result = subprocess.run(command, capture_output=True, text=True, timeout=1800)
    elapsed = time.perf_counter() - started
    (directory / "execution.stdout.txt").write_text(result.stdout, encoding="utf-8")
    (directory / "execution.stderr.txt").write_text(result.stderr, encoding="utf-8")
    if result.returncode:
        raise RuntimeError("GPU mixed campaign failed; partial evidence retained: " + result.stderr)
    if validate_lock(directory) != lock or preflight["lock_sha256"] != _sha((directory / "lock.json").read_bytes()):
        raise ValueError("frozen provenance changed during campaign")
    devices = [json.loads(line) for line in result.stderr.splitlines() if line.startswith('{')]
    if len(devices) != 1 or devices[0].get("backend") != "cuda_device_kernels" or "RTX 5090" not in devices[0].get("name", ""):
        raise ValueError("required actual RTX 5090 CUDA device evidence absent")
    receipt = {"schema": "strict-mixed-scattering-execution-1", "device": devices[0], "elapsed_seconds": elapsed,
               "physical_microticks": len(cases())*HORIZON,
               "preflight_sha256": _sha((directory / "preflight.json").read_bytes()),
               "trace_sha256": _sha((directory / "trace.jsonl").read_bytes()),
               "postflight": "complete frozen input/source validation passed"}
    (directory / "execution.json").write_text(_json(receipt)+"\n", encoding="utf-8")
    return receipt


def audit_campaign(directory, progress=None) -> dict:
    directory = Path(directory)
    lock = validate_lock(directory)
    preflight_bytes = (directory / "preflight.json").read_bytes()
    preflight = json.loads(preflight_bytes)
    receipt = json.loads((directory / "execution.json").read_bytes())
    if (receipt["preflight_sha256"] != _sha(preflight_bytes)
        or preflight["lock_sha256"] != _sha((directory / "lock.json").read_bytes())):
        raise ValueError("receipt chain not bound to accepted lock")
    if any(preflight[k] != lock[k] for k in ("registration_sha256", "manifest_sha256", "instrument_sha256", "runner_sha256")):
        raise ValueError("preflight identity differs from lock")
    trace_bytes = (directory / "trace.jsonl").read_bytes()
    if receipt["trace_sha256"] != _sha(trace_bytes) or receipt["physical_microticks"] != len(cases())*HORIZON:
        raise ValueError("trace or executed tick count mismatch")
    if (receipt["postflight"] != "complete frozen input/source validation passed"
        or receipt["device"].get("backend") != "cuda_device_kernels"
        or "RTX 5090" not in receipt["device"].get("name", "")):
        raise ValueError("required executed device/postflight evidence absent")
    traces = [json.loads(line) for line in trace_bytes.splitlines()]
    histories = {}
    evidence = []
    for case, trace in zip(cases(), traces, strict=True):
        if trace["case_id"] != case.case_id or trace["native_complete_parity"] is not True:
            raise ValueError("case identity or native parity missing")
        if len(trace["states"]) != HORIZON+1 or len(trace["events"]) != HORIZON:
            raise ValueError("incomplete per-tick evidence")
        history = []
        for tick, row in enumerate(trace["states"]):
            name = f"{case.case_id}.t{tick:02d}.bin"
            if row["file"] != name or row["microtick"] != tick:
                raise ValueError("snapshot file or clock mismatch")
            blob = (directory / name).read_bytes()
            if _sha(blob) != row["sha256"]:
                raise ValueError("complete snapshot hash mismatch")
            state = N.decode(blob)
            if state.microtick != tick or state.lattice.L != L or P.work_units(state) != row["work"]:
                raise ValueError("decoded state metadata mismatch")
            if tick == 0:
                if blob != N.encode(prepare(case)):
                    raise ValueError("GPU input differs from registered preparation")
            else:
                verify_transition(history[-1], state, trace["events"][tick-1])
            history.append(state)
        histories[case.case_id] = history
        evidence.append({"case": asdict(case), "checkpoints": trace["states"],
                         "event_counts": [{k: len(v) for k, v in ev.items()} for ev in trace["events"]],
                         "native_and_Python_complete_parity": True})
        if progress is not None:
            progress(case.case_id)
    pairs = []
    for i, defect in enumerate(DEFECTS):
        a, b = cases()[2*i:2*i+2]
        rows = pair_history(histories[a.case_id], histories[b.case_id])
        pairs.append({"kind": "field_to_relation", "defect": defect, "left": a.case_id, "right": b.case_id,
                      "relation_payload_response": any(row["hamming"]["sc"]+row["hamming"]["fcc"] for row in rows), "ticks": rows})
        if i:
            for probe in (False, True):
                reference = cases()[int(probe)]
                altered = cases()[2*i+int(probe)]
                rows = pair_history(histories[reference.case_id], histories[altered.case_id])
                center = G.site_index(L, 8, 8, 8)
                pairs.append({"kind": "relation_to_field" if probe else "field_free_defect_control",
                              "defect": defect, "left": reference.case_id, "right": altered.case_id,
                              "field_bank_response": any(row["hamming"]["bank"] for row in rows),
                              "remote_relation_payload_response": any(any(owner != center for owner in row["relation_payload_owners"]) for row in rows),
                              "ticks": rows})
    report = {"schema": "strict-mixed-scattering-report-1", "registration": lock["registration"],
              "registration_sha256": lock["registration_sha256"], "manifest_sha256": lock["manifest_sha256"],
              "runner_sha256": lock["runner_sha256"], "instrument_sha256": lock["instrument_sha256"],
              "execution": receipt, "case_evidence": evidence, "paired_interventions": pairs,
              "M1_recovery_certified": False, "M2_recovery_certified": False,
              "continuum_recovery_certified": False, "undefined_boundary_certified": False}
    validate_lock(directory)
    (directory / "report.json").write_text(_json(report)+"\n", encoding="utf-8")
    return report
