"""Fixed complete-state experiment for weak continuum conservation, not fluid closure."""
from __future__ import annotations

import argparse
import base64
from dataclasses import asdict
from datetime import datetime, timezone
from fractions import Fraction as F
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import time
import zlib

import numpy as np

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "scripts"))
from phi_v2_lattice import geometry as G  # noqa: E402
from phi_v2_lattice.hydro import channels as H, codec, prepare, staged, state  # noqa: E402
from phi_v2_lattice.hydro import weak_balance as W  # noqa: E402

SCHEMA = "ftd-hydro-weak-balance-registration-v1"
CASES = tuple((L, background) for L in (4, 8) for background in ("blank", "frozen"))
RUNTIME = {
    "module": "engine/build_strict_hydro_wasm/ftd_hydro_wasm.mjs",
    "binary": "engine/build_strict_hydro_wasm/ftd_hydro_wasm.wasm",
    "table": "engine/build_strict_hydro_tables/hydro_collision_abf25cf26072c03b.u32",
    "node": "engine/build_predictive_response/tools/node-v24.11.0-linux-x64/bin/node",
}
DOC = "engine/docs/PREREG_STRICT_WEAK_CONTINUUM_V1.md"
WEIGHTS = tuple((1, *v) for v in H.VELOCITIES)
AXES = (1, 2, 3)
EXPERIMENT = dict(Ls=[4, 8], backgrounds=["blank", "frozen"], cycles="L", max_runtime_seconds=60)
COMPARISON = dict(h="1/L", c_ref="1", cycle_duration="h", microtick_duration="h/4",
                  horizon="1", test="(1+t)*sum_a (a+1)*u_a^2*(1-u_a)^2, periodic C2",
                  source_left="cycle start", source_event="cycle start+h/4",
                  source_event_correction="exact -h/4 times the spatial weak source")


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def serial(value):
    if isinstance(value, F):
        return str(value)
    if isinstance(value, dict):
        return {key: serial(item) for key, item in value.items()}
    if isinstance(value, (list, tuple)):
        return [serial(item) for item in value]
    return value


def write_new(path, value):
    with Path(path).open("x", encoding="utf-8", newline="\n") as stream:
        json.dump(serial(value), stream, indent=2, sort_keys=True, allow_nan=False)
        stream.write("\n")


def _unique(pairs):
    obj = {}
    for key, value in pairs:
        if key in obj:
            raise ValueError("duplicate JSON field")
        obj[key] = value
    return obj


def read_json(path):
    return json.loads(Path(path).read_bytes(), object_pairs_hook=_unique)


def canonical(value):
    return json.dumps(serial(value), sort_keys=True, separators=(",", ":"), allow_nan=False)


def native_events(event):
    return dict(
        absorptions=[[x, c, 0 if kind == "sc" else 1, owner, idx[0], idx[1] if len(idx) > 1 else 0]
                     for x, c, kind, owner, idx in event["absorptions"]],
        collisions=[[site, 1 if pol == 0 else -1, mask, out]
                    for site, pol, mask, out in event["collisions"]],
        crossings=[[kind, owner, list(idx), direction] for kind, owner, idx, direction in event["crossings"]],
        gate_holds=[[kind, owner, list(idx)] for kind, owner, idx in event["gate_holds"]])


def polynomial(L):
    """Exact periodic C2 test values, gradients, and diagonal Hessian samples."""
    values, gradients, hessians = [], [], []
    for x in range(L**3):
        u = tuple(F(q, L) for q in G.coords(L, x))
        values.append(sum(a*q*q*(1-q)**2 for a, q in zip(AXES, u)))
        gradients.append(tuple(a*(2*q-6*q*q+4*q**3) for a, q in zip(AXES, u)))
        hessians.append(tuple(a*(2-12*q+12*q*q) for a, q in zip(AXES, u)))
    return tuple(values), tuple(gradients), tuple(hessians)


def caps():
    return tuple(2*sum(abs(w[k])*sum(a*abs(q) for a, q in zip(AXES, v))
                       for w, v in zip(WEIGHTS, H.VELOCITIES)) for k in range(4))


def prepare_inputs(directory):
    directory = Path(directory)
    directory.mkdir(parents=True, exist_ok=True)
    targets = [directory / f"L{L}-{b}.json" for L, b in CASES] + [directory / "input.json"]
    if any(path.exists() for path in targets):
        raise FileExistsError("fixed input already exists; overwriting is forbidden")
    cases = []
    for L, background in CASES:
        lattice = state.blank(L) if background == "blank" else prepare.frozen_background(L)
        for site in range(L**3):
            for pol in range(2):
                token = f"ftd-weak-balance-v1:L:{L}:site:{site}:polarity:{pol}"
                mask = int.from_bytes(hashlib.sha256(token.encode("ascii")).digest()[:3], "little")
                for v in range(24):
                    lattice.bank[site, H.channel(pol, 0, v)] = bool(mask >> v & 1)
        data = codec.checkpoint(staged.initialize(lattice))
        filename = f"L{L}-{background}.json"
        with (directory / filename).open("xb") as stream:
            stream.write(data)
        cases.append(dict(id=f"L{L}-{background}", L=L, cycles=L, background=background,
                          checkpoint_file=filename, checkpoint_sha256=sha(directory / filename)))
    result = dict(schema="ftd-hydro-weak-balance-input-v1", cases=cases,
                  preparation="fixed SHA256-derived masks; no statistical sampling claim",
                  created_utc=datetime.now(timezone.utc).isoformat())
    write_new(directory / "input.json", result)
    return {"input": str(directory / "input.json"), "cases": len(cases)}


def required_sources(directory):
    inputs = read_json(directory / "input.json")
    if [(c["L"], c["background"]) for c in inputs["cases"]] != list(CASES):
        raise ValueError("fixed case coverage changed")
    paths = list((ROOT / "scripts/phi_v2_lattice").rglob("*.py"))
    paths += list((ROOT / "engine/strict/hydro").glob("*.cpp"))
    paths += list((ROOT / "engine/strict/hydro").glob("*.h"))
    paths += [ROOT / "engine/strict/frozen_tables.h", ROOT / DOC,
              ROOT / "engine/docs/DERIV_STRICT_WEAK_CONTINUUM_BALANCE_V1.md",
              ROOT / "engine/strict/hydro/weak_balance_runner.mjs",
              ROOT / "scripts/tests/phi_v2_lattice/hydro/test_weak_balance.py",
              ROOT / "scripts/tests/phi_v2_lattice/hydro/test_weak_continuum.py"]
    paths += [ROOT / name for name in RUNTIME.values()]
    # These proof imports supply the unchanged relation functions through _proofs.
    for module in tuple(sys.modules.values()):
        filename = getattr(module, "__file__", None)
        if filename:
            path = Path(filename).resolve()
            if path.is_relative_to(ROOT / "scripts/proofs") and path.suffix == ".py":
                paths.append(path)
    paths += [directory / "input.json"] + [directory / row["checkpoint_file"] for row in inputs["cases"]]
    return sorted(set(p.resolve().relative_to(ROOT).as_posix() for p in paths))


def acceptance():
    return dict(exact_balances=True, all_8_state_arrays_reproduced=True,
                all_actual_events_reproduced=True, frozen_sector_absorptions=0,
                blank_first_two_cycles_absorptions=0, blank_later_absorption_witness=True,
                spacetime_orders=[1, 2], complete_error_bounds="state-weighted rational Taylor bounds",
                uniform_caps=caps(), first_uniform_envelope="2*M*h",
                second_uniform_envelope="4*M*h^2", no_adaptive_retry=True)


def verify_pins(lock):
    if (lock.get("schema") != SCHEMA or type(lock.get("sources")) is not dict
            or not lock["sources"] or type(lock.get("input_file")) is not str):
        raise ValueError("foreign or incomplete registration")
    expected_law = dict(id=staged.LAW_ID, table_sha256=H.TABLE_HASH,
                        encoding_sha256=H.ENCODING_HASH, blank=4, velocities=H.VELOCITIES)
    for key, value in (("law", expected_law), ("runtime", RUNTIME), ("experiment", EXPERIMENT),
                       ("comparison", COMPARISON), ("acceptance", acceptance())):
        if canonical(lock.get(key)) != canonical(value):
            raise ValueError(f"registered {key} changed")
    input_path = (ROOT / lock["input_file"]).resolve()
    if not input_path.is_relative_to(ROOT) or input_path.name != "input.json":
        raise ValueError("foreign input path")
    required = required_sources(input_path.parent)
    if not set(required).issubset(lock["sources"]):
        raise ValueError("registration omits required source or input closure")
    if (lock["sources"][RUNTIME["table"]] != H.TABLE_HASH
            or lock["sources"][lock["input_file"]] != lock["input_sha256"]):
        raise ValueError("table or input identity mismatch")
    for name, digest in lock["sources"].items():
        path = (ROOT / name).resolve()
        if not path.is_relative_to(ROOT) or sha(path) != digest:
            raise ValueError(f"changed or foreign pinned file: {name}")


def register(directory):
    directory = Path(directory).resolve()
    sources = {name: sha(ROOT / name) for name in required_sources(directory)}
    if sources[RUNTIME["table"]] != H.TABLE_HASH or caps() != (144, 64, 80, 96):
        raise ValueError("law identity or finite moment cap changed")
    lock = dict(schema=SCHEMA, registered_utc=datetime.now(timezone.utc).isoformat(),
                base_revision=subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
                law=dict(id=staged.LAW_ID, table_sha256=H.TABLE_HASH,
                         encoding_sha256=H.ENCODING_HASH, blank=4, velocities=H.VELOCITIES),
                runtime=RUNTIME, sources=sources, input_sha256=sha(directory / "input.json"),
                input_file=(directory / "input.json").relative_to(ROOT).as_posix(),
                experiment=EXPERIMENT, comparison=COMPARISON, acceptance=acceptance(),
                claim="exact source-aware weak conservation and finite-spacing consistency; unresolved stress; no Navier-Stokes closure",
                environment=dict(python=sys.version, numpy=np.__version__),
                registration_scope="local hash freeze before compiled outcomes, no external timestamp authority")
    verify_pins(lock)
    write_new(directory / "registration.json", lock)
    result = {"sha256": sha(directory / "registration.json")}
    write_new(directory / "registration-digest.json", result)
    return result


def decode_stage(record):
    if record.get("checkpoint_codec") != "deflate-raw-base64":
        raise ValueError("unknown stage encoding")
    compressed = base64.b64decode(record["checkpoint_deflate_raw_base64"], validate=True)
    decoder = zlib.decompressobj(-15)
    data = decoder.decompress(compressed, 512001)
    if len(data) > 512000 or not decoder.eof or decoder.unused_data or decoder.unconsumed_tail:
        raise ValueError("incomplete or oversized checkpoint compression")
    if len(data) != record["checkpoint_bytes"] or hashlib.sha256(data).hexdigest() != record["checkpoint_sha256"]:
        raise ValueError("checkpoint identity mismatch")
    st = codec.restore(data)
    if record["microtick"] != str(st.microtick):
        raise ValueError("stage clock mismatch")
    arrays = codec._arrays(st)
    actual = {name: hashlib.sha256(a.tobytes()).hexdigest() for name, a in zip(codec.NAMES, arrays)}
    if record["array_sha256"] != actual:
        raise ValueError("complete-array hash mismatch")
    obs = W.observe_checkpoint(data)
    fields = obs.field_tokens
    sc = int(np.count_nonzero(st.lattice.sc != state.BLANK_IDX))
    fcc = int(np.count_nonzero(st.lattice.fcc != state.BLANK_IDX))
    ledger = dict(field_tokens=str(sum(fields)), field_tokens_by_polarity=list(map(str, fields)),
                  field_momentum_by_polarity=[[str(sum(row[k] for row in bank)) for k in range(1, 4)]
                                              for bank in obs.moments],
                  relation_tokens_sc=str(sc), relation_tokens_fcc=str(fcc), work_units=str(obs.total_tokens))
    if record["ledger"] != ledger:
        raise ValueError("stage ledger mismatch")
    return data, st


def same_state(left, right):
    if left.microtick != right.microtick or left.lattice.L != right.lattice.L:
        raise ValueError("reproduced clock or shape mismatch")
    for name, a, b in zip(codec.NAMES, codec._arrays(left), codec._arrays(right)):
        if not np.array_equal(a, b):
            raise ValueError(f"independent Python/WASM state mismatch: {name}")


def spacetime_cycle(initial, prestream, final):
    old, pre, new = (codec.restore(data) for data in (initial, prestream, final))
    L, h = old.lattice.L, F(1, old.lattice.L)
    t, tau = F(old.microtick, 4*L), h
    psi, gradient, hessian = polynomial(L)
    balance = W.certify_cycle(initial, prestream, final, psi, cell_volume=h**3)
    snapshots = tuple(W.observe_checkpoint(data) for data in (initial, final))
    occ = pre.lattice.bank.reshape(-1, 2, 4, 24).sum(axis=(1, 2), dtype=np.int16)
    endpoint = [h**3*sum(((1+t+tau)*sum(snapshots[1].moments[p][x][k] for p in range(2))
                         -(1+t)*sum(snapshots[0].moments[p][x][k] for p in range(2)))*psi[x]
                        for x in range(L**3)) for k in range(4)]
    spatial_source = [sum(balance.weak_source[p][k] for p in range(2)) for k in range(4)]
    source_left = [(1+t)*v for v in spatial_source]
    source_event = [(1+t+tau/4)*v for v in spatial_source]
    event_correction = [-tau*v/4 for v in spatial_source]
    exact, linear, quadratic, budget1, budget2 = ([F(0) for _ in range(4)] for _ in range(5))
    for x in range(L**3):
        for c, v in enumerate(H.VELOCITIES):
            count = int(occ[x, c])
            if not count:
                continue
            y = G.shift(L, x, v)
            dg = sum(a*b for a, b in zip(v, gradient[x]))
            d2g = sum(a*a*b for a, b in zip(v, hessian[x]))
            coefficient = sum(a*abs(b) for a, b in zip(AXES, v))
            first = tau*psi[x]+(1+t)*h*dg
            second = first+tau*h*dg+(1+t)*h*h*d2g/2
            r1 = ((1+t)*h*h+tau*h/2)*coefficient
            r2 = (2*(1+t)*h**3+tau*h*h)*coefficient
            for k, weight in enumerate(WEIGHTS[c]):
                factor = h**3*count*weight
                absolute = h**3*count*abs(weight)
                exact[k] += factor*((1+t+tau)*psi[y]-(1+t)*psi[x])
                linear[k] += factor*first
                quadratic[k] += factor*second
                budget1[k] += absolute*r1
                budget2[k] += absolute*r2
    if any(endpoint[k] != source_left[k]+exact[k]
           or source_left[k] != source_event[k]+event_correction[k] for k in range(4)):
        raise ValueError("exact spacetime identity failed")
    residual1 = [exact[k]-linear[k] for k in range(4)]
    residual2 = [exact[k]-quadratic[k] for k in range(4)]
    if any(abs(residual1[k]) > budget1[k] or abs(residual2[k]) > budget2[k] for k in range(4)):
        raise ValueError("registered spacetime Taylor bound failed")
    absorbed = sum(row[0] for bank in balance.absorbed_moments for row in bank)
    return dict(initial_microtick=old.microtick, final_microtick=new.microtick, h=h, t=t,
                endpoint=endpoint, source_left=source_left, source_event=source_event,
                source_event_correction=event_correction, exact_transport=exact,
                first_residual=residual1, second_residual=residual2,
                first_bound=budget1, second_bound=budget2, absorbed_tokens=absorbed,
                exact_balance=True, taylor_bounds_pass=True, conservative_sector=balance.conservative_sector)


def audit(registration, input_path, wasm_path, output):
    started = time.monotonic()
    paths = tuple(map(Path, (registration, input_path, wasm_path)))
    hashes = tuple(sha(p) for p in paths)
    lock, inputs, wasm = map(read_json, paths)
    verify_pins(lock)
    if paths[1].resolve() != (ROOT / lock["input_file"]).resolve():
        raise ValueError("foreign input location")
    if (wasm.get("schema") != "ftd-hydro-weak-balance-wasm-v1"
            or wasm["registration_sha256"] != hashes[0] or wasm["input_sha256"] != hashes[1]
            or lock["input_sha256"] != hashes[1]):
        raise ValueError("foreign result or input lineage")
    if ([(row["L"], row["background"]) for row in inputs["cases"]] != list(CASES)
            or [row["id"] for row in wasm["cases"]] != [row["id"] for row in inputs["cases"]]):
        raise ValueError("case coverage mismatch")
    expected_backend = dict(platform="linux", arch="x64", node_version="v24.11.0", runtime="compiled-wasm-cpu",
                            node_sha256=lock["sources"][RUNTIME["node"]],
                            module_sha256=lock["sources"][RUNTIME["module"]],
                            binary_sha256=lock["sources"][RUNTIME["binary"]], table_sha256=H.TABLE_HASH,
                            actual_advance_calls=48, actual_microticks=96, completed_cases=4)
    if (canonical(wasm.get("backend")) != canonical(expected_backend)
            or wasm.get("source_postchecks") is not True or wasm.get("input_postchecks") is not True):
        raise ValueError("backend provenance or postchecks mismatch")
    table = H.load_table()
    rows, stage_count, event_count = [], 0, 0
    for specification, observed in zip(inputs["cases"], wasm["cases"]):
        source = paths[1].parent / specification["checkpoint_file"]
        if sha(source) != specification["checkpoint_sha256"]:
            raise ValueError("initial preparation changed")
        reference = codec.restore(source.read_bytes())
        previous, actual = decode_stage(observed["initial"])
        same_state(reference, actual)
        stage_count += 1
        if specification["background"] == "frozen":
            W.admit_conservative_sector(previous)
        if len(observed["cycles"]) != specification["cycles"]:
            raise ValueError("cycle coverage mismatch")
        cycles = []
        for number, row in enumerate(observed["cycles"], 1):
            if (row["cycle"] != number
                    or row["initial_checkpoint_sha256"] != hashlib.sha256(previous).hexdigest()):
                raise ValueError("state lineage or cycle ordinal mismatch")
            stage_data = []
            for label, event_key in (("prestream", "first_half_events"), ("final", "second_half_events")):
                events = dict(absorptions=[], collisions=[], crossings=[], gate_holds=[])
                for _ in range(2):
                    reference, event = staged.step(reference, table)
                    for name, values in asdict(event).items():
                        events[name].extend(values)
                data, actual = decode_stage(row[label])
                same_state(reference, actual)
                normalized = native_events(events)
                if canonical(row[event_key]) != canonical(normalized):
                    raise ValueError(f"independent event mismatch at {specification['id']} cycle {number}/{label}")
                event_count += sum(len(values) for values in events.values())
                stage_count += 1
                stage_data.append(data)
            cycle = spacetime_cycle(previous, *stage_data)
            if ((number <= 2 or specification["background"] == "frozen") and cycle["absorbed_tokens"] != 0):
                raise ValueError("registered conservative preparation condition failed")
            cycles.append(cycle)
            previous = stage_data[1]
        h = F(1, specification["L"])
        aggregates = {key: [sum(row[key][k] for row in cycles) for k in range(4)]
                      for key in ("first_residual", "second_residual", "first_bound", "second_bound")}
        envelope1, envelope2 = [2*M*h for M in caps()], [4*M*h*h for M in caps()]
        if any(aggregates["first_bound"][k] > envelope1[k]
               or aggregates["second_bound"][k] > envelope2[k] for k in range(4)):
            raise ValueError("uniform finite-horizon envelope failed")
        absorbed = sum(row["absorbed_tokens"] for row in cycles)
        if specification["background"] == "blank" and absorbed == 0:
            raise ValueError("registered later-cycle absorption witness absent")
        rows.append(dict(id=specification["id"], L=specification["L"], background=specification["background"],
                         cycles=cycles, absorbed_tokens=absorbed, totals=aggregates,
                         uniform_first_bound=envelope1, uniform_second_bound=envelope2,
                         first_initial_field_tokens=W.observe_checkpoint(source.read_bytes()).field_tokens,
                         final_field_tokens=W.observe_checkpoint(previous).field_tokens))
    if stage_count != 52 or wasm["backend"]["actual_advance_calls"] != 48 or wasm["backend"]["actual_microticks"] != 96:
        raise ValueError("execution coverage mismatch")
    verify_pins(lock)
    if tuple(sha(p) for p in paths) != hashes:
        raise ValueError("evidence changed during audit")
    result = dict(schema="ftd-hydro-weak-continuum-audit-v1", disposition="PASS_WEAK_CONSERVATION",
                  registration_sha256=hashes[0], input_sha256=hashes[1], wasm_sha256=hashes[2],
                  complete_stages_reproduced=stage_count, complete_arrays_reproduced=stage_count*8,
                  events_reproduced=event_count, cases=rows, all_balances_and_bounds_pass=True,
                  source_input_postchecks=True, elapsed_seconds=time.monotonic()-started,
                  full_fluid_recovery=False, scope=lock["claim"])
    write_new(output, result)
    return {key: value for key, value in result.items() if key != "cases"}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("prepare-input", "register", "audit"))
    parser.add_argument("paths", nargs="+")
    args = parser.parse_args()
    fn = {"prepare-input": prepare_inputs, "register": register, "audit": audit}[args.command]
    print(json.dumps(serial(fn(*args.paths)), indent=2))


if __name__ == "__main__":
    main()
