"""Independent V2 composite evidence codecs and bounded readiness controls.

The filename is the accepted ownership path; the container/map version is V2.
Importing this module performs no preparations, transitions, or catalogue I/O.
Registered production execution is unavailable until its separate readiness gate.
"""
from __future__ import annotations

import argparse
import ast
from array import array
from dataclasses import dataclass, replace
from datetime import datetime, timezone
import hashlib
import heapq
import importlib
import importlib.util
import itertools
import json
import mmap
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys
import threading
import time
import uuid
from functools import lru_cache

ROOT = Path(__file__).resolve().parents[3]
NATIVE = ROOT / "scripts/phi_v2_lattice/native"
STATE_MAGIC = b"FTD-Q4-NATIVE-1\n"
EXPORT_MAGIC = b"FTD-Q2-COMPOSITE-EXPORT-1\n"
MAP_MAGIC = b"FTD-Q4-COMPOSITE-MAP-2\n"
RANGE_MAGIC = b"FTD-Q4-COMPOSITE-RANGE-2\n"
FRAME_MAGIC = b"Q4F2"
PROBE_MAGIC = b"FTD-Q4-ADAPTER-PROBE-1\n"
ACTION_MAGIC = b"FTD-Q4-COMPOSITE-ACTIONS-1\n"
TEMPLATE_MAGIC = b"FTD-Q2-COMPOSITE-TEMPLATES-1\n"
CODEC = "q4-tick-payload-u24-1"
GROUP_KEY = "complete-checkpoint-and-ordered-complete-controls-1"
WIRE_ID = "credit-exchange-q4-native-wire-1"
TIER_COUNTS = {"head_on": 7296, "head_on_interventions": 80256, "full_impact": 1094400}
H = 304
FRAME_CAP = 131072
OUTER_MONITOR_CAP = 128 << 20
SUPERVISOR_PARENT_CAP = 1 << 30
DESCENDANT_ALLOWANCE = (8 << 30) - OUTER_MONITOR_CAP - SUPERVISOR_PARENT_CAP
OUTER_METADATA_ALLOWANCE_SECONDS = 300
PROBE_CAP = 16 << 20
PROBE_RESPONSE_CAP = 64 << 20
EVENT_NAMES = ("moves", "redirects", "capacity_holds", "attempt_marks", "attempt_expiries", "onsite_alignments", "credit_exchanges")
EVENT_WIDTHS = (9, 5, 3, 3, 2, 5, 13)
EVENT_CAPS = (2, 2, 1, 2, 4, 2, 2)
REASONS = ("accepted", "capacity", "flux_capacity", "credit_deficit", "credit_capacity")
KEY_NAMES = ("attempted", "charge_frame", "credit", "direction", "flux", "matching_frame")
FROZEN = {
    "scripts/phi_v2_lattice/recovery_process_limits.py": "cef521b1fd1aa3d5d4711b283dbbf5ecbaa45cb4299f60c442f9746c8827e0a1",
    "scripts/phi_v2_lattice/sparse_credit_exchange_q4.py": "417272af0ff258b92f31da26ef51a920a75de5dc3bdb3adf00335354b8d30b42",
    "scripts/phi_v2_lattice/recovery_composite_interaction.py": "3aaccee38442ba456a086e8d4a483fdec9e0511953d75d80f1ddfcce650b3aab",
    "scripts/phi_v2_lattice/credit_exchange_binding.py": "9a317076455949c15b743e7a2576e2dcc9a0311c47825160bee188cc39f12489",
    "scripts/phi_v2_lattice/experiments/certify_full_memory.py": "4d8ab048c892638e84ae09db8fcc09890f2ba6be62ac87eee467b7221db2d9e8",
    "scripts/phi_v2_lattice/recovery_credit_exchange_q2.py": "0330aa114bbb3d883642f104db71ffc8a08546e102b93c922bccdfee8589d40e",
    "scripts/phi_v2_lattice/balanced_matching.py": "5bd68b23b7dfa089103e4c593cbb4a739d0ed395e828112ab84d2dc5bc7e2c62",
    "scripts/phi_v2_lattice/flux_binding.py": "33203442187324ebf8d0f4c47668ffe0f3c5aa2d918284dab46fdb4aa6991740",
    "scripts/phi_v2_lattice/recorded_matching.py": "f81c72673159142158f863d06e8d5b3847d4e239f8df8752c2efbbf0e6a97172",
}
PRIOR_MANIFEST = ROOT / "engine/docs/evidence/strict-recovery-wave7-2026-09-08/manifest.json"
PRIOR_MANIFEST_SHA256 = "ea20fdd81bbc4b87b5afbb6fba33cc240f416df7391eab30a316b1f3df793047"
_ORACLE = None
DESCRIPTOR_KEYS = {"schema", "tier", "N", "H", "L", "range_count", "workers", "deadline_seconds", "memory_bytes", "source_sha256", "catalogue", "actions", "templates", "build", "readiness", "preservation"}
RANGE_KEYS = {"schema", "tier", "range", "start", "stop", "lock_sha256", "input_descriptor_sha256", "executable_sha256", "group_map_sha256", "H", "L", "kernel_wire", "observation_encoding", "compression"}
MAP_KEYS = {"schema", "tier", "N", "G_state", "G_execution", "O", "input_descriptor_sha256", "action_table_sha256", "group_key"}
LOCK_KEYS = {"schema", "tier", "N", "H", "L", "range_count", "workers", "deadline_seconds", "memory_bytes", "source_sha256", "catalogue_npz", "catalogue_pins", "build", "readiness", "preservation", "specifications", "output", "storage", "sorting"}
PRODUCTION_GATES = ("native_kernel", "native_observer", "catalogue_export", "support_templates", "ordered_execution_equivalence",
                    "state_equivalence", "action_maps", "streamed_preparations", "range_protocol", "complete_case_reconstruction",
                    "capture_escape", "source_cache_guard", "job_assignment_memory_descendant_kill", "deadline_finalization",
                    "storage_generations_spare", "all_alias_reduction", "failure_prefix_preservation", "complete_integrated_suite")


def sha(path):
    with Path(path).open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def canonical(value):
    return json.dumps(value, sort_keys=True, ensure_ascii=True, separators=(",", ":"), allow_nan=False).encode("ascii")


def parse_json(raw, keys=None, require_canonical=True):
    def pairs(rows):
        out = {}
        for key, value in rows:
            if key in out:
                raise ValueError("duplicate JSON key")
            out[key] = value
        return out
    value = json.loads(raw.decode("ascii"), object_pairs_hook=pairs,
                       parse_constant=lambda value: (_ for _ in ()).throw(ValueError("nonfinite JSON")))
    if type(value) is not dict or (require_canonical and canonical(value) != raw) or (keys is not None and set(value) != set(keys)):
        raise ValueError("noncanonical JSON or wrong key set")
    return value


def metadata_json(raw, keys):
    if len(raw) > 65536:
        raise ValueError("metadata64KiB limit")
    try:
        value = parse_json(raw, keys)
    except RecursionError as error:
        raise ValueError("metadata depth limit") from error
    def visit(item, depth=0):
        if depth > 12:
            raise ValueError("metadata depth limit")
        if isinstance(item, dict):
            for nested in item.values():
                visit(nested, depth + 1)
        elif isinstance(item, list):
            for nested in item:
                visit(nested, depth + 1)
    visit(value)
    return value


def absolute_path(value):
    if type(value) is not str or not value or len(value) > 32767 or any(not 32 <= ord(char) <= 126 for char in value) or not Path(value).is_absolute():
        raise ValueError("absolute printable-ASCII backend path required")
    return value


def artifact(value, verify=True):
    if type(value) is not dict or set(value) != {"path", "sha256"}:
        raise ValueError("exact artifact path/hash pair required")
    absolute_path(value["path"])
    hash_text(value["sha256"])
    if verify and sha(value["path"]) != value["sha256"]:
        raise ValueError("artifact identity changed")
    return value


def read_artifact(value, cap):
    """Hash the exact owned buffer parsed by admission, never hash then reopen."""
    artifact(value, False)
    with Path(value["path"]).open("rb") as stream:
        raw = stream.read(cap + 1)
    if len(raw) > cap or hashlib.sha256(raw).hexdigest() != value["sha256"]:
        raise ValueError("owned artifact bytes exceed cap or changed identity")
    return raw


def parse_identity(raw):
    if not raw.endswith(b"\n"):
        raise ValueError("identity requires final newline")
    row = parse_json(raw, {"schema", "kernel_wire", "observation_encoding", "compression", "supported_probe_opcodes", "campaign_ready", "canonical_adoption"}, False)
    if (row["schema"] != "strict-composite-native-adapter-identity-1" or row["kernel_wire"] != WIRE_ID
            or row["observation_encoding"] != "frozen-python-case-result-1" or row["compression"] != CODEC
            or type(row["campaign_ready"]) is not bool or row["canonical_adoption"] is not False):
        raise ValueError("adapter identity fields")
    opcodes = row["supported_probe_opcodes"]
    if type(opcodes) is not list or any(type(op) is not int or not 1 <= op <= 6 for op in opcodes) or opcodes != sorted(set(opcodes)):
        raise ValueError("implemented opcode inventory")
    return row


def validate_native_identity(returncode, raw):
    """Validate capabilities; the separately verified receipt admits production."""
    if type(returncode) is not int or returncode != 0:
        raise ValueError("native identity command failed")
    row = parse_identity(raw)
    if row["supported_probe_opcodes"] != [1, 2, 3, 4, 5, 6]:
        raise ValueError("native adapter capabilities are incomplete")
    return row


def integer(value, name, lo=0, hi=None):
    if type(value) is not int or value < lo or (hi is not None and value > hi):
        raise ValueError("integer outside domain: " + name)
    return value


def hash_text(value):
    if type(value) is not str or re.fullmatch("[0-9a-f]{64}", value) is None:
        raise ValueError("canonical SHA256 required")
    return value


def write_new(path, data):
    """Exclusive writes retain interrupted bytes; never replace an earlier file."""
    with Path(path).open("xb") as stream:
        stream.write(data)
        stream.flush()
        os.fsync(stream.fileno())


def check_pins(pins):
    for name, digest in pins.items():
        path = Path(name)
        path = path if path.is_absolute() else ROOT / path
        if sha(path) != hash_text(digest):
            raise ValueError("source/input identity changed: " + str(path))


def _oracle():
    global _ORACLE
    if _ORACLE is not None:
        return _ORACLE
    # Per-record codec work uses these owned loaded modules. Entry and final
    # scientific admission separately recheck the complete locked source map.
    verify_import_closure()
    if str(ROOT / "scripts") not in sys.path:
        sys.path.insert(0, str(ROOT / "scripts"))
    S = importlib.import_module("phi_v2_lattice.sparse_credit_exchange_q4")
    I = importlib.import_module("phi_v2_lattice.recovery_composite_interaction")
    _ORACLE = S, I
    return _ORACLE


def verify_import_closure():
    raw = PRIOR_MANIFEST.read_bytes()
    if hashlib.sha256(raw).hexdigest() != PRIOR_MANIFEST_SHA256:
        raise ValueError("accepted ancestry manifest changed")
    manifest = json.loads(raw)
    accepted = dict(manifest["source_sha256"])
    accepted.update(FROZEN)
    package = ROOT / "scripts/phi_v2_lattice"
    pending = [package / "sparse_credit_exchange_q4.py", package / "recovery_composite_interaction.py",
               package / "experiments/certify_full_memory.py", package / "recovery_process_limits.py", package / "__init__.py"]
    closure = {}
    while pending:
        path = pending.pop().resolve()
        name = path.relative_to(ROOT).as_posix()
        if name in closure:
            continue
        expected = accepted.get(name, accepted.get(str(path)))
        if expected is None or sha(path) != expected:
            raise ValueError("unaccepted transitive source before import: " + name)
        closure[name] = expected
        directory = path.parent
        while directory.is_relative_to(package):
            initializer = directory / "__init__.py"
            if initializer.is_file() and initializer.resolve() != path:
                pending.append(initializer)
            if directory == package:
                break
            directory = directory.parent
        tree = ast.parse(path.read_bytes(), filename=str(path))
        for node in ast.walk(tree):
            candidates = []
            if isinstance(node, ast.ImportFrom) and node.level:
                base = path.parent
                for _ in range(node.level - 1):
                    base = base.parent
                if node.module:
                    candidates.append(base.joinpath(*node.module.split(".")).with_suffix(".py"))
                else:
                    candidates.extend(base / (alias.name + ".py") for alias in node.names)
            elif isinstance(node, (ast.Import, ast.ImportFrom)):
                names = [alias.name for alias in node.names] if isinstance(node, ast.Import) else [node.module or ""]
                candidates.extend(ROOT.joinpath("scripts", *name.split(".")).with_suffix(".py") for name in names if name.startswith("phi_v2_lattice."))
                if isinstance(node, ast.ImportFrom) and node.module and node.module.startswith("phi_v2_lattice"):
                    base = ROOT.joinpath("scripts", *node.module.split("."))
                    candidates.extend(base / (alias.name + ".py") for alias in node.names)
            for candidate in candidates:
                if candidate.is_file() and candidate.resolve().is_relative_to(package):
                    pending.append(candidate)
    return closure


def verify_loaded_imports(closure=None):
    """Check dynamic/import-cache completeness against the accepted closure."""
    closure = verify_import_closure() if closure is None else closure
    loaded = {}
    for name, module in tuple(sys.modules.items()):
        source = getattr(module, "__file__", None)
        if (not name.startswith("phi_v2_lattice") and name != "_ftd_verified_process_limits") or not source:
            continue
        path = Path(source).resolve()
        if path == Path(__file__).resolve():
            continue
        relative = path.relative_to(ROOT).as_posix()
        if not path.suffix == ".py" or relative not in closure or sha(path) != closure[relative]:
            raise ValueError("loaded module outside accepted source closure: " + name)
        loaded[name] = {"path": str(path), "sha256": closure[relative]}
    return loaded


class Reader:
    def __init__(self, data):
        self.data = memoryview(data)
        self.pos = 0

    @property
    def remaining(self):
        return len(self.data) - self.pos

    def take(self, count):
        integer(count, "read length", 0, self.remaining)
        result = self.data[self.pos:self.pos + count].tobytes()
        self.pos += count
        return result

    def unpack(self, fmt):
        return struct.unpack("<" + fmt, self.take(struct.calcsize("<" + fmt)))

    def one(self, fmt):
        return self.unpack(fmt)[0]

    def blob(self, cap=PROBE_CAP, fmt="Q"):
        size = self.one(fmt)
        integer(size, "blob length", 0, cap)
        return self.take(size)

    def done(self):
        if self.remaining:
            raise ValueError("trailing bytes")


def blob(data):
    return struct.pack("<Q", len(data)) + data


def _state(state):
    S, _ = _oracle()
    state = S._normalized(state)
    if state.L > 64:
        raise ValueError("unsupported native L")
    return state


def state_wire(state):
    state = _state(state)
    out = bytearray(STATE_MAGIC + b"".join(bytes.fromhex(v) for v in (state.rule_hash, state.frame_hash, state.encoding_hash)))
    out += struct.pack("<HBBB3x", state.L, state.origin_code, state.charge_frame, len(state.edges))
    for c in state.carriers:
        out += struct.pack("<IBBBB", c.site, c.slot, c.direction, c.credit, c.attempted)
    for e in state.edges:
        out += struct.pack("<IBb2x", e.owner, e.axis, e.q)
    out += bytes(8 * (4 - len(state.edges)))
    ordinal = format(state.microtick, "x").encode("ascii")
    return bytes(out) + blob(ordinal)


def decode_state(data):
    S, _ = _oracle()
    r = Reader(data)
    identities = b"".join(bytes.fromhex(v) for v in (S.RULE_HASH, S.FRAME_HASH, S.DENSE_ENCODING))
    if r.take(112) != STATE_MAGIC + identities:
        raise ValueError("kernel state identity mismatch")
    L, origin, eta, count = r.unpack("HBBB")
    integer(count, "edge_count", 0, 4)
    if r.take(3) != bytes(3):
        raise ValueError("reserved header bits")
    carriers = tuple(S.Carrier(*r.unpack("IBBBB")) for _ in range(4))
    edges = []
    for i in range(4):
        row = r.take(8)
        if row[6:] != bytes(2) or (i >= count and row != bytes(8)):
            raise ValueError("noncanonical edge padding")
        if i < count:
            edges.append(S.Edge(*struct.unpack("<IBb2x", row)))
    ordinal = r.blob(cap=r.remaining).decode("ascii")
    r.done()
    if re.fullmatch("0|[1-9a-f][0-9a-f]*", ordinal) is None:
        raise ValueError("noncanonical full ordinal")
    state = _state(S.SparseQ4State(L, int(ordinal, 16), carriers, tuple(edges), origin, eta))
    if state_wire(state) != data:
        raise ValueError("noncanonical kernel state wire")
    return state


def pack_payload(state):
    state = _state(state)
    out = bytearray()
    for c in state.carriers:
        word = c.site | (c.slot << 18) | (c.direction << 19) | (c.credit << 22) | (c.attempted << 23)
        out += word.to_bytes(3, "little")
    out.append(len(state.edges))
    for e in state.edges:
        out += (e.owner | (e.axis << 18) | ((e.q == 1) << 20)).to_bytes(3, "little")
    return bytes(out)


def _unpack_payload(r, begin, offset):
    S, _ = _oracle()
    begin = _state(begin)
    integer(offset, "Tick offset", 0, H)
    start = r.pos
    carriers = []
    for _ in range(4):
        word = int.from_bytes(r.take(3), "little")
        direction = (word >> 19) & 7
        integer(direction, "direction", 1, 6)
        carriers.append(S.Carrier(word & 0x3ffff, (word >> 18) & 1, direction, (word >> 22) & 1, (word >> 23) & 1))
    count = integer(r.one("B"), "edge_count", 0, 4)
    edges = []
    for _ in range(count):
        word = int.from_bytes(r.take(3), "little")
        if word >> 21 or ((word >> 18) & 3) == 3:
            raise ValueError("edge reserved bits or axis")
        edges.append(S.Edge(word & 0x3ffff, (word >> 18) & 3, 2 * ((word >> 20) & 1) - 1))
    state = _state(S.SparseQ4State(begin.L, begin.microtick + offset, tuple(carriers), tuple(edges), begin.origin_code, begin.charge_frame))
    if pack_payload(state) != r.data[start:r.pos].tobytes():
        raise ValueError("noncanonical compact payload")
    return state


def unpack_payload(data, begin, offset):
    r = Reader(data)
    state = _unpack_payload(r, begin, offset)
    r.done()
    return state


def _event_numbers(events):
    S, I = _oracle()
    I.event_record(events)
    lists = []
    for name, width, cap in zip(EVENT_NAMES, EVENT_WIDTHS, EVENT_CAPS):
        rows = []
        for original in getattr(events, name):
            row = list(original)
            if len(row) != width:
                raise ValueError("event row width")
            if name in ("redirects", "attempt_marks"):
                if row[-1] not in REASONS:
                    raise ValueError("unknown event reason")
                row[-1] = REASONS.index(row[-1])
            rows.append(tuple(integer(v, "event word", -(1 << 31), (1 << 31) - 1) for v in row))
        integer(len(rows), "event count", 0, cap)
        lists.append(tuple(rows))
    reset = bool(lists[4] or lists[5])
    exchange = bool(lists[6])
    hopping = any(lists[:4])
    if sum((reset, exchange, hopping)) > 1:
        raise ValueError("mixed-stage event bundle")
    # Per-field domains, separate from state-bound transition equivalence.
    domains = {
        "moves": ((0, (-1, 1)), (4, range(3)), (5, range(-1, 2)), (6, range(-1, 2)), (7, range(2)), (8, range(2))),
        "redirects": ((0, (-1, 1)), (2, range(1, 7)), (3, range(1, 7)), (4, range(2, 5))),
        "capacity_holds": ((0, (-1, 1)), (2, range(3))),
        "attempt_marks": ((0, (-1, 1)), (2, range(5))),
        "attempt_expiries": ((0, (-1, 1)),),
        "onsite_alignments": ((1, (-1, 1)), (2, range(1, 7)), (3, range(1, 7)), (4, range(1, 7))),
        "credit_exchanges": ((1, range(3)), (2, (-1, 1)), (4, (-1, 1)), (6, range(2)), (7, range(2)), (8, range(2)), (9, range(2)), (10, range(1, 7)), (11, range(1, 7)), (12, range(1, 7))),
    }
    sites = ((1, 2, 3), (1,), (1,), (1,), (1,), (0,), (0, 3, 5))
    for name, rows, site_columns in zip(EVENT_NAMES, lists, sites):
        for row in rows:
            if any(row[column] not in allowed for column, allowed in domains[name]):
                raise ValueError("invalid event field")
            for column in site_columns:
                integer(row[column], "event site", 0, 64 ** 3 - 1)
    return lists


def event_wire(events):
    rows = _event_numbers(events)
    out = bytes([*(len(v) for v in rows), 0])
    out += b"".join(struct.pack("<" + "i" * len(row), *row) for group in rows for row in group)
    if len(out) > 112:
        raise ValueError("event wire bound")
    return out


def decode_events(data):
    S, _ = _oracle()
    r = Reader(data)
    counts = r.take(8)
    if counts[7] or len(data) > 112:
        raise ValueError("event reserved byte or length")
    events = S.dense.ExchangeEvents()
    for name, width, cap, count in zip(EVENT_NAMES, EVENT_WIDTHS, EVENT_CAPS, counts):
        integer(count, "event count", 0, cap)
        rows = []
        for _ in range(count):
            row = list(r.unpack("i" * width))
            if name in ("redirects", "attempt_marks"):
                integer(row[-1], "reason", 0, 4)
                row[-1] = REASONS[row[-1]]
            rows.append(tuple(row))
        setattr(events, name, rows)
    r.done()
    if event_wire(events) != data:
        raise ValueError("event re-encoding mismatch")
    return events


def lift_wire(state, lift):
    _, I = _oracle()
    I.validate_lift(state, lift)
    return b"".join(struct.pack("<hhh", *(integer(v, "lift coordinate", -512, 512) for v in point)) for point in lift.carriers + lift.edges)


def _read_lift(r, state):
    _, I = _oracle()
    lift = I.IntegerLift(tuple(r.unpack("hhh") for _ in range(4)), tuple(r.unpack("hhh") for _ in state.edges))
    lift_wire(state, lift)
    return lift


def controls_wire(controls, state):
    _, I = _oracle()
    if len(controls) != 2:
        raise ValueError("ordered pair of controls required")
    out = bytearray()
    for c in controls:
        c = I._control(c)
        if (c.L, c.microtick, c.origin_code, c.charge_frame) != (state.L, state.microtick, state.origin_code, state.charge_frame):
            raise ValueError("control context mismatch")
        out += struct.pack("<Ihhh", c.node, *(integer(v, "control coordinate", -512, 512) for v in c.chart_plus_position))
    return bytes(out)


def _read_controls(r, state, count=2):
    _, I = _oracle()
    out = []
    for _ in range(count):
        node, *point = r.unpack("Ihhh")
        out.append(I._control(I.Q2Control(state.L, state.microtick, state.origin_code, state.charge_frame, tuple(point), node)))
    if count == 2:
        controls_wire(out, state)
    return tuple(out)


def graph_wire(state):
    S, _ = _oracle()
    graph = S.observe(state)
    out = bytearray([len(graph.components)])
    for c in graph.components:
        cm = sum(1 << state.carriers.index(row) for row in c.carriers)
        em = sum(1 << state.edges.index(row) for row in c.edges)
        out += bytes((len(c.sites), cm, em, *c.populations, c.F, c.K, c.Q))
        out += b"".join(struct.pack("<I", site) for site in c.sites)
    out.append(len(graph.degrees))
    out += b"".join(struct.pack("<IBB", *row) for row in graph.degrees)
    out += bytes((*graph.populations, graph.F, graph.K, graph.Q, len(graph.gauss_residual)))
    out += b"".join(struct.pack("<Ib", *row) for row in graph.gauss_residual)
    return bytes(out)


def keys_wire(keys):
    if tuple(keys) != tuple(sorted(set(keys))) or len(keys) > 2376:
        raise ValueError("noncanonical contact keys")
    out = bytearray(struct.pack("<H", len(keys)))
    for name, site, column in keys:
        if name not in KEY_NAMES:
            raise ValueError("contact key name")
        integer(site, "key site", 0, 64 ** 3 - 1)
        integer(column, "key column", 0, 0 if name in ("charge_frame", "matching_frame") else 2 if name == "flux" else 1)
        out += struct.pack("<BIB", KEY_NAMES.index(name), site, column)
    return bytes(out)


def conflict_wire(conflict):
    _, I = _oracle()
    if type(conflict) is not I.SuperpositionConflict:
        raise ValueError("complete conflict required")
    if conflict.reason == "context_or_clock" and conflict.keys == ():
        return bytes(3)
    if conflict.reason != "same_slot_or_shared_edge" or len(conflict.keys) != 2:
        raise ValueError("conflict reason or keys")
    out = bytearray([1])
    for rows, high in zip(conflict.keys, (1, 2)):
        if len(rows) > 2 or tuple(rows) != tuple(sorted(set(rows))):
            raise ValueError("conflict key order/count")
        out.append(len(rows))
        for site, column in rows:
            out += struct.pack("<IB", integer(site, "conflict site", 0, 64 ** 3 - 1), integer(column, "conflict column", 0, high))
    return bytes(out)


@lru_cache(maxsize=1)
def capture_pairs():
    return tuple((start, end) for end in range(305) for start in range(end) if (end - start) % 19 == 0)


@lru_cache(maxsize=1)
def _capture_indices():
    return {(end, start): index for index, (start, end) in enumerate(capture_pairs())}


def capture_wire(capture):
    pairs = _capture_indices()
    out = bytearray(287)
    seen = set()
    for witness in capture.witnesses:
        key = (witness.end, witness.start)
        if key not in pairs or key in seen:
            raise ValueError("invalid or duplicate capture pair")
        seen.add(key)
        index = pairs[key]
        out[index // 8] |= 1 << (index % 8)
    return bytes(out)


def escape_wire(state, escape):
    reasons = ("not_two_components", "not_two_Q2_components", "nonrecurrent_component", "exact_sufficient_support_test")
    if escape.reason not in reasons:
        raise ValueError("escape reason")
    flags = int(escape.applicable) | (int(escape.infinite_lift) << 1) | (int(escape.periodic_torus) << 2)
    if len(escape.controls) not in (0, 2) or len(escape.drifts) not in (0, 2):
        raise ValueError("escape cardinality")
    out = bytearray([flags, reasons.index(escape.reason), len(escape.controls)])
    if escape.controls:
        out += controls_wire(escape.controls, state)
    out.append(len(escape.drifts))
    for drift in escape.drifts:
        out += struct.pack("<bbb", *drift)
    out.append(integer(escape.modular_period, "modular period", 0, 32))
    for kind, witness in enumerate((escape.interval_witness, escape.residue_witness)):
        out.append(int(witness is not None))
        if witness is None:
            continue
        phase, point, slope, *tail = witness
        out.append(integer(phase, "support phase", 0, 37))
        out += struct.pack("<hhhbbb", *point, *slope)
        if kind == 0:
            lo, hi = tail[0]
            out += struct.pack("<IB", integer(lo, "interval lower", 0, 1023), int(hi is not None))
            if hi is not None:
                out += struct.pack("<I", integer(hi, "interval upper", lo, 1023))
        else:
            n, residue = tail
            out += bytes([integer(n, "residue n", 0, 31), *(integer(v, "residue", 0, 63) for v in residue)])
    if len(out) > 66:
        raise ValueError("escape byte bound")
    return bytes(out)


def observation_chain(previous, state, events, observation, lift_observation):
    S, I = _oracle()
    if len(previous) != 32:
        raise ValueError("complete previous chain digest required")
    checkpoint = S.checkpoint(state)
    representation = repr((I.event_record(events), observation, lift_observation)).encode("ascii")
    return hashlib.sha256(previous + len(checkpoint).to_bytes(8, "big") + checkpoint + representation).digest(), representation


def flags_byte(observation):
    return sum(int(value) << bit for bit, value in enumerate((observation.structural_contact, observation.state_effect, observation.event_difference, observation.counterfactual_invalid)))


def tick_wire(offset, state, lift, events, chain, flags):
    integer(offset, "Tick offset", 1, H)
    integer(flags, "Tick flags", 0, 15)
    if len(chain) != 32:
        raise ValueError("chain digest length")
    wire = event_wire(events)
    return struct.pack("<H", offset) + pack_payload(state) + lift_wire(state, lift) + struct.pack("<H", len(wire)) + wire + chain + bytes([flags])


def read_tick(r, begin):
    offset = integer(r.one("H"), "Tick offset", 1, H)
    state = _unpack_payload(r, begin, offset)
    lift = _read_lift(r, state)
    events = decode_events(r.blob(112, "H"))
    chain = r.take(32)
    flags = integer(r.one("B"), "Tick flags", 0, 15)
    return offset, state, lift, events, chain, flags


@dataclass(frozen=True)
class Begin:
    group: int
    representative: int
    multiplicity: int
    orbit: int
    preparation: object
    state: object
    lift: object
    controls: tuple
    seed_nodes: tuple
    lift_steps: int


def begin_wire(begin):
    S, _ = _oracle()
    n = TIER_COUNTS[begin.preparation.tier]
    fields = (integer(begin.group, "group", 0, n - 1), integer(begin.representative, "representative", 0, n - 1),
              integer(begin.multiplicity, "multiplicity", 1, n), integer(begin.orbit, "orbit", 0, n - 1))
    if begin.state.L != 64 or begin.state.microtick != 1 + begin.preparation.stage_advance or begin.lift_steps != begin.state.microtick:
        raise ValueError("BEGIN campaign context/ordinal")
    if len(begin.seed_nodes) != 2:
        raise ValueError("ordered seed-node pair required")
    wire = state_wire(begin.state)
    out = struct.pack("<IIII", *fields) + preparation_wire(begin.preparation) + struct.pack("<I", len(wire)) + wire
    out += lift_wire(begin.state, begin.lift) + controls_wire(begin.controls, begin.state)
    out += struct.pack("<IIB", *(integer(node, "seed node", 0, 941183) for node in begin.seed_nodes), begin.lift_steps)
    out += hashlib.sha256(S.checkpoint(begin.state)).digest()
    if len(out) > 333:
        raise ValueError("BEGIN body bound")
    return out


def decode_begin(raw, tier):
    S, I = _oracle()
    r = Reader(raw)
    group, representative, multiplicity, orbit = r.unpack("IIII")
    preparation = _read_preparation(r, tier)
    state = decode_state(r.blob(194, "I"))
    lift = _read_lift(r, state)
    controls = _read_controls(r, state)
    seeds = r.unpack("II")
    steps = r.one("B")
    if r.take(32) != hashlib.sha256(S.checkpoint(state)).digest():
        raise ValueError("initial checkpoint hash mismatch")
    r.done()
    begin = Begin(group, representative, multiplicity, orbit, preparation, state, lift, controls, seeds, steps)
    if begin_wire(begin) != raw or lift != I.initial_lift(state):
        raise ValueError("noncanonical BEGIN")
    I.check_no_wrap(state, lift)
    return begin


def original_row_wire(row):
    """Exact 76-byte V2 original row, retaining both equivalence relations."""
    n = TIER_COUNTS[row["preparation"].tier]
    integer(row["index"], "index", 0, n - 1)
    for key in ("group", "representative", "orbit", "state_group", "state_representative"):
        integer(row[key], key, 0, n - 1)
    for key in ("multiplicity", "state_multiplicity"):
        integer(row[key], key, 1, n)
    integer(row["background"], "background", 0, 95)
    integer(row["owner_range"], "owner range", 0, 63)
    if row["owner_range"] != row["representative"] // (n // 64):
        raise ValueError("execution representative owner mismatch")
    return (struct.pack("<I", row["index"]) + preparation_wire(row["preparation"]) + bytes.fromhex(hash_text(row["checkpoint_sha256"]))
            + struct.pack("<IIIBBIIII", row["group"], row["representative"], row["orbit"], row["background"], row["owner_range"],
                          row["multiplicity"], row["state_group"], row["state_representative"], row["state_multiplicity"]))


def decode_original_row(raw, tier):
    r = Reader(raw)
    row = {"index": r.one("I"), "preparation": _read_preparation(r, tier), "checkpoint_sha256": r.take(32).hex()}
    row.update(zip(("group", "representative", "orbit", "background", "owner_range", "multiplicity", "state_group", "state_representative", "state_multiplicity"), r.unpack("IIIBBIIII")))
    r.done()
    if original_row_wire(row) != raw:
        raise ValueError("noncanonical original row")
    return row


def orbit_row_wire(orbit, state):
    S, _ = _oracle()
    integer(orbit, "orbit", 0, 1094399)
    if state.origin_code or state.charge_frame or state.L != 64 or not 1 <= state.microtick <= 19:
        raise ValueError("normalized campaign orbit state required")
    wire = state_wire(state)
    body = struct.pack("<IH", orbit, len(wire)) + wire + hashlib.sha256(S.checkpoint(state)).digest()
    if len(body) + 2 > 256:
        raise ValueError("orbit row bound")
    return struct.pack("<H", len(body)) + body


def decode_orbit_row(raw):
    S, _ = _oracle()
    outer = Reader(raw)
    r = Reader(outer.blob(254, "H"))
    outer.done()
    orbit = r.one("I")
    state = decode_state(r.blob(194, "H"))
    if r.take(32) != hashlib.sha256(S.checkpoint(state)).digest():
        raise ValueError("orbit checkpoint hash mismatch")
    r.done()
    if orbit_row_wire(orbit, state) != raw:
        raise ValueError("noncanonical orbit row")
    return orbit, state


def validate_range_header(raw, expected=None):
    header = parse_json(raw, RANGE_KEYS)
    tier = header["tier"]
    if tier not in TIER_COUNTS:
        raise ValueError("unknown range tier")
    index = integer(header["range"], "range", 0, 63)
    width = TIER_COUNTS[tier] // 64
    for name, value in {"H": 304, "L": 64, "start": index * width, "stop": (index + 1) * width}.items():
        if integer(header[name], name) != value:
            raise ValueError("range interval or registered parameter mismatch")
    for name, value in {"schema": "q4-composite-range-2", "kernel_wire": WIRE_ID,
                        "observation_encoding": "frozen-python-case-result-1", "compression": CODEC}.items():
        if header[name] != value:
            raise ValueError("range version/codec mismatch")
    for name in ("lock_sha256", "input_descriptor_sha256", "executable_sha256", "group_map_sha256"):
        hash_text(header[name])
    if expected is not None and canonical(header) != canonical(expected):
        raise ValueError("range differs from admitted descriptor/lock/map")
    return header


def frame_wire(sequence, kind, body, previous):
    integer(sequence, "sequence", 0, (1 << 64) - 1)
    integer(kind, "frame kind", 1, 4)
    integer(len(body), "frame body length", 0, FRAME_CAP)
    if len(previous) != 32:
        raise ValueError("previous frame digest length")
    raw = FRAME_MAGIC + struct.pack("<QBQ", sequence, kind, len(body)) + previous + body
    return raw + hashlib.sha256(raw).digest()


def decode_frames(raw, expected_header=None):
    """Recover only a contiguous authenticated prefix; never repair raw bytes."""
    r, frames, previous, error = Reader(raw), [], bytes(32), None
    header, accepted = None, 0
    try:
        if r.take(len(RANGE_MAGIC)) != RANGE_MAGIC:
            raise ValueError("V2 range magic required")
        header = validate_range_header(r.blob(2048), expected_header)
        accepted = r.pos
        terminal = False
        while r.remaining:
            start = r.pos
            if terminal:
                raise ValueError("suffix after RANGE_FINAL")
            if r.take(4) != FRAME_MAGIC:
                raise ValueError("V2 frame magic required")
            sequence, kind, count = r.unpack("QBQ")
            integer(kind, "frame kind", 1, 4)
            integer(count, "frame body length", 0, FRAME_CAP)
            if sequence != len(frames) or r.take(32) != previous:
                raise ValueError("frame sequence/digest chain mismatch")
            body = r.take(count)
            digest = r.take(32)
            if hashlib.sha256(raw[start:r.pos - 32]).digest() != digest:
                raise ValueError("frame digest mismatch")
            frames.append({"sequence": sequence, "kind": kind, "body": body, "digest": digest, "offset": start, "end": r.pos})
            previous, accepted, terminal = digest, r.pos, kind == 4
    except (ValueError, UnicodeError, struct.error) as failure:
        error = type(failure).__name__ + ": " + str(failure)
    return {"header": header, "frames": tuple(frames), "accepted_bytes": accepted,
            "tail_error": error, "tail_offset": accepted, "tail_sha256": hashlib.sha256(raw[accepted:]).hexdigest(),
            "raw_sha256": hashlib.sha256(raw).hexdigest()}


class RangeStream:
    """One owned handle, one bounded frame, semantic commit before advancement.

    A rejected frame and every following byte remain the unaccepted suffix.
    Finishing this reader only hashes existing bytes; it invents no terminal.
    """
    def __init__(self, path, expected_header=None):
        self.path = Path(path).resolve()
        self.stream = self.path.open("rb")
        self.raw_hash, self.pending = hashlib.sha256(), bytearray()
        self.offset = self.accepted = self.sequence = 0
        self.previous, self.terminal = bytes(32), False
        self.header, self.error, self.finished = None, None, None
        self.initial_stat = self.path.stat()
        try:
            if self._take(len(RANGE_MAGIC)) != RANGE_MAGIC:
                raise ValueError("V2 range magic required")
            count = integer(struct.unpack("<Q", self._take(8))[0], "range header length", 1, 2048)
            self.header = validate_range_header(self._take(count), expected_header)
            self.accepted = self.offset
            self.pending.clear()
        except (ValueError, UnicodeError, struct.error) as error:
            self.error = repr(error)

    def _take(self, count):
        raw = self.stream.read(count)
        self.raw_hash.update(raw)
        self.pending.extend(raw)
        self.offset += len(raw)
        if len(raw) != count:
            raise ValueError("truncated range bytes")
        return raw

    def next_frame(self):
        if self.error or self.finished is not None:
            return None
        if self.pending:
            raise ValueError("previous frame lacks semantic admission")
        first = self.stream.read(1)
        if not first:
            return None
        self.raw_hash.update(first)
        self.pending.extend(first)
        start = self.offset
        self.offset += 1
        try:
            if self.terminal:
                raise ValueError("suffix after RANGE_FINAL")
            if first + self._take(3) != FRAME_MAGIC:
                raise ValueError("V2 frame magic required")
            sequence, kind, count = struct.unpack("<QBQ", self._take(17))
            integer(kind, "frame kind", 1, 4)
            integer(count, "frame length", 0, FRAME_CAP)
            if sequence != self.sequence or self._take(32) != self.previous:
                raise ValueError("frame sequence/digest chain mismatch")
            body = self._take(count)
            expected = hashlib.sha256(self.pending).digest()
            digest = self._take(32)
            if digest != expected:
                raise ValueError("frame digest mismatch")
            return {"sequence": sequence, "kind": kind, "body": body, "digest": digest, "offset": start, "end": self.offset}
        except (ValueError, UnicodeError, struct.error) as error:
            self.error = repr(error)
            return None

    def commit(self, frame):
        if (not self.pending or frame["sequence"] != self.sequence or frame["end"] != self.offset
                or bytes(self.pending[-32:]) != frame["digest"]):
            raise ValueError("frame admission differs from owned bytes")
        self.previous, self.terminal = frame["digest"], frame["kind"] == 4
        self.sequence += 1
        self.accepted = self.offset
        self.pending.clear()

    def reject(self, error):
        self.error = repr(error)

    def finish(self, check=lambda: None):
        if self.finished is not None:
            return self.finished
        tail = hashlib.sha256(self.pending)
        while True:
            check()
            raw = self.stream.read(1 << 20)
            if not raw:
                break
            tail.update(raw)
            self.raw_hash.update(raw)
            self.offset += len(raw)
        final_stat = self.path.stat()
        if (final_stat.st_size, final_stat.st_mtime_ns, final_stat.st_ino) != (self.initial_stat.st_size, self.initial_stat.st_mtime_ns, self.initial_stat.st_ino):
            self.error = "range changed during owned-handle verification"
        self.finished = {"path": str(self.path), "header": self.header, "accepted_frames": self.sequence,
                         "accepted_bytes": self.accepted, "tail_offset": self.accepted, "tail_error": self.error,
                         "tail_bytes": self.offset - self.accepted, "tail_sha256": tail.hexdigest(),
                         "raw_bytes": self.offset, "raw_sha256": self.raw_hash.hexdigest(), "terminal_frame": self.terminal}
        return self.finished

    def close(self):
        self.stream.close()

    def __enter__(self):
        return self

    def __exit__(self, *_):
        self.close()


def decode_range_final(raw):
    if len(raw) > 2048:
        raise ValueError("RANGE_FINAL body bound")
    r = Reader(raw)
    names = ("range", "status", "start", "stop", "owned_groups", "completed_groups", "reported_executed_ticks",
             "verified_durable_ticks", "locally_covered_originals", "unresolved_aliases")
    row = dict(zip(names, r.unpack("BBIIIIQQII")))
    row.update(case_terminal_chain=r.take(32).hex(), group_map_sha256=r.take(32).hex(), error_code=r.one("H"))
    row["error"] = r.blob(1024, "H").decode("ascii")
    r.done()
    integer(row["range"], "range", 0, 63)
    integer(row["status"], "range status", 0, 1)
    integer(row["error_code"], "range error code", 0, 11)
    if "\0" in row["error"] or (row["status"] == 0 and (row["error_code"] or row["error"])) or (row["status"] and not row["error_code"]):
        raise ValueError("range terminal status/error mismatch")
    if not row["start"] < row["stop"] or row["completed_groups"] > row["owned_groups"] or row["verified_durable_ticks"] > row["reported_executed_ticks"]:
        raise ValueError("range terminal count domain")
    return row


def range_final_wire(row):
    names = ("range", "status", "start", "stop", "owned_groups", "completed_groups", "reported_executed_ticks",
             "verified_durable_ticks", "locally_covered_originals", "unresolved_aliases")
    error = row["error"].encode("ascii")
    raw = (struct.pack("<BBIIIIQQII", *(row[name] for name in names)) + bytes.fromhex(hash_text(row["case_terminal_chain"]))
           + bytes.fromhex(hash_text(row["group_map_sha256"])) + struct.pack("<HH", row["error_code"], len(error)) + error)
    if decode_range_final(raw) != row:
        raise ValueError("noncanonical RANGE_FINAL")
    return raw


def _new_case(begin):
    S, I = _oracle()
    return {"begin": begin, "history": [begin.state], "lifts": [begin.lift], "controls": begin.controls,
            "checkpoints": [S.checkpoint(begin.state)], "chain": [], "accounts": [],
            "lift_observations": [I.observe_lift(begin.state, begin.lift)], "first": [0] * 4, "counts": [0] * 4,
            "blocks": 0, "last_chain": bytes(32)}


def _progress_body(raw, case, catalogue):
    S, I = _oracle()
    # Mutate a detached working prefix only after the entire frame validates.
    case = {key: list(value) if isinstance(value, list) else value for key, value in case.items()}
    r = Reader(raw)
    representative, previous, completed = r.unpack("IHH")
    if representative != case["begin"].representative or previous != len(case["chain"]) or not 1 <= completed - previous <= 19 or completed > H:
        raise ValueError("PROGRESS range/representative mismatch")
    diagnostics = {}
    for expected_offset in range(previous + 1, completed + 1):
        offset, after, lift, events, chain, flags = read_tick(r, case["begin"].state)
        before, controls = case["history"][-1], case["controls"]
        expected_after, expected_events = S.step(before)
        if offset != expected_offset or after != expected_after or I.event_record(events) != I.event_record(expected_events):
            raise ValueError("stored Tick differs from complete local-law transition")
        following = tuple(I.advance_control(c, catalogue) for c in controls)
        observation = I.observe_step(before, after, events, controls, following, catalogue)
        expected_lift = I.advance_lift(before, after, events, case["lifts"][-1])
        if lift != expected_lift:
            raise ValueError("stored Tick lift differs from complete transition")
        I.check_no_wrap(after, lift)
        ledger = I.observe_lift(after, lift, case["history"][0], case["lifts"][0])
        digest, _ = observation_chain(case["last_chain"], after, events, observation, ledger)
        if chain != digest or flags != flags_byte(observation):
            raise ValueError("Tick observation/hash mismatch")
        for bit in range(4):
            if flags & (1 << bit):
                case["counts"][bit] += 1
                if not case["first"][bit]:
                    case["first"][bit] = offset
                    extra = b""
                    if bit == 0:
                        extra = keys_wire(observation.contact_outputs)
                    elif bit == 2:
                        baseline = I.superpose_controls(*controls, catalogue)
                        isolated = I._merge_events(*(I.advance_control(c, catalogue, True)[1] for c in controls), baseline)
                        wire = event_wire(isolated)
                        extra = struct.pack("<H", len(wire)) + wire
                    elif bit == 3:
                        extra = conflict_wire(observation.conflict)
                    diagnostics[bit] = struct.pack("<H", offset) + extra
        case["history"].append(after)
        case["lifts"].append(lift)
        case["controls"] = following
        case["chain"].append(chain.hex())
        case["checkpoints"].append(S.checkpoint(after))
        case["accounts"].append((observation.graph.F, observation.graph.K, observation.graph.Q))
        case["lift_observations"].append(ledger)
        case["last_chain"] = chain
    mask = sum(1 << bit for bit in diagnostics)
    exact = bytes([mask]) + b"".join(diagnostics[bit] for bit in sorted(diagnostics))
    if r.take(len(exact)) != exact or _read_controls(r, case["history"][-1]) != case["controls"]:
        raise ValueError("first-diagnostic or final ordered-control mismatch")
    r.done()
    case["blocks"] += 1
    if case["blocks"] > 16:
        raise ValueError("too many progress blocks")
    return case


def _final_body(raw, case, catalogue):
    S, I = _oracle()
    r = Reader(raw)
    representative, completed, status = r.unpack("IHB")
    if representative != case["begin"].representative or completed != len(case["chain"]):
        raise ValueError("CASE_FINAL representative/progress mismatch")
    integer(status, "case status", 0, 4)
    if r.take(32) != case["last_chain"] or r.unpack("HHHH") != tuple(case["first"]) or r.unpack("HHHH") != tuple(case["counts"]):
        raise ValueError("CASE_FINAL counters/chain mismatch")
    graph = r.blob(284, "H")
    present = integer(r.one("B"), "certificate present", 0, 1)
    escape_bytes = r.blob(66, "H")
    capture_bytes = r.take(287)
    classification, error_code = r.unpack("BH")
    error = r.blob(1024, "H").decode("ascii")
    integer(error_code, "error code", 0, 11)
    r.done()
    if len(raw) > 2048 or "\0" in error:
        raise ValueError("CASE_FINAL body/error bound")
    if graph and graph != graph_wire(case["history"][-1]):
        raise ValueError("final graph differs from complete state")
    if status:
        if present or escape_bytes or capture_bytes != bytes(287) or classification != 3 or not error_code:
            raise ValueError("incomplete case contains successful certificate")
        return {"status": status, "completed_ticks": completed, "error_code": error_code, "error": error, "case_result": None}
    if completed != H or not present or not graph or error_code or error:
        raise ValueError("incomplete execution cannot be COMPLETE_CASE")
    if case["blocks"] != 16:
        raise ValueError("complete case block inventory")
    escape = I.escape_certificate(case["history"][-1], catalogue)
    capture = I.capture_certificate(tuple(case["history"]), tuple(case["lifts"]))
    expected_class = 0 if capture.witnesses else 1 if escape.infinite_lift or escape.periodic_torus else 2
    if escape_bytes != escape_wire(case["history"][-1], escape) or capture_bytes != capture_wire(capture) or classification != expected_class:
        raise ValueError("terminal certificate differs from full frozen predicate")
    begin = case["begin"]
    provenance = (("catalogue_sha256", catalogue.sha256), ("seed_nodes", begin.seed_nodes), ("lift_steps", begin.lift_steps),
                  ("final_nodes", tuple(control.node for control in begin.controls)))
    result = I.CaseResult(begin.preparation, provenance, completed, tuple(case["checkpoints"]), tuple(case["chain"]),
                          *(value or None for value in case["first"]), escape, capture, S.observe(case["history"][-1]),
                          tuple(case["accounts"]), escape.drifts,
                          ("RELATIVE_PERIODIC_CAPTURE", "CERTIFIED_INDEPENDENT_ESCAPE", "UNRESOLVED_AT_304")[expected_class], tuple(case["lift_observations"]))
    return {"status": 0, "completed_ticks": completed, "error_code": 0, "error": "", "case_result": result}


def reconstruct_case_frames(frames, catalogue, admitted_begin):
    """Independent exact reconstruction; callers supply an already admitted map row.

    This is read-only verification of supplied evidence, never a preparation
    generator or a substitute for missing primary campaign work.
    """
    case = None
    for index, frame in enumerate(frames):
        kind, raw = frame["kind"], frame["body"]
        if index == 0:
            if kind != 1 or raw != begin_wire(admitted_begin):
                raise ValueError("case differs from immutable admitted BEGIN")
            case = _new_case(decode_begin(raw, admitted_begin.preparation.tier))
        elif kind == 2:
            case = _progress_body(raw, case, catalogue)
        elif kind == 3:
            if index != len(frames) - 1:
                raise ValueError("suffix after CASE_FINAL")
            return _final_body(raw, case, catalogue)
        else:
            raise ValueError("unexpected frame inside case")
    return {"status": 1, "completed_ticks": 0 if case is None else len(case["chain"]), "error_code": 10,
            "error": "missing CASE_FINAL", "case_result": None}


def _stored_progress(raw, case):
    """Validate the retained record, without repeating the native joint law."""
    _, I = _oracle()
    case = {key: list(value) if isinstance(value, list) else value for key, value in case.items()}
    r = Reader(raw)
    representative, previous, completed = r.unpack("IHH")
    if representative != case["begin"].representative or previous != len(case["chain"]) or not 1 <= completed - previous <= 19 or completed > H:
        raise ValueError("stored PROGRESS inventory")
    newly_first = {}
    for expected in range(previous + 1, completed + 1):
        offset, state, lift, events, chain, flags = read_tick(r, case["begin"].state)
        if offset != expected:
            raise ValueError("stored Tick offset gap")
        I.check_no_wrap(state, lift)
        # Payload/lift/event domains are checked by their independent codecs.
        # Observer identities are established by the separately required native
        # parity gate, and may be reexecuted by reconstruct_case_frames.
        for bit in range(4):
            if flags & (1 << bit):
                case["counts"][bit] += 1
                if not case["first"][bit]:
                    case["first"][bit] = offset
                    newly_first[bit] = offset
        case["history"].append(state)
        case["lifts"].append(lift)
        case["chain"].append(chain.hex())
        case["last_chain"] = chain
    mask = r.one("B")
    if mask != sum(1 << bit for bit in newly_first):
        raise ValueError("stored first-event mask mismatch")
    for bit, offset in sorted(newly_first.items()):
        if r.one("H") != offset:
            raise ValueError("stored first-event offset mismatch")
        if bit == 0:
            count = integer(r.one("H"), "first contact key count", 1, 2376)
            rows = []
            for _ in range(count):
                kind, site, column = r.unpack("BIB")
                integer(kind, "contact key kind", 0, len(KEY_NAMES) - 1)
                rows.append((KEY_NAMES[kind], site, column))
            keys_wire(rows)
        elif bit == 2:
            decode_events(r.blob(112, "H"))
        elif bit == 3:
            reason = integer(r.one("B"), "conflict reason", 0, 1)
            keys = []
            for high in (1, 2):
                count = integer(r.one("B"), "conflict count", 0, 2)
                rows = tuple(r.unpack("IB") for _ in range(count))
                if rows != tuple(sorted(set(rows))) or any(not 0 <= site < 64 ** 3 or not 0 <= column <= high for site, column in rows):
                    raise ValueError("conflict key domain/order")
                keys.append(rows)
            if (reason == 0 and any(keys)) or (reason == 1 and not any(keys)):
                raise ValueError("conflict reason/key mismatch")
    case["controls"] = _read_controls(r, case["history"][-1])
    r.done()
    case["blocks"] += 1
    if case["blocks"] > 16:
        raise ValueError("stored progress block count")
    return case


def decode_escape(raw, state):
    r = Reader(raw)
    flags, reason, count = r.unpack("BBB")
    integer(flags, "escape flags", 0, 7)
    integer(reason, "escape reason", 0, 3)
    if count not in (0, 2):
        raise ValueError("escape control cardinality")
    controls = _read_controls(r, state, count)
    drift_count = r.one("B")
    if drift_count not in (0, 2):
        raise ValueError("escape drift cardinality")
    drifts = tuple(r.unpack("bbb") for _ in range(drift_count))
    if any(any(v not in (-1, 0, 1) for v in drift) or sum(v != 0 for v in drift) != 1 for drift in drifts):
        raise ValueError("escape recurrent drift domain")
    period = integer(r.one("B"), "escape modular period", 0, 32)
    witnesses = []
    for kind in range(2):
        present = integer(r.one("B"), "escape witness present", 0, 1)
        if not present:
            witnesses.append(None)
            continue
        phase = integer(r.one("B"), "escape phase", 0, 37)
        point, slope = r.unpack("hhh"), r.unpack("bbb")
        if any(abs(v) > 512 for v in point) or any(v not in (-4, -2, 0, 2, 4) for v in slope):
            raise ValueError("escape affine witness domain")
        if kind == 0:
            lower = integer(r.one("I"), "escape interval lower", 0, 1023)
            has_upper = integer(r.one("B"), "escape upper present", 0, 1)
            upper = integer(r.one("I"), "escape upper", lower, 1023) if has_upper else None
            witness = (phase, point, slope, (lower, upper))
        else:
            n = integer(r.one("B"), "escape residue multiplier", 0, 31)
            residue = r.unpack("BBB")
            if any(v >= 64 for v in residue):
                raise ValueError("escape residue domain")
            witness = (phase, point, slope, n, residue)
        witnesses.append(witness)
    r.done()
    applicable, infinite, torus = (bool(flags & (1 << bit)) for bit in range(3))
    if reason != 3:
        if flags or drift_count or period or any(witnesses) or (reason < 2 and count):
            raise ValueError("inapplicable escape fields")
    elif not applicable or count != 2 or drift_count != 2 or period not in (1, 16, 32):
        raise ValueError("applicable escape fields")
    if applicable and ((witnesses[0] is None) != infinite or (witnesses[1] is None) != torus):
        raise ValueError("escape predicate/witness mismatch")
    return {"applicable": applicable, "infinite_lift": infinite, "periodic_torus": torus, "reason": reason,
            "controls": controls, "drifts": tuple(sorted(drifts)), "modular_period": period,
            "interval_witness": witnesses[0], "residue_witness": witnesses[1]}


def _stored_final(raw, case):
    """Exact terminal inventory/domain checks; no trajectory or predicate rerun."""
    S, _ = _oracle()
    r = Reader(raw)
    representative, completed, status = r.unpack("IHB")
    if representative != case["begin"].representative or completed != len(case["chain"]):
        raise ValueError("stored CASE_FINAL inventory")
    integer(status, "case status", 0, 4)
    if r.take(32) != case["last_chain"] or r.unpack("HHHH") != tuple(case["first"]) or r.unpack("HHHH") != tuple(case["counts"]):
        raise ValueError("stored CASE_FINAL counters/chain")
    graph = r.blob(284, "H")
    present = integer(r.one("B"), "certificate present", 0, 1)
    escape_bytes, bitmap = r.blob(66, "H"), r.take(287)
    classification, error_code = r.unpack("BH")
    error = r.blob(1024, "H").decode("ascii")
    r.done()
    integer(error_code, "case error code", 0, 11)
    if len(raw) > 2048 or "\0" in error:
        raise ValueError("stored terminal bound")
    if graph and graph != graph_wire(case["history"][-1]):
        raise ValueError("stored final graph differs from payload")
    result = {"status": status, "completed_ticks": completed, "error_code": error_code, "error": error}
    if status:
        if present or escape_bytes or any(bitmap) or classification != 3 or not error_code:
            raise ValueError("incomplete case contains a success certificate")
        return result
    if completed != H or case["blocks"] != 16 or not present or not graph or error_code or error:
        raise ValueError("incomplete records cannot establish COMPLETE_CASE")
    escape = decode_escape(escape_bytes, case["history"][-1])
    pairs = capture_pairs()
    if any(bitmap[index // 8] & (1 << (index % 8)) for index in range(len(pairs), len(bitmap) * 8)):
        raise ValueError("capture reserved bit")
    captures = []
    for index, (start, end) in enumerate(pairs):
        if not bitmap[index // 8] & (1 << (index % 8)):
            continue
        la, lb = case["lifts"][start], case["lifts"][end]
        sums = tuple(sum(b[a] for b in lb.carriers) - sum(b[a] for b in la.carriers) for a in range(3))
        if any(v % 8 for v in sums):
            raise ValueError("capture translation must be even and integral")
        translation = tuple(v // 4 for v in sums)
        diameter = max(max(max(p[a] for p in lift.carriers) - min(p[a] for p in lift.carriers) for a in range(3)) for lift in case["lifts"][start:end + 1])
        if diameter > 8:
            raise ValueError("capture window diameter outside predicate")
        captures.append((start, end, end - start, translation, diameter))
    expected = 0 if captures else 1 if escape["infinite_lift"] or escape["periodic_torus"] else 2
    if classification != expected:
        raise ValueError("stored classification erases a predicate")
    observed = S.observe(case["history"][-1])
    result.update(first=tuple(case["first"]), counts=tuple(case["counts"]), escape=escape, captures=tuple(captures),
                  classification=classification, graph_class=tuple((c.populations, c.F, c.K, c.Q, len(c.sites)) for c in observed.components))
    return result


def reduce_range(path, expected_header, admitted_map, groups, complete, consume, check=lambda: None):
    """Stream one range; only a semantically admitted frame extends its prefix."""
    group_pos, case, terminal = 0, None, None
    completed_groups = durable_ticks = 0
    terminals = hashlib.sha256()
    with RangeStream(path, expected_header) as stream:
        while True:
            check()
            frame = stream.next_frame()
            if frame is None:
                break
            try:
                kind, raw = frame["kind"], frame["body"]
                if kind == 1:
                    if case is not None or group_pos >= len(groups):
                        raise ValueError("unexpected owned BEGIN")
                    begin = admitted_map.begin(groups[group_pos])
                    if raw != begin_wire(begin):
                        raise ValueError("BEGIN differs from exact admitted execution group")
                    case = _new_case(begin)
                    group_pos += 1
                elif kind == 2:
                    if case is None:
                        raise ValueError("PROGRESS without BEGIN")
                    following = _stored_progress(raw, case)
                    durable_ticks += len(following["chain"]) - len(case["chain"])
                    case = following
                elif kind == 3:
                    if case is None:
                        raise ValueError("CASE_FINAL without BEGIN")
                    result = _stored_final(raw, case)
                    terminals.update(frame["digest"])
                    if result["status"] == 0:
                        group = case["begin"].group
                        if complete[group]:
                            raise ValueError("duplicate completed execution group")
                        consume(case["begin"], result)
                        complete[group] = 1
                        completed_groups += 1
                    case = None
                else:
                    row = decode_range_final(raw)
                    if case is not None and row["status"] == 0:
                        raise ValueError("complete range terminal inside an unfinished case")
                    local = sum(bool(complete[admitted_map.original(index)["group"]])
                                for index in range(expected_header["start"], expected_header["stop"])
                                if admitted_map.original(index)["owner_range"] == expected_header["range"])
                    expected = {"range": expected_header["range"], "start": expected_header["start"], "stop": expected_header["stop"],
                                "owned_groups": len(groups), "completed_groups": completed_groups, "verified_durable_ticks": durable_ticks,
                                "locally_covered_originals": local, "unresolved_aliases": expected_header["stop"] - expected_header["start"] - local,
                                "case_terminal_chain": terminals.hexdigest(), "group_map_sha256": expected_header["group_map_sha256"]}
                    if any(row[name] != value for name, value in expected.items()):
                        raise ValueError("RANGE_FINAL inventory differs from authenticated records/map")
                    if row["status"] == 0 and (completed_groups != len(groups) or group_pos != len(groups)
                            or row["reported_executed_ticks"] != 2 * durable_ticks or durable_ticks != H * len(groups)):
                        raise ValueError("COMPLETE range lacks all owned real ticks")
                    terminal = row
                stream.commit(frame)
            except (ValueError, UnicodeError, struct.error) as error:
                stream.reject(error)
                break
        report = stream.finish(check)
    report.update(range=expected_header["range"], terminal=terminal, owned_groups=len(groups), completed_groups=completed_groups,
                  verified_durable_ticks=durable_ticks, begun_groups=group_pos,
                  status="COMPLETE" if terminal and terminal["status"] == 0 and not report["tail_error"] and not report["tail_bytes"] else "INTERRUPTED")
    return report


def export_catalogue(npz_path, npz_sha256, output):
    """Load through the accepted factory; hash and verify the one emitted buffer."""
    _, I = _oracle()
    catalogue = I.load_catalogue(npz_path, npz_sha256)
    pins = dict(zip(("npz", "report", "audit", "q2_source", "dense_source"), (digest for _, digest in catalogue.pins)))
    arrays, body = [], bytearray()
    for name in sorted(catalogue.arrays):
        array = catalogue.arrays[name]
        dtype = array.dtype.newbyteorder("<")
        raw = array.astype(dtype, copy=False).tobytes(order="C")
        arrays.append({"name": name, "dtype": dtype.str, "shape": list(array.shape), "offset": len(body), "nbytes": len(raw)})
        body += raw
    if len(body) != 28297440:
        raise ValueError("catalogue raw volume mismatch")
    header = canonical({"schema": "q2-composite-export-1", "pins": pins, "arrays": arrays})
    if len(header) > 65536:
        raise ValueError("catalogue header bound")
    wire = EXPORT_MAGIC + blob(header) + body
    decode_export(wire, catalogue)
    write_new(output, wire)
    receipt = {"schema": "q2-composite-export-receipt-1", "path": str(Path(output).resolve()),
               "sha256": hashlib.sha256(wire).hexdigest(), "pins": pins,
               "raw_bytes": len(body), "arrays_compared": len(arrays), "factory": "recovery_composite_interaction.load_catalogue"}
    write_new(Path(str(output) + ".receipt.json"), canonical(receipt))
    return receipt


def decode_export(wire, catalogue):
    """Codec comparison primitive; its public Catalogue argument is not admission.

    Production uses admit_export or export_catalogue, both of which call the
    pinned factory themselves before consulting any array values.
    """
    import numpy as np
    r = Reader(wire)
    if r.take(len(EXPORT_MAGIC)) != EXPORT_MAGIC:
        raise ValueError("catalogue export magic")
    header = parse_json(r.blob(65536), {"schema", "pins", "arrays"})
    pins = dict(zip(("npz", "report", "audit", "q2_source", "dense_source"), (digest for _, digest in catalogue.pins)))
    if header["schema"] != "q2-composite-export-1" or header["pins"] != pins:
        raise ValueError("catalogue export source identity")
    if type(header["arrays"]) is not list or len(header["arrays"]) != 12:
        raise ValueError("complete catalogue inventory required")
    offset, decoded = 0, {}
    for row, name in zip(header["arrays"], sorted(catalogue.arrays)):
        expected = catalogue.arrays[name]
        if type(row) is not dict or set(row) != {"name", "dtype", "shape", "offset", "nbytes"}:
            raise ValueError("catalogue array descriptor")
        dtype = expected.dtype.newbyteorder("<")
        exact = {"name": name, "dtype": dtype.str, "shape": list(expected.shape), "offset": offset, "nbytes": expected.nbytes}
        # Canonical bytes distinguish booleans from integer offsets/dimensions.
        if canonical(row) != canonical(exact):
            raise ValueError("catalogue array layout mismatch")
        raw = r.take(expected.nbytes)
        if raw != expected.astype(dtype, copy=False).tobytes(order="C"):
            raise ValueError("catalogue value mismatch")
        decoded[name] = np.frombuffer(raw, dtype=dtype).reshape(expected.shape)
        offset += len(raw)
    r.done()
    return header, decoded


def admit_export(npz_path, npz_sha256, export_path, export_sha256):
    _, I = _oracle()
    catalogue = I.load_catalogue(npz_path, npz_sha256)
    raw = Path(export_path).read_bytes()
    if hashlib.sha256(raw).hexdigest() != hash_text(export_sha256):
        raise ValueError("owned export byte snapshot identity mismatch")
    header, arrays = decode_export(raw, catalogue)
    return catalogue, header, arrays


def export_templates(catalogue, output, check=lambda: None):
    """Every accepted recurrent node's exact finite support, with no evolution."""
    _, I = _oracle()
    import numpy as np
    nodes = np.flatnonzero(catalogue.arrays["transient_to_cycle"] == 0)
    if len(nodes) != 1824:
        raise ValueError("complete recurrent catalogue support inventory required")
    header = {"schema": "q4-composite-support-templates-1", "catalogue_sha256": catalogue.sha256,
              "oracle_source_sha256": FROZEN["scripts/phi_v2_lattice/recovery_composite_interaction.py"],
              "count": 1824, "coordinate_encoding": "relative-i8-triples-1"}
    with Path(output).open("xb") as stream:
        stream.write(TEMPLATE_MAGIC + blob(canonical(header)))
        for index, node in enumerate(nodes):
            if index % 32 == 0:
                check()
            q = I.Q2.decode(int(node))
            point = tuple(q.color)
            control = I.Q2Control(64, q.stage, 0, 0, point, int(node))
            support = tuple(tuple(v - origin for v, origin in zip(row, point)) for row in I._control_support(control))
            if len(support) > 375 or support != tuple(sorted(set(support))):
                raise ValueError("finite support bound/order")
            stream.write(struct.pack("<IH", int(node), len(support)))
            for triple in support:
                stream.write(struct.pack("<bbb", *(integer(v, "relative support coordinate", -128, 127) for v in triple)))
        stream.flush(); os.fsync(stream.fileno())
    if Path(output).stat().st_size >= 3 << 20:
        raise ValueError("support template reservation exceeded")
    return {"path": str(Path(output).resolve()), "sha256": sha(output)}


def validate_templates(raw, catalogue):
    _, I = _oracle()
    r = Reader(raw)
    if r.take(len(TEMPLATE_MAGIC)) != TEMPLATE_MAGIC:
        raise ValueError("template magic")
    header = metadata_json(r.blob(65536), {"schema", "catalogue_sha256", "oracle_source_sha256", "count", "coordinate_encoding"})
    expected = {"schema": "q4-composite-support-templates-1", "catalogue_sha256": catalogue.sha256,
                "oracle_source_sha256": FROZEN["scripts/phi_v2_lattice/recovery_composite_interaction.py"], "count": 1824,
                "coordinate_encoding": "relative-i8-triples-1"}
    if canonical(header) != canonical(expected):
        raise ValueError("template source/schema mismatch")
    import numpy as np
    nodes = np.flatnonzero(catalogue.arrays["transient_to_cycle"] == 0)
    for node in nodes:
        actual, count = r.unpack("IH")
        integer(count, "support site count", 0, 375)
        if actual != int(node):
            raise ValueError("template node inventory/order mismatch")
        support = tuple(r.unpack("bbb") for _ in range(count))
        q = I.Q2.decode(actual)
        point = tuple(q.color)
        control = I.Q2Control(64, q.stage, 0, 0, point, actual)
        expected_support = tuple(tuple(v - p for v, p in zip(row, point)) for row in I._control_support(control))
        if support != expected_support:
            raise ValueError("template omits or changes exact support")
    r.done()
    return header


@lru_cache(maxsize=1)
def action_tables():
    """The finite physical background action, with no state evolution."""
    axes = tuple(itertools.permutations(range(3)))
    matrices = sorted(tuple(tuple(signs[i] if j == permutation[i] else 0 for j in range(3)) for i in range(3))
                      for permutation in axes for signs in itertools.product((-1, 1), repeat=3))
    actions = tuple((matrix, charge) for matrix in matrices for charge in range(2))
    index = {action: i for i, action in enumerate(actions)}
    inverse, products, backgrounds = [], [], []
    for g, charge in actions:
        inverse.append(index[tuple(zip(*g)), charge])
        products.append(tuple(index[tuple(tuple(sum(g[i][k] * h[k][j] for k in range(3)) for j in range(3)) for i in range(3)), charge ^ other]
                              for h, other in actions))
        row = []
        for background in range(96):
            origin, eta = divmod(background, 2)
            new_axes, bits = [], 0
            for a, old_axis in enumerate(axes[origin // 8]):
                target = next(i for i in range(3) if g[i][old_axis])
                new_axes.append(target)
                bits |= (((origin >> a) & 1) ^ (g[target][old_axis] == -1)) << a
            row.append(2 * (8 * axes.index(tuple(new_axes)) + bits) + (eta ^ charge))
        backgrounds.append(tuple(row))
    return actions, tuple(inverse), tuple(products), tuple(backgrounds)


def actions_wire():
    actions, inverse, products, backgrounds = action_tables()
    body = b"".join(struct.pack("<9bBB", *(v for row in g for v in row), charge, inv) for (g, charge), inv in zip(actions, inverse))
    return ACTION_MAGIC + body + bytes(v for row in products for v in row) + bytes(v for row in backgrounds for v in row)


def normalize_action(state):
    S, _ = _oracle()
    actions, _, _, backgrounds = action_tables()
    source = 2 * state.origin_code + state.charge_frame
    candidates = [i for i, row in enumerate(backgrounds) if row[source] == 0]
    if len(candidates) != 1:
        raise ValueError("background action is not free and transitive")
    g, charge = actions[candidates[0]]
    shift = tuple(32 - 32 * sum(row) for row in g)
    normalized = S.transform(state, g, shift, bool(charge))
    if normalized.origin_code or normalized.charge_frame:
        raise ValueError("background normalization mismatch")
    return normalized, candidates[0]


def preparation_wire(preparation):
    _, I = _oracle()
    p = I._preparation(preparation)
    color = sum(v << a for a, v in enumerate(p.color))
    return struct.pack("<BBBbbBbbBB", color, p.v_A, p.v_B, p.sigma_A, p.sigma_B, p.branch, p.m, p.n, p.stage_advance, p.variant)


def _read_preparation(r, tier):
    _, I = _oracle()
    color, *fields = r.unpack("BBBbbBbbBB")
    if color >> 3:
        raise ValueError("preparation reserved color bits")
    return I._preparation(I.PreparationID(tier, tuple((color >> a) & 1 for a in range(3)), *fields))


def original_sort_record(index, state, controls):
    S, _ = _oracle()
    checkpoint = S.checkpoint(state)
    row = struct.pack("<IH", integer(index, "original index", 0, 1094399), len(checkpoint)) + checkpoint + controls_wire(controls, state)
    if len(row) > 2048:
        raise ValueError("original sort record cap")
    return struct.pack("<H", len(row)) + row


def normalized_sort_record(index, normalized):
    S, _ = _oracle()
    checkpoint = S.checkpoint(normalized)
    row = struct.pack("<IH", integer(index, "original index", 0, 1094399), len(checkpoint)) + checkpoint
    if len(row) > 2048:
        raise ValueError("normalized sort record cap")
    return struct.pack("<H", len(row)) + row


def decode_sort_record(raw, normalized=False):
    S, _ = _oracle()
    outer = Reader(raw)
    r = Reader(outer.blob(2048, "H"))
    outer.done()
    index = integer(r.one("I"), "original index", 0, 1094399)
    checkpoint = r.blob(699, "H")
    state = _state(S.restore(checkpoint))
    if state.L != 64 or not 1 <= state.microtick <= 19:
        raise ValueError("campaign sort state domain")
    controls = () if normalized else _read_controls(r, state)
    r.done()
    if normalized and (state.origin_code or state.charge_frame):
        raise ValueError("normalized sort background")
    return index, checkpoint, controls


def _sort_key(raw, normalized=False):
    index, checkpoint, controls = decode_sort_record(raw, normalized)
    if normalized:
        return checkpoint, index
    ordered = tuple((c.L, c.microtick, c.origin_code, c.charge_frame, c.chart_plus_position, c.node) for c in controls)
    return checkpoint, *ordered, index


def iter_sort_records(path, expected_sha256=None):
    """At most one bounded record is allocated; every input byte is consumed."""
    digest = hashlib.sha256()
    with Path(path).open("rb") as stream:
        while True:
            prefix = stream.read(2)
            if not prefix:
                if expected_sha256 is not None and digest.hexdigest() != hash_text(expected_sha256):
                    raise ValueError("owned sort stream identity changed")
                return
            if len(prefix) != 2:
                raise ValueError("truncated sort-record prefix")
            size = integer(struct.unpack("<H", prefix)[0], "sort record length", 1, 2048)
            body = stream.read(size)
            if len(body) != size:
                raise ValueError("truncated sort record")
            digest.update(prefix)
            digest.update(body)
            yield prefix + body


def external_sort_stream(source, directory, normalized, chunk_records, check=lambda: None, source_sha256=None):
    """Exactly input -> sorted runs -> merged output; all generations survive."""
    directory = Path(directory)
    directory.mkdir(exist_ok=False)
    integer(chunk_records, "sort chunk records", 1, 17100)
    runs, run_hashes, chunk, count = [], {}, [], 0
    source_sha256 = sha(source) if source_sha256 is None else hash_text(source_sha256)

    def emit_run():
        if len(runs) >= 64:
            raise ValueError("at-most-64-way merge bound exceeded")
        check()
        chunk.sort(key=lambda row: row[0])
        path = directory / ("run-%02d.bin" % len(runs))
        digest = hashlib.sha256()
        with path.open("xb") as stream:
            for _, raw in chunk:
                stream.write(raw)
                digest.update(raw)
            stream.flush(); os.fsync(stream.fileno())
        runs.append(path)
        run_hashes[str(path.resolve())] = digest.hexdigest()
        chunk.clear()

    for raw in iter_sort_records(source, source_sha256):
        chunk.append((_sort_key(raw, normalized), raw))
        count += 1
        if len(chunk) == chunk_records:
            emit_run()
    if chunk:
        emit_run()
    output = directory / "merged.bin"
    inputs = [iter_sort_records(path, run_hashes[str(path.resolve())]) for path in runs]
    queue = []
    for i, records in enumerate(inputs):
        raw = next(records, None)
        if raw is not None:
            heapq.heappush(queue, (_sort_key(raw, normalized), i, raw))
    written = 0
    merged_hash = hashlib.sha256()
    with output.open("xb") as stream:
        previous = None
        while queue:
            key, i, raw = heapq.heappop(queue)
            if previous is not None and key < previous:
                raise ValueError("external merge ordering failure")
            stream.write(raw)
            merged_hash.update(raw)
            previous = key
            written += 1
            if written % 256 == 0:
                check()
            following = next(inputs[i], None)
            if following is not None:
                heapq.heappush(queue, (_sort_key(following, normalized), i, following))
        stream.flush(); os.fsync(stream.fileno())
    if written != count:
        raise ValueError("external merge lost a complete original")
    check()
    return {"source": str(Path(source).resolve()), "runs": tuple(str(path.resolve()) for path in runs),
            "merged": str(output.resolve()), "records": count, "generations": 3,
            "source_sha256": source_sha256, "run_sha256": run_hashes, "merged_sha256": merged_hash.hexdigest(),
            "retained_bytes": Path(source).stat().st_size + sum(path.stat().st_size for path in runs) + output.stat().st_size}


def _class_arrays(original, normalized, n, check, original_sha256=None, normalized_sha256=None):
    integer(n, "class array original count", 1, 1094400)
    names = ("state", "execution", "orbit")
    arrays = {kind: {field: array("I", [0]) * n for field in ("member", "representative", "multiplicity", "remap")} for kind in names}
    if any(values["member"].itemsize != 4 for values in arrays.values()):
        raise ValueError("exact uint32 class arrays required")
    counts = dict.fromkeys(names, 0)
    previous_state = previous_execution = previous_orbit = None
    previous_key = None
    seen = bytearray(n)
    visited = 0
    for raw in iter_sort_records(original, original_sha256):
        key = _sort_key(raw)
        if previous_key is not None and key <= previous_key:
            raise ValueError("original merged stream not strictly sorted")
        previous_key = key
        index = key[-1]
        integer(index, "original index", 0, n - 1)
        if seen[index]:
            raise ValueError("duplicate original in class merge")
        seen[index] = 1
        state, execution = key[0], key[:-1]
        for kind, value, previous in (("state", state, previous_state), ("execution", execution, previous_execution)):
            data = arrays[kind]
            if value != previous:
                label = counts[kind]
                counts[kind] += 1
                data["representative"][label] = index
            label = counts[kind] - 1
            data["member"][index] = label
            data["representative"][label] = min(data["representative"][label], index)
            data["multiplicity"][label] += 1
        previous_state, previous_execution = state, execution
        visited += 1
        if visited % 256 == 0:
            check()
    if visited != n or not all(seen):
        raise ValueError("original class coverage incomplete")
    seen[:] = bytes(n)
    visited = 0
    previous_key = None
    for raw in iter_sort_records(normalized, normalized_sha256):
        key = _sort_key(raw, True)
        if previous_key is not None and key <= previous_key:
            raise ValueError("normalized merged stream not strictly sorted")
        previous_key = key
        index = key[-1]
        integer(index, "normalized original index", 0, n - 1)
        if seen[index]:
            raise ValueError("duplicate normalized original")
        seen[index] = 1
        data = arrays["orbit"]
        if key[0] != previous_orbit:
            label = counts["orbit"]
            counts["orbit"] += 1
            data["representative"][label] = index
        label = counts["orbit"] - 1
        data["member"][index] = label
        data["representative"][label] = min(data["representative"][label], index)
        data["multiplicity"][label] += 1
        previous_orbit = key[0]
        visited += 1
        if visited % 256 == 0:
            check()
    if visited != n or not all(seen) or not counts["state"] <= counts["execution"] <= n:
        raise ValueError("normalized or equivalence coverage incomplete")
    # Twelve native uint32 arrays + one-byte seen map and one remapping list
    # stay below256MiB at the full registered count; never32 private copies.
    resident = sum(sys.getsizeof(a) for values in arrays.values() for a in values.values()) + sys.getsizeof(seen)
    for kind in names:
        values = arrays[kind]
        order = sorted(range(counts[kind]), key=values["representative"].__getitem__)
        measured = resident + sys.getsizeof(order) + sum(sys.getsizeof(i) for i in order)
        if measured > 256 << 20:
            raise ValueError("aggregate preparation/class-remap allocation bound")
        for new, old in enumerate(order):
            values["remap"][old] = new
        del order
    return arrays, counts


def _class_row(arrays, kind, index):
    values = arrays[kind]
    old = values["member"][index]
    return values["remap"][old], values["representative"][old], values["multiplicity"][old]


def preparation_seed_nodes(preparation):
    """Original-specific frozen seed recipe; no transition or ID search."""
    _, I = _oracle()
    p = I._preparation(preparation)
    origin = sum(v << a for a, v in enumerate(p.color))
    a, b = (32, 32, 32), I._add((32, 32, 32), I.preparation_displacement(p))
    ka = kb = None
    if 1 <= p.variant <= 5:
        ka = tuple(v for v in range(1, 7) if v != p.v_A)[p.variant - 1]
    elif p.variant >= 6:
        kb = tuple(v for v in range(1, 7) if v != p.v_B)[p.variant - 6]
    return (I._seed(p.v_A, p.sigma_A, a, origin, ka).node, I._seed(p.v_B, p.sigma_B, b, origin, kb).node)


def reconstruct_alias_case(original_row, admitted_begin, representative_result):
    """Restore one original's full result from an already admitted execution group.

    The immutable map/factory admission must already establish that the original
    has this complete initial state and these exact ordered isolated controls.
    This read-only helper checks the supplied identities and result provenance;
    it does not rerun preparation or certify an arbitrary synthetic CaseResult.
    Only preparation_id and original-specific provenance are substituted.
    """
    S, I = _oracle()
    tier = admitted_begin.preparation.tier
    if (type(original_row) is not dict
            or decode_original_row(original_row_wire(original_row), tier) != original_row
            or decode_begin(begin_wire(admitted_begin), tier) != admitted_begin):
        raise ValueError("alias requires canonical admitted original and BEGIN records")
    p = original_row["preparation"]
    checkpoint = S.checkpoint(admitted_begin.state)
    if (p.tier != tier or original_row["group"] != admitted_begin.group
            or original_row["representative"] != admitted_begin.representative
            or original_row["multiplicity"] != admitted_begin.multiplicity
            or original_row["orbit"] != admitted_begin.orbit
            or original_row["representative"] > original_row["index"]
            or original_row["checkpoint_sha256"] != hashlib.sha256(checkpoint).hexdigest()
            or original_row["background"] != 2 * admitted_begin.state.origin_code + admitted_begin.state.charge_frame
            or sum(v << a for a, v in enumerate(p.color)) != admitted_begin.state.origin_code
            or admitted_begin.state.charge_frame != 0
            or 1 + p.stage_advance != admitted_begin.lift_steps):
        raise ValueError("original alias differs from admitted execution group identity")
    if (admitted_begin.seed_nodes != preparation_seed_nodes(admitted_begin.preparation)
            or (original_row["index"] == admitted_begin.representative and p != admitted_begin.preparation)):
        raise ValueError("admitted representative preparation provenance mismatch")
    provenance = (("catalogue_sha256", I.CATALOGUE_SHA256), ("seed_nodes", admitted_begin.seed_nodes),
                  ("lift_steps", admitted_begin.lift_steps),
                  ("final_nodes", tuple(control.node for control in admitted_begin.controls)))
    if (type(representative_result) is not I.CaseResult
            or I._preparation(representative_result.preparation_id) != admitted_begin.preparation
            or representative_result.provenance != provenance
            or repr(representative_result.provenance) != repr(provenance)
            or type(representative_result.completed_ticks) is not int or representative_result.completed_ticks != H
            or len(representative_result.checkpoints) != H + 1 or representative_result.checkpoints[0] != checkpoint
            or len(representative_result.hash_chain) != H or len(representative_result.accounts) != H
            or len(representative_result.lift_observations) != H + 1):
        raise ValueError("representative CaseResult identity, provenance or complete inventory mismatch")
    original_provenance = (("catalogue_sha256", I.CATALOGUE_SHA256), ("seed_nodes", preparation_seed_nodes(p)),
                           ("lift_steps", 1 + p.stage_advance),
                           ("final_nodes", tuple(control.node for control in admitted_begin.controls)))
    return replace(representative_result, preparation_id=p, provenance=original_provenance)


def prepare_map(catalogue, tier, descriptor_sha256, action_sha256, directory, check):
    """Production implementation only: caller must already hold the reviewed Job.

    This function is never invoked by bounded readiness tests. The registered
    originals are generated once, all sort generations retained, and only
    twelve fixed uint32 arrays are resident while the final map is streamed.
    """
    S, I = _oracle()
    # A constructible public Catalogue is not a capability. Re-enter the
    # accepted factory and compare the supplied handle before any preparation.
    accepted = I.load_catalogue(catalogue.path, catalogue.sha256)
    if (catalogue.pins != accepted.pins or set(catalogue.arrays) != set(accepted.arrays)
            or any(catalogue.arrays[name].dtype != actual.dtype or catalogue.arrays[name].shape != actual.shape
                   or catalogue.arrays[name].tobytes() != actual.tobytes() for name, actual in accepted.arrays.items())):
        raise ValueError("forged or changed public catalogue handle")
    catalogue = accepted
    n = TIER_COUNTS[tier]
    directory = Path(directory)
    directory.mkdir(exist_ok=False)
    original, normalized = directory / "original-input.bin", directory / "normalized-input.bin"
    original_hash, normalized_hash = hashlib.sha256(), hashlib.sha256()
    with original.open("xb") as a, normalized.open("xb") as b:
        produced = 0
        for index, pid in enumerate(I.preparation_ids(tier)):
            if index % 64 == 0:
                check()
            prepared = I.prepare(pid, catalogue)
            original_raw = original_sort_record(index, prepared.state, prepared.controls)
            normalized_raw = normalized_sort_record(index, normalize_action(prepared.state)[0])
            a.write(original_raw)
            b.write(normalized_raw)
            original_hash.update(original_raw)
            normalized_hash.update(normalized_raw)
            produced += 1
        a.flush(); b.flush(); os.fsync(a.fileno()); os.fsync(b.fileno())
    if produced != n:
        raise ValueError("registered preparation inventory mismatch")
    chunk = (n + 63) // 64
    original_sort = external_sort_stream(original, directory / "original-sort", False, chunk, check, original_hash.hexdigest())
    normalized_sort = external_sort_stream(normalized, directory / "normalized-sort", True, chunk, check, normalized_hash.hexdigest())
    arrays, counts = _class_arrays(original_sort["merged"], normalized_sort["merged"], n, check,
                                  original_sort["merged_sha256"], normalized_sort["merged_sha256"])
    header = {"schema": "q4-composite-map-2", "tier": tier, "N": n, "G_state": counts["state"], "G_execution": counts["execution"],
              "O": counts["orbit"], "input_descriptor_sha256": hash_text(descriptor_sha256), "action_table_sha256": hash_text(action_sha256), "group_key": GROUP_KEY}
    raw_header = canonical(header)
    if len(raw_header) > 2048:
        raise ValueError("map header bound")
    output = directory / "preparation-map.bin"
    with output.open("xb") as stream:
        stream.write(MAP_MAGIC + blob(raw_header))
        for index, (raw, pid) in enumerate(zip(iter_sort_records(original, original_hash.hexdigest()), I.preparation_ids(tier))):
            actual, checkpoint, controls = decode_sort_record(raw)
            if actual != index:
                raise ValueError("original input order changed")
            state = S.restore(checkpoint)
            group, representative, multiplicity = _class_row(arrays, "execution", index)
            state_group, state_rep, state_multiplicity = _class_row(arrays, "state", index)
            orbit = _class_row(arrays, "orbit", index)[0]
            row = {"index": index, "preparation": pid, "checkpoint_sha256": hashlib.sha256(checkpoint).hexdigest(),
                   "group": group, "representative": representative, "multiplicity": multiplicity, "orbit": orbit,
                   "background": 2 * state.origin_code + state.charge_frame, "owner_range": representative // (n // 64),
                   "state_group": state_group, "state_representative": state_rep, "state_multiplicity": state_multiplicity}
            stream.write(original_row_wire(row))
            if index % 256 == 0:
                check()
        for index, (raw, pid) in enumerate(zip(iter_sort_records(original, original_hash.hexdigest()), I.preparation_ids(tier))):
            group, representative, multiplicity = _class_row(arrays, "execution", index)
            if index != representative:
                continue
            _, checkpoint, controls = decode_sort_record(raw)
            state = S.restore(checkpoint)
            begin = Begin(group, representative, multiplicity, _class_row(arrays, "orbit", index)[0], pid, state,
                          I.initial_lift(state), controls, preparation_seed_nodes(pid), 1 + pid.stage_advance)
            body = begin_wire(begin)
            stream.write(struct.pack("<H", len(body)) + body)
            check()
        for index, raw in enumerate(iter_sort_records(normalized, normalized_hash.hexdigest())):
            orbit, representative, _ = _class_row(arrays, "orbit", index)
            if index == representative:
                _, checkpoint, _ = decode_sort_record(raw, True)
                stream.write(orbit_row_wire(orbit, S.restore(checkpoint)))
            if index % 256 == 0:
                check()
        stream.flush(); os.fsync(stream.fileno())
    check()
    if output.stat().st_size > 844 * n + len(MAP_MAGIC) + 8 + 2048:
        raise ValueError("map byte reservation exceeded")
    receipt = {"schema": "q4-composite-map-construction-2", "path": str(output.resolve()), "sha256": sha(output), "header": header,
               "original_sort": original_sort, "normalized_sort": normalized_sort, "originals_admitted": n,
               "group_key": GROUP_KEY, "class_array_bytes": sum(sys.getsizeof(a) for values in arrays.values() for a in values.values())}
    write_new(directory / "map-construction.json", canonical(receipt))
    return receipt


class MapIndex:
    """Read-only mapped rows with bounded uint64 offsets, never a BEGIN object list."""
    def __init__(self, path, digest, descriptor_sha256, action_sha256):
        self.path = Path(path).resolve()
        if sha(self.path) != hash_text(digest):
            raise ValueError("map bytes changed")
        self.stream = self.path.open("rb")
        self.mapping = mmap.mmap(self.stream.fileno(), 0, access=mmap.ACCESS_READ)
        self.header = None
        try:
            r = Reader(self.mapping)
            if r.take(len(MAP_MAGIC)) != MAP_MAGIC:
                raise ValueError("V2 map magic")
            self.header = parse_json(r.blob(2048), MAP_KEYS)
            h = self.header
            if h["schema"] != "q4-composite-map-2" or h["tier"] not in TIER_COUNTS or h["group_key"] != GROUP_KEY:
                raise ValueError("map schema/group key")
            n = integer(h["N"], "N", 1, 1094400)
            if n != TIER_COUNTS[h["tier"]] or h["input_descriptor_sha256"] != descriptor_sha256 or h["action_table_sha256"] != action_sha256:
                raise ValueError("map input identity/count")
            gs = integer(h["G_state"], "G_state", 1, n)
            ge = integer(h["G_execution"], "G_execution", gs, n)
            orbit_count = integer(h["O"], "O", 1, n)
            self.original_offset = r.pos
            r.pos += 76 * n
            if r.remaining < 0:
                raise ValueError("truncated original map rows")
            self.group_offsets = array("Q")
            self.orbit_offsets = array("Q")
            previous_rep = -1
            for group in range(ge):
                self.group_offsets.append(r.pos)
                raw = r.blob(510, "H")
                # Counts/owners are cheap fixed fields; full preparation and
                # BEGIN validation occurs on consumption, one group at a time.
                if len(raw) < 16:
                    raise ValueError("short group row")
                gid, rep, multiplicity, orbit = struct.unpack_from("<IIII", raw)
                if gid != group or not previous_rep < rep < n or not 1 <= multiplicity <= n or orbit >= orbit_count:
                    raise ValueError("group row identity/order")
                previous_rep = rep
            for orbit in range(orbit_count):
                self.orbit_offsets.append(r.pos)
                raw = r.blob(254, "H")
                if len(raw) < 4 or struct.unpack_from("<I", raw)[0] != orbit:
                    raise ValueError("orbit row identity/order")
            r.done()
            # Release the exported memoryview before closing the mapping.
            r.data.release()
            del r
        except BaseException:
            if "r" in locals():
                r.data.release()
                del r
            self.close()
            raise

    def close(self):
        if getattr(self, "mapping", None) is not None:
            self.mapping.close()
            self.mapping = None
        if getattr(self, "stream", None) is not None:
            self.stream.close()
            self.stream = None

    def __enter__(self):
        return self

    def __exit__(self, *_):
        self.close()

    def original(self, index):
        integer(index, "original index", 0, self.header["N"] - 1)
        start = self.original_offset + index * 76
        row = decode_original_row(self.mapping[start:start + 76], self.header["tier"])
        if row["index"] != index:
            raise ValueError("original map order")
        return row

    def begin(self, group):
        integer(group, "execution group", 0, self.header["G_execution"] - 1)
        offset = self.group_offsets[group]
        size = struct.unpack_from("<H", self.mapping, offset)[0]
        return decode_begin(self.mapping[offset + 2:offset + 2 + size], self.header["tier"])

    def group_metadata(self, group):
        integer(group, "execution group", 0, self.header["G_execution"] - 1)
        return struct.unpack_from("<IIII", self.mapping, self.group_offsets[group] + 2)

    def orbit(self, orbit):
        integer(orbit, "orbit", 0, self.header["O"] - 1)
        offset = self.orbit_offsets[orbit]
        size = struct.unpack_from("<H", self.mapping, offset)[0]
        return decode_orbit_row(self.mapping[offset:offset + 2 + size])[1]


def equivalence_maps(records):
    """Bounded independent control utility; production must use retained sorts.

    Each supplied record is (original index, state, ordered controls). This
    function generates no preparation and does not run a transition.
    """
    S, _ = _oracle()
    records = tuple(records)
    if len(records) > 4096 or tuple(row[0] for row in records) != tuple(range(len(records))):
        raise ValueError("bounded contiguous synthetic records required")
    keys = [[], [], []]
    for index, state, controls in records:
        raw = S.checkpoint(state)
        control = controls_wire(controls, state)
        keys[0].append(raw)
        keys[1].append((raw, control))
        keys[2].append(S.checkpoint(normalize_action(state)[0]))
    maps = []
    for kind in keys:
        groups = {}
        for i, key in enumerate(kind):
            groups.setdefault(key, []).append(i)
        members = sorted(groups.values(), key=lambda row: row[0])
        mapping = [None] * len(records)
        for group, members_row in enumerate(members):
            for i in members_row:
                mapping[i] = (group, members_row[0], len(members_row))
        maps.append(tuple(mapping))
    return {"state": maps[0], "execution": maps[1], "orbit": maps[2],
            "G_state": len(set(keys[0])), "G_execution": len(set(keys[1])), "O": len(set(keys[2]))}


def validate_all_aliases(admitted_map, check=lambda: None):
    """Every original is counted once; all three class maps retain their counts."""
    h = admitted_map.header
    n, groups, states, orbits = (h[name] for name in ("N", "G_execution", "G_state", "O"))
    counts = array("I", [0]) * groups
    state_counts = array("I", [0]) * states
    state_reps = array("I", [n]) * states
    state_expected = array("I", [0]) * states
    orbit_counts = array("I", [0]) * orbits
    by_range = [[] for _ in range(64)]
    for group in range(groups):
        gid, rep, multiplicity, orbit = admitted_map.group_metadata(group)
        if gid != group:
            raise ValueError("execution group order")
        by_range[rep // (n // 64)].append(group)
    for index in range(n):
        if index % 256 == 0:
            check()
        row = admitted_map.original(index)
        group, state, orbit = row["group"], row["state_group"], row["orbit"]
        if not 0 <= group < groups or not 0 <= state < states or not 0 <= orbit < orbits:
            raise ValueError("alias refers to absent class")
        gid, rep, multiplicity, group_orbit = admitted_map.group_metadata(group)
        if (row["representative"], row["multiplicity"], orbit) != (rep, multiplicity, group_orbit) or rep > index or row["state_representative"] > index:
            raise ValueError("alias execution representative/count provenance")
        representative = admitted_map.original(rep)
        if (row["checkpoint_sha256"] != representative["checkpoint_sha256"]
                or row["background"] != representative["background"] or row["state_group"] != representative["state_group"]):
            raise ValueError("execution alias differs from representative state identity")
        if not state_counts[state]:
            state_reps[state], state_expected[state] = row["state_representative"], row["state_multiplicity"]
        if (state_reps[state], state_expected[state]) != (row["state_representative"], row["state_multiplicity"]):
            raise ValueError("state-only alias representative/count provenance")
        state_original = admitted_map.original(row["state_representative"])
        if state_original["state_group"] != state or row["checkpoint_sha256"] != state_original["checkpoint_sha256"]:
            raise ValueError("state-only alias checkpoint identity")
        counts[group] += 1
        state_counts[state] += 1
        orbit_counts[orbit] += 1
    if any(not value for value in counts) or any(not value for value in state_counts) or any(not value for value in orbit_counts):
        raise ValueError("class map has an empty or missing class")
    if tuple(state_reps) != tuple(sorted(state_reps)) or any(state_counts[i] != state_expected[i] for i in range(states)):
        raise ValueError("state-only canonical order/count")
    for group in range(groups):
        if counts[group] != admitted_map.group_metadata(group)[2]:
            raise ValueError("execution multiplicity differs from all original rows")
    return counts, tuple(tuple(values) for values in by_range)


def decode_native_terminal(raw, range_report):
    keys = {"schema", "range", "complete", "reported_executed_ticks", "primary_executed_ticks", "certificate_replay_ticks",
            "verified_durable_ticks", "support_cache_used", "canonical_adoption"}
    if len(raw) > 65536 or not raw.endswith(b"\n"):
        raise ValueError("native terminal JSON bound/newline")
    row = parse_json(raw, keys, False)
    if (row["schema"] != "q4-composite-native-range-terminal-1" or type(row["complete"]) is not bool
            or row["canonical_adoption"] is not False or row["support_cache_used"] is not False):
        raise ValueError("native terminal schema/capability")
    integer(row["range"], "native terminal range", 0, 63)
    for name in ("reported_executed_ticks", "primary_executed_ticks", "certificate_replay_ticks", "verified_durable_ticks"):
        integer(row[name], name)
    if (row["reported_executed_ticks"] != row["primary_executed_ticks"] + row["certificate_replay_ticks"]
            or row["verified_durable_ticks"] > row["primary_executed_ticks"] or row["range"] != range_report["range"]):
        raise ValueError("native invocation/durable accounting")
    terminal = range_report["terminal"]
    if terminal is not None and any(row[name] != terminal[name] for name in ("reported_executed_ticks", "verified_durable_ticks")):
        raise ValueError("native stdout differs from authenticated terminal")
    if row["complete"] != (range_report["status"] == "COMPLETE"):
        raise ValueError("native stdout cannot elevate an incomplete range")
    if row["complete"] and (row["primary_executed_ticks"] != range_report["verified_durable_ticks"]
            or row["certificate_replay_ticks"] != range_report["verified_durable_ticks"]):
        raise ValueError("complete native primary/replay split")
    return row


class Reduction:
    def __init__(self):
        self.unique = {}
        self.weighted = {}
        self.groups = self.originals = 0

    def add(self, begin, result):
        weight = begin.multiplicity
        self.groups += 1
        self.originals += weight
        def count(name, value, amount=1):
            key = canonical([name, value]).decode("ascii")
            self.unique[key] = self.unique.get(key, 0) + amount
            self.weighted[key] = self.weighted.get(key, 0) + amount * weight
        for name, first, total in zip(("contact", "effect", "event_difference", "counterfactual_invalid"), result["first"], result["counts"]):
            count("first_" + name, first or None)
            count("total_" + name, total)
        escape = result["escape"]
        for name in ("applicable", "infinite_lift", "periodic_torus"):
            count("escape_" + name, escape[name])
        count("escape_reason", escape["reason"])
        count("unordered_drifts", escape["drifts"])
        count("final_graph_component_accounts", result["graph_class"])
        count("classification", result["classification"])
        count("capture_case", bool(result["captures"]))
        for start, end, period, translation, diameter in result["captures"]:
            count("capture_period", period)
            count("capture_translation", translation)
            count("capture_maximum_diameter", diameter)
            count("capture_kind", "translation" if any(translation) else "trap")

    def report(self):
        return {"completed_execution_groups": self.groups, "weighted_originals": self.originals,
                "unique_group_counts": self.unique, "original_weighted_counts": self.weighted,
                "complete_witness_inventory": "lossless range containers, group map, and independent CaseResult reconstruction"}


def probe_request(opcode, **values):
    integer(opcode, "probe opcode", 1, 6)
    def state_blob(state):
        raw = state_wire(state)
        integer(len(raw), "probe state size", 1, 1 << 20)
        return blob(raw)
    if opcode in (1, 4, 5):
        body = state_blob(values["state"])
    elif opcode == 2:
        state, reference = values["state"], values["reference"]
        offset = integer(state.microtick - reference.microtick, "before/reference offset", 0, 303)
        if offset == 0 and values["previous_chain"] != bytes(32):
            raise ValueError("offset-zero chain must be zero")
        if offset == 0 and (state != reference or values["lift"] != values["reference_lift"]):
            raise ValueError("offset-zero state/lift must equal reference")
        body = state_blob(state) + state_blob(reference)
        body += lift_wire(state, values["lift"]) + lift_wire(reference, values["reference_lift"])
        body += controls_wire(values["controls"], state) + values["previous_chain"] + bytes([int(values.get("no_wrap", False))])
        if len(values["previous_chain"]) != 32 or type(values.get("no_wrap", False)) is not bool:
            raise ValueError("probe chain or no-wrap flag")
    elif opcode == 3:
        history, lifts = values["history"], values["lifts"]
        integer(len(history), "bounded capture history", 1, 77)
        if len(history) != len(lifts):
            raise ValueError("capture history/lift length")
        body = struct.pack("<H", len(history)) + b"".join(state_blob(s) + lift_wire(s, lift) for s, lift in zip(history, lifts))
    else:
        integer(len(values["payload"]), "compact probe payload", 13, 25)
        body = state_blob(values["begin"]) + struct.pack("<H", integer(values["offset"], "offset", 0, H)) + blob(values["payload"])
    integer(len(body), "probe request size", 1, PROBE_CAP)
    return opcode, body


def reference_probe(opcode, catalogue, **values):
    """Independent complete expected bytes for one accepted synthetic request."""
    S, I = _oracle()
    if opcode == 1:
        s = values["state"]
        lift = I.initial_lift(s)
        return {1: S.checkpoint(s), 2: repr(S.observe(s)).encode("ascii"), 3: graph_wire(s),
                4: lift_wire(s, lift), 5: repr(I.observe_lift(s, lift)).encode("ascii"), 6: pack_payload(s)}
    if opcode == 2:
        before, reference = values["state"], values["reference"]
        offset = integer(before.microtick - reference.microtick, "before/reference offset", 0, 303)
        if offset == 0 and values["previous_chain"] != bytes(32):
            raise ValueError("offset-zero chain must be zero")
        if offset == 0 and (before != reference or values["lift"] != values["reference_lift"]):
            raise ValueError("offset-zero state/lift must equal reference")
        if (before.L, before.origin_code, before.charge_frame) != (reference.L, reference.origin_code, reference.charge_frame):
            raise ValueError("probe reference context mismatch")
        after, events = S.step(before)
        controls = values["controls"]
        following = tuple(I.advance_control(c, catalogue) for c in controls)
        lift = I.advance_lift(before, after, events, values["lift"])
        if values.get("no_wrap", False):
            I.check_no_wrap(after, lift)
        observation = I.observe_step(before, after, events, controls, following, catalogue)
        ledger = I.observe_lift(after, lift, reference, values["reference_lift"])
        chain, representation = observation_chain(values["previous_chain"], after, events, observation, ledger)
        unions = (I.superpose_controls(*controls, catalogue), I.superpose_controls(*following, catalogue))
        isolated = b""
        if all(type(union) is S.SparseQ4State for union in unions):
            isolated = event_wire(I._merge_events(*(I.advance_control(c, catalogue, True)[1] for c in controls), unions[0]))
        flags = flags_byte(observation)
        return {1: state_wire(after), 2: event_wire(events), 3: controls_wire(following, after), 4: lift_wire(after, lift),
                5: S.checkpoint(after), 6: repr(observation).encode("ascii"), 7: repr(ledger).encode("ascii"),
                8: representation, 9: chain, 10: tick_wire(after.microtick - reference.microtick, after, lift, events, chain, flags),
                11: isolated, 12: keys_wire(observation.contact_outputs), 13: bytes([flags])}
    if opcode == 3:
        result = I.capture_certificate(values["history"], values["lifts"])
        return {1: repr(result).encode("ascii"), 2: capture_wire(result)}
    if opcode == 4:
        result = I.escape_certificate(values["state"], catalogue)
        return {1: repr(result).encode("ascii"), 2: escape_wire(values["state"], result), 3: repr(S.observe(values["state"])).encode("ascii")}
    if opcode == 5:
        packed = pack_payload(values["state"])
        return {1: packed, 2: state_wire(unpack_payload(packed, values["state"], 0))}
    if opcode == 6:
        restored = unpack_payload(values["payload"], values["begin"], values["offset"])
        return {1: state_wire(restored), 2: S.checkpoint(restored)}
    raise ValueError("unknown probe opcode")


PROBE_TAGS = {1: set(range(1, 7)), 2: set(range(1, 14)), 3: {1, 2}, 4: {1, 2, 3}, 5: {1, 2}, 6: {1, 2}}


def decode_probe_output(raw, opcodes):
    r, results = Reader(raw), []
    for opcode in opcodes:
        status = integer(r.one("B"), "probe status", 0, 1)
        payload = r.blob(1024 if status else PROBE_RESPONSE_CAP)
        if status:
            message = payload.decode("ascii")
            if not message or "\0" in message:
                raise ValueError("invalid rejection diagnostic")
            results.append((1, message))
            continue
        body, fields = Reader(payload), {}
        count = body.one("H")
        if count != len(PROBE_TAGS[opcode]):
            raise ValueError("probe field count")
        previous_tag = 0
        for _ in range(count):
            tag = body.one("B")
            if tag <= previous_tag or tag not in PROBE_TAGS[opcode]:
                raise ValueError("duplicate or extra probe tag")
            previous_tag = tag
            fields[tag] = body.blob(PROBE_RESPONSE_CAP)
        body.done()
        if set(fields) != PROBE_TAGS[opcode]:
            raise ValueError("missing probe tag")
        results.append((0, fields))
    r.done()
    return tuple(results)


def run_probe(binary, export, export_hash, requests, evidence, timeout=60):
    """One durable subprocess call; every failed call retains its raw streams."""
    requests = tuple(requests)
    integer(len(requests), "probe count", 1, 4096)
    path = Path(evidence).resolve()
    path.mkdir(parents=True, exist_ok=False)
    raw = PROBE_MAGIC + struct.pack("<I", len(requests))
    for opcode, body in requests:
        integer(opcode, "opcode", 1, 6)
        integer(len(body), "probe request size", 0, PROBE_CAP)
        raw += bytes([opcode]) + blob(body)
    write_new(path / "input.bin", raw)
    command = [str(Path(binary).resolve()), "--probe-stream", "--catalogue", str(Path(export).resolve()), "--catalogue-sha256", hash_text(export_hash)]
    if sha(export) != export_hash:
        raise ValueError("export changed before probe")
    record = {"schema": "q4-composite-probe-call-1", "command": command, "binary_sha256": sha(binary),
              "catalogue_sha256": export_hash, "input_sha256": hashlib.sha256(raw).hexdigest(), "registered_campaign": False}
    write_new(path / "lock.json", canonical(record))
    with (path / "stdout.bin").open("xb") as out, (path / "stderr.bin").open("xb") as err:
        try:
            proc = subprocess.run(command, input=raw, stdout=out, stderr=err, timeout=timeout, env=runtime_environment(binary), cwd=ROOT)
            record["exit_code"] = proc.returncode
        except BaseException as error:
            record.update(exit_code=None, failure=repr(error))
        finally:
            out.flush(); err.flush(); os.fsync(out.fileno()); os.fsync(err.fileno())
    record.update(stdout_sha256=sha(path / "stdout.bin"), stderr_sha256=sha(path / "stderr.bin"),
                  binary_unchanged=sha(binary) == record["binary_sha256"], export_unchanged=sha(export) == export_hash)
    write_new(path / "receipt.json", canonical(record))
    if record["exit_code"] != 0 or not record["binary_unchanged"] or not record["export_unchanged"]:
        raise ValueError("probe failed; retained " + str(path))
    return decode_probe_output((path / "stdout.bin").read_bytes(), [op for op, _ in requests])


def source_inventory():
    paths = {Path(__file__).resolve()}
    paths.update(ROOT / "scripts/tests/phi_v2_lattice" / name for name in ("test_composite_interaction_execution_v1.py", "test_sparse_credit_exchange_q4.py"))
    paths.update(ROOT / name for name in verify_import_closure())
    paths.update(ROOT / name for name in FROZEN)
    paths.update(NATIVE / name for name in ("sparse_q4_kernel_v1.hpp", "sparse_q4_kernel_v1.cpp", "composite_interaction_adapter_v1.cpp"))
    paths.update(ROOT / "engine/docs" / name for name in ("SPEC_STRICT_COMPOSITE_INTERACTION_EXECUTION_V1.md", "SPEC_STRICT_COMPOSITE_INTERACTION_EXECUTION_V2.md",
                 "SPEC_STRICT_COMPOSITE_ADAPTER_CLI_V1.md", "SPEC_STRICT_COMPOSITE_EXECUTION_METADATA_V1.md",
                 "SPEC_STRICT_COMPOSITE_EXECUTION_ACCOUNTING_V1.md", "SPEC_STRICT_COMPOSITE_EXECUTION_READINESS_V1.md",
                 "SPEC_STRICT_COMPOSITE_EXECUTION_MEMORY_V1.md"))
    for name, module in tuple(sys.modules.items()):
        source = getattr(module, "__file__", None)
        if name.startswith("phi_v2_lattice") and source and source.endswith(".py"):
            paths.add(Path(source).resolve())
    return {str(path): sha(path) for path in sorted(paths)}


def runtime_environment(binary=None, compiler=None):
    environment = dict(os.environ)
    if compiler is None and binary is not None:
        receipt = Path(binary).parent / "build.json"
        if receipt.is_file():
            compiler = parse_json(receipt.read_bytes()).get("compiler")
    if compiler:
        environment["PATH"] = str(Path(compiler).resolve().parent) + os.pathsep + environment.get("PATH", "")
    environment.update({key: "1" for key in ("OPENBLAS_NUM_THREADS", "OMP_NUM_THREADS", "MKL_NUM_THREADS")})
    return environment


def build_adapter(directory, checked=False):
    directory = Path(directory).resolve()
    directory.mkdir(parents=True, exist_ok=False)
    record = {"schema": "q4-composite-adapter-build-1", "status": "FAIL", "checked": bool(checked),
              "qualification": "BOUNDED_PROBES_ONLY", "campaign_build_ready": False,
              "registered_campaign": False, "created_utc": datetime.now(timezone.utc).isoformat()}
    try:
        compiler_name = shutil.which("g++")
        if not compiler_name:
            raise ValueError("accepted g++ compiler unavailable")
        compiler = Path(compiler_name).resolve()
        binary = directory / "composite-adapter.exe"
        sources = (NATIVE / "sparse_q4_kernel_v1.hpp", NATIVE / "sparse_q4_kernel_v1.cpp", NATIVE / "composite_interaction_adapter_v1.cpp")
        if any(not path.is_file() for path in sources):
            raise ValueError("complete owned adapter/kernel sources required")
        check_pins(FROZEN)
        before = source_inventory()
        command = [str(compiler), "-std=c++17", "-O0" if checked else "-O3", "-Wall", "-Wextra", "-Wpedantic", "-H"]
        if checked:
            command += ["-g", "-D_GLIBCXX_ASSERTIONS"]
        command += ["-I", str(NATIVE), str(sources[1]), str(sources[2]), "-o", str(binary)]
        environment = runtime_environment(compiler=compiler)
        overrides = ("CPATH", "CPLUS_INCLUDE_PATH", "C_INCLUDE_PATH", "COMPILER_PATH", "LIBRARY_PATH", "GCC_EXEC_PREFIX")
        for name in overrides:
            environment.pop(name, None)
        tools = {str(compiler): sha(compiler)}
        tool_versions = {}
        for name in ("ld", "cc1plus", "as"):
            raw = subprocess.run([str(compiler), "-print-prog-name=" + name], capture_output=True, check=True, env=environment).stdout.decode().strip()
            tool = Path(raw)
            if not tool.is_file():
                found = shutil.which(raw, path=environment["PATH"])
                if not found:
                    raise ValueError("actual tool unavailable: " + raw)
                tool = Path(found)
            tools[str(tool.resolve())] = sha(tool)
            tool_versions[str(tool.resolve())] = subprocess.run([str(tool.resolve()), "--version"], capture_output=True, check=True, env=environment).stdout.decode("ascii")
        record.update(command=command, cwd=str(ROOT), compiler=str(compiler), compiler_sha256=sha(compiler),
                      compiler_version=subprocess.run([str(compiler), "--version"], capture_output=True, check=True, env=environment).stdout.decode("ascii"),
                      tool_sha256=tools, pre_tool_sha256=tools, tool_version=tool_versions, source_sha256=before, cleared_build_overrides=list(overrides), binary=str(binary))
        write_new(directory / "build-lock.json", canonical(record))
        with (directory / "stdout").open("xb") as out, (directory / "stderr").open("xb") as err:
            try:
                process = subprocess.run(command, stdout=out, stderr=err, cwd=ROOT, env=environment, timeout=120)
                record["exit_code"] = process.returncode
            finally:
                out.flush(); err.flush(); os.fsync(out.fileno()); os.fsync(err.fileno())
        includes = {}
        for line in (directory / "stderr").read_text(errors="replace").splitlines():
            match = re.match(r"^\.+ (.+)$", line)
            if match:
                path = Path(match[1]).resolve()
                includes[str(path)] = sha(path)
        runtime = {}
        for name in ("libstdc++-6.dll", "libgcc_s_seh-1.dll", "libwinpthread-1.dll"):
            path = compiler.parent / name
            if not path.is_file():
                raise ValueError("required compiler runtime unavailable: " + name)
            runtime[str(path)] = sha(path)
        record.update(include_sha256=includes, compiler_runtime_sha256=runtime, post_tool_sha256={name: sha(name) for name in tools},
                      post_source_sha256={name: sha(name) for name in before},
                      binary_sha256=sha(binary) if binary.is_file() else None,
                      stdout_sha256=sha(directory / "stdout"), stderr_sha256=sha(directory / "stderr"))
        if record["exit_code"] != 0 or record["post_source_sha256"] != before or record["post_tool_sha256"] != tools:
            raise ValueError("build failed or source drift during build")
        record["status"] = "PASS"
    except BaseException as error:
        record["failure"] = repr(error)
    write_new(directory / "build.json", canonical(record))
    if record["status"] != "PASS":
        raise ValueError("adapter build failed; retained " + str(directory))
    return Path(record["binary"])


def verify_build(binary, receipt_path=None, receipt_sha256=None, *, production=False):
    binary = Path(binary).resolve()
    receipt = Path(receipt_path).resolve() if receipt_path else binary.parent / "build.json"
    row = parse_json(read_artifact({"path": str(receipt), "sha256": receipt_sha256 or sha(receipt)}, 4 << 20))
    schema = row.get("schema")
    if production and schema != "strict-composite-native-build-1":
        raise ValueError("production requires the complete strict-composite-native-build-1 receipt; convenience builds are bounded-only")
    statuses = {"q4-composite-adapter-build-1": "PASS", "strict-composite-native-build-1": "PASS_BUILD"}
    if schema not in statuses or row.get("status") != statuses[schema] or row.get("binary") != str(binary):
        raise ValueError("wrong or failed adapter build receipt")
    if sha(binary) != row.get("binary_sha256"):
        raise ValueError("adapter binary identity changed")
    source_key = "snapshot_sha256" if schema == "strict-composite-native-build-1" else "source_sha256"
    for key in (source_key, "tool_sha256", "include_sha256", "compiler_runtime_sha256"):
        if type(row.get(key)) is not dict or not row[key]:
            raise ValueError("missing build provenance: " + key)
        check_pins(row[key])
    if row.get("post_source_sha256") != row["source_sha256"] or sha(row["compiler"]) != row["compiler_sha256"]:
        raise ValueError("incomplete build source/compiler identity")
    if source_key == "snapshot_sha256":
        if {Path(name).name: digest for name, digest in row[source_key].items()} != {Path(name).name: digest for name, digest in row["source_sha256"].items()}:
            raise ValueError("frozen source copies differ from build inputs")
    if row.get("post_tool_sha256") != row["tool_sha256"] or row.get("pre_tool_sha256") != row["tool_sha256"]:
        raise ValueError("build tool identities changed or missing pre/post checks")
    if schema == "strict-composite-native-build-1":
        roles = {"as", "cc1plus", "collect2", "driver", "ld"}
        if (type(row.get("exit_code")) is not int or row["exit_code"] != 0
                or type(row.get("tool_paths")) is not dict or set(row["tool_paths"]) != roles
                or type(row.get("tool_version")) is not dict or set(row["tool_version"]) != roles):
            raise ValueError("incomplete build tool-role provenance")
        tool_paths = row["tool_paths"]
        if (len(set(tool_paths.values())) != 5 or set(row["tool_sha256"]) != set(tool_paths.values())
                or tool_paths["driver"] != row["compiler"]
                or row["tool_sha256"][tool_paths["driver"]] != row["compiler_sha256"]):
            raise ValueError("build tool roles do not identify all five actual tools")
        for version in row["tool_version"].values():
            if (type(version) is not dict or set(version) != {"exit_code", "stdout", "stderr"}
                    or type(version["exit_code"]) is not int or version["exit_code"] != 0
                    or type(version["stdout"]) is not str or type(version["stderr"]) is not str):
                raise ValueError("incomplete actual tool version result")
        expected_sources = {"sparse_q4_kernel_v1.hpp", "sparse_q4_kernel_v1.cpp", "composite_interaction_adapter_v1.cpp"}
        if (len(row["source_sha256"]) != 3 or len(row["snapshot_sha256"]) != 3
                or {Path(name).name for name in row["source_sha256"]} != expected_sources):
            raise ValueError("incomplete exact native source inventory")
        command = row.get("command")
        if (type(command) is not list or not command or command[0] != row["compiler"]
                or any(type(arg) is not str for arg in command) or "-v" not in command or "-H" not in command
                or any(name not in command for name in row["snapshot_sha256"] if name.endswith(".cpp"))):
            raise ValueError("build command omits actual verbose invocation or frozen inputs")
        required_runtime = {"libstdc++-6.dll", "libgcc_s_seh-1.dll", "libwinpthread-1.dll"}
        if (len(row["compiler_runtime_sha256"]) != 3
                or {Path(name).name for name in row["compiler_runtime_sha256"]} != required_runtime):
            raise ValueError("incomplete native runtime inventory")
        # Consume the same owned bytes whose digest is admitted. The include
        # listing independently prevents a receipt from silently omitting a header.
        read_artifact({"path": str(binary.parent / "stdout"), "sha256": row["stdout_sha256"]}, 8 << 20)
        stderr = read_artifact({"path": str(binary.parent / "stderr"), "sha256": row["stderr_sha256"]}, 8 << 20)
        included = {str(Path(match[1]).resolve()) for line in stderr.decode("utf-8", errors="strict").splitlines()
                    if (match := re.match(r"^\.+ (.+)$", line))}
        if included != set(row["include_sha256"]):
            raise ValueError("build include inventory differs from the retained complete -H log")
    return row


def verified_supervisor():
    name = "scripts/phi_v2_lattice/experiments/certify_full_memory.py"
    closure = verify_import_closure()
    if str(ROOT / "scripts") not in sys.path:
        sys.path.insert(0, str(ROOT / "scripts"))
    module = importlib.import_module("phi_v2_lattice.experiments.certify_full_memory")
    check_pins({name: FROZEN[name]})
    verify_loaded_imports(closure)
    return module


def bootstrap_environment(prefix):
    environment = dict(os.environ)
    removed = sorted(name for name in environment if name.startswith("PYTHON"))
    for name in removed:
        environment.pop(name)
    environment.update(PYTHONHASHSEED="0", PYTHONDONTWRITEBYTECODE="1", PYTHONPYCACHEPREFIX=str(Path(prefix).resolve()))
    environment.update({key: "1" for key in ("OPENBLAS_NUM_THREADS", "OMP_NUM_THREADS", "MKL_NUM_THREADS")})
    return environment, removed


def check_entry_cache(prefix):
    prefix = Path(prefix).resolve()
    if (sys.pycache_prefix != str(prefix) or not sys.dont_write_bytecode or not prefix.is_dir()
            or any(prefix.iterdir()) or sys.flags.optimize or sys.flags.inspect
            or os.environ.get("PYTHONHASHSEED") != "0"
            or any(name in os.environ for name in ("PYTHONPATH", "PYTHONHOME", "PYTHONSTARTUP", "PYTHONOPTIMIZE", "PYTHONINSPECT"))):
        raise ValueError("use the accepted empty-prefix composite bootstrap")
    return prefix


def bootstrap_guard(directory):
    """Exercise this real absolute-file entry before any candidate import."""
    directory = Path(directory).resolve()
    directory.mkdir(parents=True, exist_ok=False)
    prefix = directory / "empty-cache"
    prefix.mkdir(exist_ok=False)
    environment, removed = bootstrap_environment(prefix)
    command = [sys.executable, "-B", "-X", "pycache_prefix=" + str(prefix), str(Path(__file__).resolve()), "--guard-entry", str(prefix), "--entry-sha256", sha(__file__)]
    receipt = {"schema": "q4-composite-bootstrap-1", "argv": command, "removed_environment_names": removed,
               "effective_overrides": {key: environment[key] for key in ("PYTHONHASHSEED", "PYTHONDONTWRITEBYTECODE", "PYTHONPYCACHEPREFIX")},
               "interpreter": sys.executable, "interpreter_sha256": sha(sys.executable), "python_version": sys.version,
               "entry_sha256": sha(__file__), "registered_campaign": False}
    write_new(directory / "lock.json", canonical(receipt))
    with (directory / "stdout").open("xb") as out, (directory / "stderr").open("xb") as err:
        try:
            proc = subprocess.run(command, env=environment, cwd=ROOT, stdout=out, stderr=err, timeout=30)
            receipt["exit_code"] = proc.returncode
        except BaseException as error:
            receipt.update(exit_code=None, failure=repr(error))
        finally:
            out.flush(); err.flush(); os.fsync(out.fileno()); os.fsync(err.fileno())
    receipt.update(stdout_sha256=sha(directory / "stdout"), stderr_sha256=sha(directory / "stderr"),
                   cache_empty=not any(prefix.iterdir()), entry_unchanged=sha(__file__) == receipt["entry_sha256"])
    write_new(directory / "receipt.json", canonical(receipt))
    if receipt["exit_code"] != 0 or not receipt["cache_empty"] or not receipt["entry_unchanged"]:
        raise ValueError("actual composite bootstrap failed; retained " + str(directory))
    return receipt


def reservation_bytes(remaining_tiers, copies=0):
    remaining_tiers = tuple(remaining_tiers)
    if len(set(remaining_tiers)) != len(remaining_tiers) or any(tier not in TIER_COUNTS for tier in remaining_tiers):
        raise ValueError("distinct registered remaining tiers required")
    integer(copies, "additional copy bytes")
    return sum(TIER_COUNTS[tier] * 101376 + (2 << 30) for tier in remaining_tiers) + (2 << 30) + copies


def disk_admission(output, remaining_tiers, copies=0):
    import ctypes
    from ctypes import wintypes as W
    path = Path(output).resolve()
    while not path.exists():
        path = path.parent
    if os.name != "nt":
        raise ValueError("registered Windows filesystem required")
    kernel = ctypes.WinDLL("kernel32", use_last_error=True)
    volume = ctypes.create_unicode_buffer(32768)
    kernel.GetVolumePathNameW.argtypes = (W.LPCWSTR, W.LPWSTR, W.DWORD)
    if not kernel.GetVolumePathNameW(str(path), volume, len(volume)):
        raise ctypes.WinError(ctypes.get_last_error())
    sectors, per_sector, available, total = (W.DWORD() for _ in range(4))
    kernel.GetDiskFreeSpaceW.argtypes = (W.LPCWSTR, ctypes.POINTER(W.DWORD), ctypes.POINTER(W.DWORD), ctypes.POINTER(W.DWORD), ctypes.POINTER(W.DWORD))
    if not kernel.GetDiskFreeSpaceW(volume.value, ctypes.byref(sectors), ctypes.byref(per_sector), ctypes.byref(available), ctypes.byref(total)):
        raise ctypes.WinError(ctypes.get_last_error())
    unit = sectors.value * per_sector.value
    usage = shutil.disk_usage(path)
    required = reservation_bytes(remaining_tiers, copies)
    return {"schema": "q4-composite-storage-admission-2", "path": str(Path(output).resolve()), "volume": volume.value,
            "created_utc": datetime.now(timezone.utc).isoformat(), "allocation_unit": unit, "free_bytes": usage.free,
            "total_bytes": usage.total, "required_bytes": required, "remaining_tiers": list(remaining_tiers),
            "additional_copies_bytes": copies, "spare_bytes": 2 << 30,
            "status": "PASS" if 0 < unit <= 4096 and usage.free >= required else "INCOMPLETE_RESOURCE"}


class AttemptBudget:
    """Deadline and retained-volume checks shared by preparation and reduction."""
    def __init__(self, directory, deadline, byte_limit, file_limit=8192, spare_bytes=2 << 30):
        self.directory = Path(directory).resolve()
        self.deadline = deadline
        self.byte_limit = integer(byte_limit, "attempt byte quota", 1)
        self.file_limit = integer(file_limit, "attempt file quota", 1, 8192)
        self.spare_bytes = integer(spare_bytes, "live spare")
        self.last_check = 0.0
        self.peak_bytes = self.peak_files = 0

    def check(self, force=False):
        now = time.monotonic()
        if now >= self.deadline:
            raise TimeoutError("global monotonic deadline")
        if not force and now - self.last_check < 0.2:
            return
        self.last_check = now
        allocated = count = 0
        for base, directories, files in os.walk(self.directory, followlinks=False):
            for name in directories + files:
                path = Path(base) / name
                if path.is_symlink() or not path.resolve().is_relative_to(self.directory):
                    raise ValueError("attempt resource path escapes owned directory")
            for name in files:
                size = (Path(base) / name).stat().st_size
                allocated += (size + 4095) // 4096 * 4096
                count += 1
        self.peak_bytes = max(self.peak_bytes, allocated)
        self.peak_files = max(self.peak_files, count)
        if allocated > self.byte_limit or count > self.file_limit:
            raise OSError("retained attempt allocation/file quota")
        if shutil.disk_usage(self.directory).free < self.spare_bytes:
            raise OSError("mandatory live 2-GiB spare")
        if time.monotonic() >= self.deadline:
            raise TimeoutError("deadline during retained resource accounting")


class BoundedLogs:
    """Retain each log through its one crossing chunk, then signal failure."""
    def __init__(self, process, directory, limit=8 << 20):
        self.directory = Path(directory)
        self.limit = integer(limit, "stream log quota", 1, 8 << 20)
        self.failures, self.sizes, self.threads = [], {}, []
        for name in ("stdout", "stderr"):
            source = getattr(process, name)
            target = self.directory / name
            thread = threading.Thread(target=self._copy, args=(name, source, target), daemon=True)
            self.threads.append(thread)
            thread.start()

    def _copy(self, name, source, target):
        count = 0
        try:
            with target.open("xb") as stream:
                while True:
                    raw = source.read(4096)
                    if not raw:
                        break
                    stream.write(raw)
                    count += len(raw)
                    if count > self.limit:
                        self.failures.append(name + " retained log quota")
                        break
                stream.flush()
                os.fsync(stream.fileno())
        except BaseException as error:
            self.failures.append(name + ": " + repr(error))
        finally:
            self.sizes[name] = count
            source.close()

    def check(self):
        if self.failures:
            raise OSError("; ".join(self.failures))

    def finish(self, deadline):
        for thread in self.threads:
            thread.join(timeout=max(0, min(1, deadline - time.monotonic())))
        if any(thread.is_alive() for thread in self.threads):
            raise TimeoutError("log drain did not finish within deadline")
        self.check()


def monitor_supervisor(process, logs, deadline):
    """Bound both scientific supervision and separately timed metadata cleanup."""
    peak = _private_memory()
    failure = None
    try:
        while process.poll() is None:
            peak = max(peak, _private_memory())
            if peak > OUTER_MONITOR_CAP:
                raise MemoryError("outer metadata monitor exceeded its OS memory reservation")
            if time.monotonic() >= deadline:
                raise TimeoutError("outer scientific-plus-metadata watchdog")
            logs.check()
            time.sleep(0.02)
        logs.finish(deadline)
    except BaseException as error:
        failure = repr(error)
        if process.poll() is None:
            process.kill()
        process.wait(timeout=5)
        try:
            logs.finish(time.monotonic() + 2)
        except (OSError, TimeoutError) as cleanup_error:
            failure += "; bounded log cleanup: " + repr(cleanup_error)
    return {"outer_failure": failure, "exit_code": process.returncode, "outer_monitor_peak_private_bytes": peak}


@lru_cache(maxsize=1)
def process_limits():
    """Load the reviewed shared helper from the exact hashed source buffer."""
    name = "scripts/phi_v2_lattice/recovery_process_limits.py"
    path = ROOT / name
    with path.open("rb") as stream:
        raw = stream.read((1 << 20) + 1)
    if len(raw) > 1 << 20 or hashlib.sha256(raw).hexdigest() != FROZEN[name]:
        raise ValueError("shared process-limit source changed before load")
    spec = importlib.util.spec_from_file_location("_ftd_verified_process_limits", path)
    module = importlib.util.module_from_spec(spec)
    exec(compile(raw, str(path), "exec"), module.__dict__)
    if (module.OUTER_MONITOR_CAP, module.SUPERVISOR_PARENT_CAP, module.DESCENDANT_ALLOWANCE) != (OUTER_MONITOR_CAP, SUPERVISOR_PARENT_CAP, DESCENDANT_ALLOWANCE):
        raise ValueError("shared process-budget constants differ from admission")
    sys.modules[spec.name] = module
    return module


def _pid_alive(pid):
    return process_limits().pid_alive(pid)


def _private_memory():
    return process_limits().private_memory()


def MetadataProcessJob(cap=None, cleanup=False):
    return process_limits().MetadataProcessJob(cap, cleanup)


def _outer_control_entry(directory, prefix):
    check_entry_cache(prefix)
    directory = Path(directory)
    cleanup_job = MetadataProcessJob(cleanup=True)
    job = MetadataProcessJob(OUTER_MONITOR_CAP)
    allocation_rejected = False
    try:
        oversized = bytearray(160 << 20)
        del oversized
    except MemoryError:
        allocation_rejected = True
    if not allocation_rejected:
        raise ValueError("outer OS process-memory cap was not enforced")
    command = [sys.executable, "-B", "-X", "pycache_prefix=" + str(prefix), str(Path(__file__).resolve()),
               "--control-descendant", "outer-breakaway", "--entry-sha256", sha(__file__)]
    with (directory / "child.stdout").open("xb") as out, (directory / "child.stderr").open("xb") as err:
        process = subprocess.Popen(command, cwd=ROOT, stdout=out, stderr=err)
        inside = job.contains(process.pid)
        contained_for_cleanup = cleanup_job.contains(process.pid)
        code = process.wait(timeout=5)
    record = {"schema": "q4-composite-outer-monitor-control-1", "os_process_cap_bytes": OUTER_MONITOR_CAP,
              "actual_outer_assignment": job.contains(os.getpid()), "outer_oversized_allocation_rejected": allocation_rejected,
              "child_in_outer_job": inside, "child_exit": code, "outer_peak_private_bytes": job.peak_memory(),
              "child_alive_after_cleanup": _pid_alive(process.pid), "child_in_cleanup_job": contained_for_cleanup, "registered_preparations": False}
    write_new(directory / "outer-control.json", canonical(record))
    if inside or code or record["child_alive_after_cleanup"] or not contained_for_cleanup:
        raise ValueError("scientific supervisor child did not break away from outer cap")


def _supervisor_control_target(payload, barrier):
    """Synthetic process/resource work only, through the actual frozen Job."""
    barrier.wait()
    directory = Path(payload["directory"])
    check_entry_cache(payload["prefix"])
    if sha(__file__) != payload["entry_sha256"]:
        raise ValueError("control source changed before assigned work")
    write_new(directory / "assigned-entry.json", canonical({"pid": os.getpid(), "after_assignment_barrier": True,
              "entry_sha256": sha(__file__), "registered_preparations": False}))
    mode = payload["kind"]
    command = [sys.executable, "-B", "-X", "pycache_prefix=" + payload["prefix"], str(Path(__file__).resolve()),
               "--control-descendant", mode, "--entry-sha256", payload["entry_sha256"]]
    process = subprocess.Popen(command, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    write_new(directory / "descendant.json", canonical({"pid": process.pid, "argv": command}))
    logs = BoundedLogs(process, directory, 8192 if mode == "log" else 8 << 20)
    budget = AttemptBudget(directory, payload["deadline"], 64 << 20)
    try:
        if mode == "quota":
            budget.byte_limit = 1
            budget.check(True)
        elif mode == "memory":
            retained = []
            while True:
                retained.append(bytearray(16 << 20))
                budget.check()
        while process.poll() is None:
            budget.check()
            logs.check()
            time.sleep(0.01)
        logs.finish(payload["deadline"])
        if process.returncode:
            raise RuntimeError("bounded descendant nonzero exit")
        budget.check(True)
        write_new(directory / "control-complete.json", canonical({"status": "COMPLETE_CONTROL", "kind": mode,
                  "peak_allocated_bytes": budget.peak_bytes, "registered_preparations": False}))
    except BaseException as error:
        write_new(directory / "control-failure.json", canonical({"kind": mode, "error": repr(error), "registered_preparations": False}))
        raise


def _run_supervisor_control_entry(directory, kind, prefix):
    check_entry_cache(prefix)
    cleanup_job = MetadataProcessJob(cleanup=True)
    parent_job = MetadataProcessJob(SUPERVISOR_PARENT_CAP)
    module = verified_supervisor()
    directory = Path(directory).resolve()
    write_new(directory / "parent-os-binding.json", canonical({"pid": os.getpid(), "process_cap_bytes": SUPERVISOR_PARENT_CAP,
              "process_cap_installed": parent_job.contains(os.getpid()), "cleanup_job_installed": cleanup_job.contains(os.getpid())}))
    before = source_inventory()
    record = module.supervise(_supervisor_control_target,
                              {"directory": str(directory), "kind": kind, "prefix": str(prefix), "entry_sha256": sha(__file__)},
                              2 if kind == "deadline" else 12, 256 << 20)
    descendant = directory / "descendant.json"
    record.update(kind=kind, source_sha256=before, source_unchanged=before == source_inventory(), registered_preparations=False,
                  descendant_alive_after_cleanup=_pid_alive(parse_json(descendant.read_bytes())["pid"]) if descendant.exists() else None,
                  supervisor_parent_os_cap_bytes=SUPERVISOR_PARENT_CAP, supervisor_parent_peak_private_bytes=parent_job.peak_memory(),
                  actual_supervisor_parent_os_assignment=parent_job.contains(os.getpid()),
                  actual_parent_cleanup_assignment=cleanup_job.contains(os.getpid()))
    write_new(directory / "supervisor.json", canonical(record))
    if (not record["actual_job_assignment"] or record["coordinator_alive_after_cleanup"] or record["descendant_alive_after_cleanup"] is not False
            or not record["source_unchanged"]):
        raise ValueError("actual supervisor assignment/cleanup/source control failed")
    if kind == "success" and (record["exit_code"] or record["failure"] or not (directory / "control-complete.json").exists()):
        raise ValueError("successful bounded supervisor control failed")
    if kind != "success" and record["exit_code"] == 0 and record["failure"] is None:
        raise ValueError("negative resource control did not fail closed")
    return record


def _parent_loss_control_entry(directory, kind, prefix):
    """Actual metadata-parent loss across the same production Job hierarchy."""
    check_entry_cache(prefix)
    directory = Path(directory).resolve()
    cleanup_job = MetadataProcessJob(cleanup=True)
    memory_job = MetadataProcessJob(OUTER_MONITOR_CAP)
    inner = directory / "inner"
    inner.mkdir()
    command = [sys.executable, "-B", "-X", "pycache_prefix=" + str(prefix), str(Path(__file__).resolve()), "--control-entry", str(inner),
               "--control-kind", "deadline", "--entry-sha256", sha(__file__)]
    log_directory = directory / "inner-logs"
    log_directory.mkdir()
    process = subprocess.Popen(command, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    logs = BoundedLogs(process, log_directory)
    try:
        deadline = time.monotonic() + 8
        descendant = inner / "descendant.json"
        while not descendant.is_file():
            if process.poll() is not None or time.monotonic() >= deadline:
                raise ValueError("parent-loss fixture failed before assigned descendant")
            time.sleep(0.01)
        # Exclusive record writes are fsynced; a concurrent read may still
        # catch the initial creation, so retry JSON until the bounded deadline.
        while True:
            try:
                grandchild = parse_json(descendant.read_bytes())["pid"]
                coordinator = parse_json((inner / "assigned-entry.json").read_bytes())["pid"]
                break
            except (ValueError, OSError):
                if time.monotonic() >= deadline:
                    raise
                time.sleep(0.01)
        proof = {"schema": "q4-composite-parent-loss-ready-1", "kind": kind, "outer_pid": os.getpid(), "supervisor_pid": process.pid,
                 "coordinator_pid": coordinator, "descendant_pid": grandchild, "supervisor_in_outer_cleanup": cleanup_job.contains(process.pid),
                 "supervisor_in_outer_memory_job": memory_job.contains(process.pid), "entry_sha256": sha(__file__), "registered_preparations": False}
        write_new(directory / "loss-ready.json", canonical(proof))
        if not proof["supervisor_in_outer_cleanup"] or proof["supervisor_in_outer_memory_job"]:
            raise ValueError("loss fixture hierarchy is not the production hierarchy")
        if kind == "outer_loss":
            os._exit(97)
        if kind == "outer_timeout":
            monitored = monitor_supervisor(process, logs, time.monotonic() + 0.05)
            write_new(directory / "outer-watchdog.json", canonical(monitored))
            if not monitored["outer_failure"] or "watchdog" not in monitored["outer_failure"]:
                raise ValueError("actual outer watchdog did not terminate its supervised tree")
        else:
            process.kill()
            process.wait(timeout=5)
            logs.finish(time.monotonic() + 2)
        while any(_pid_alive(pid) for pid in (coordinator, grandchild)) and time.monotonic() < deadline:
            time.sleep(0.01)
        if any(_pid_alive(pid) for pid in (coordinator, grandchild)):
            raise ValueError("supervisor loss left a scientific descendant alive")
    finally:
        if process.poll() is None:
            process.kill()
            process.wait(timeout=5)


def supervisor_control(directory, kind):
    if kind not in ("success", "deadline", "memory", "quota", "log", "outer_memory", "outer_loss", "supervisor_loss", "outer_timeout"):
        raise ValueError("known bounded supervisor control required")
    directory = Path(directory).resolve()
    directory.mkdir(parents=True, exist_ok=False)
    prefix = directory / "empty-cache"
    prefix.mkdir()
    environment, removed = bootstrap_environment(prefix)
    command = [sys.executable, "-B", "-X", "pycache_prefix=" + str(prefix), str(Path(__file__).resolve()),
               "--control-entry", str(directory), "--control-kind", kind, "--entry-sha256", sha(__file__)]
    write_new(directory / "bootstrap.json", canonical({"argv": command, "removed_environment_names": removed,
              "entry_sha256": sha(__file__), "interpreter_sha256": sha(sys.executable), "registered_preparations": False}))
    with (directory / "entry.stdout").open("xb") as out, (directory / "entry.stderr").open("xb") as err:
        process = subprocess.run(command, env=environment, cwd=ROOT, stdout=out, stderr=err, timeout=30)
    if kind in ("outer_loss", "supervisor_loss", "outer_timeout"):
        expected = 97 if kind == "outer_loss" else 0
        ready = parse_json((directory / "loss-ready.json").read_bytes())
        pids = tuple(ready[name] for name in ("supervisor_pid", "coordinator_pid", "descendant_pid"))
        deadline = time.monotonic() + 2
        while any(_pid_alive(pid) for pid in pids) and time.monotonic() < deadline:
            time.sleep(0.01)
        alive = [pid for pid in pids if _pid_alive(pid)]
        result = {"schema": "q4-composite-parent-loss-control-1", "kind": kind, "outer_exit": process.returncode,
                  "surviving_descendants": alive, "ready": ready, "registered_preparations": False}
        write_new(directory / "loss-control.json", canonical(result))
        if process.returncode != expected or alive:
            raise ValueError("actual metadata-parent loss cleanup failed; retained " + str(directory))
        return result
    if process.returncode:
        raise ValueError("actual supervisor control failed; retained " + str(directory))
    return parse_json((directory / ("outer-control.json" if kind == "outer_memory" else "supervisor.json")).read_bytes())


def validate_descriptor(raw, check_artifacts=True):
    row = metadata_json(raw, DESCRIPTOR_KEYS)
    if row["schema"] != "q4-composite-input-descriptor-2" or row["tier"] not in TIER_COUNTS:
        raise ValueError("descriptor schema/tier")
    for key, expected in {"N": TIER_COUNTS[row["tier"]], "H": 304, "L": 64, "range_count": 64, "workers": 32,
                          "deadline_seconds": 1800, "memory_bytes": 8 << 30}.items():
        if integer(row[key], key) != expected:
            raise ValueError("descriptor registered parameter mismatch")
    if type(row["source_sha256"]) is not dict or not row["source_sha256"]:
        raise ValueError("complete source identities required")
    for path, digest in row["source_sha256"].items():
        absolute_path(path)
        hash_text(digest)
    for name in ("catalogue", "actions", "templates", "build", "readiness", "preservation"):
        artifact = row[name]
        if type(artifact) is not dict or set(artifact) != ({"path", "sha256", "pins"} if name == "catalogue" else {"path", "sha256"}):
            raise ValueError("artifact descriptor shape")
        absolute_path(artifact["path"])
        hash_text(artifact["sha256"])
        if check_artifacts and sha(artifact["path"]) != artifact["sha256"]:
            raise ValueError("descriptor artifact changed")
    if type(row["catalogue"]["pins"]) is not dict or set(row["catalogue"]["pins"]) != {"npz", "report", "audit", "q2_source", "dense_source"}:
        raise ValueError("catalogue source pins")
    for digest in row["catalogue"]["pins"].values():
        hash_text(digest)
    if check_artifacts:
        check_pins(row["source_sha256"])
    return row


def validate_production_readiness(raw, source_sha256, build_sha256):
    keys = {"schema", "verdict", "campaign_ready", "canonical_adoption", "source_sha256", "build_sha256", "gates", "integrated_suite"}
    row = metadata_json(raw, keys)
    if (row["schema"] != "q4-composite-execution-readiness-2" or row["verdict"] != "PASS_SCOPED_EXACT_EXECUTION"
            or row["campaign_ready"] is not True or row["canonical_adoption"] is not False
            or row["source_sha256"] != source_sha256 or row["build_sha256"] != build_sha256):
        raise ValueError("independent production readiness does not bind this exact source/build")
    if type(row["gates"]) is not dict or set(row["gates"]) != set(PRODUCTION_GATES) or any(value != "PASS" for value in row["gates"].values()):
        raise ValueError("incomplete independent production gates")
    suite = row["integrated_suite"]
    if type(suite) is not dict or set(suite) != {"file_count", "failed", "errors", "skipped", "deselected", "receipt"}:
        raise ValueError("complete integrated-suite evidence required")
    if integer(suite["file_count"], "integrated file count") != 19 or any(integer(suite[name], name) for name in ("failed", "errors", "skipped", "deselected")):
        raise ValueError("integrated suite has missing or failed coverage")
    artifact(suite["receipt"])
    return row


def validate_lock(raw, lock_path, check_artifacts=True, require_ready=True):
    row = metadata_json(raw, LOCK_KEYS)
    if row["schema"] != "q4-composite-execution-lock-2" or row["tier"] not in TIER_COUNTS:
        raise ValueError("execution lock schema/tier")
    n = TIER_COUNTS[row["tier"]]
    for key, expected in {"N": n, "H": 304, "L": 64, "range_count": 64, "workers": 32, "deadline_seconds": 1800, "memory_bytes": 8 << 30}.items():
        if integer(row[key], key) != expected:
            raise ValueError("execution lock changes registered parameter")
    absolute_path(row["output"])
    if Path(row["output"]).resolve() != Path(lock_path).resolve().parent:
        raise ValueError("lock must be inside its exclusive attempt output")
    if type(row["source_sha256"]) is not dict or not row["source_sha256"]:
        raise ValueError("complete direct/transitive source map required")
    for path, digest in row["source_sha256"].items():
        absolute_path(path)
        hash_text(digest)
    for name in ("catalogue_npz", "build", "readiness", "preservation"):
        artifact(row[name], check_artifacts)
    pins = row["catalogue_pins"]
    if type(pins) is not dict or set(pins) != {"npz", "report", "audit", "q2_source", "dense_source"}:
        raise ValueError("lock catalogue pin inventory")
    for value in pins.values():
        hash_text(value)
    if pins["npz"] != row["catalogue_npz"]["sha256"]:
        raise ValueError("lock catalogue identity mismatch")
    specs = row["specifications"]
    if type(specs) is not dict or set(specs) != {"execution_v1", "execution_v2", "adapter_cli", "kernel"}:
        raise ValueError("complete frozen specification identity set required")
    for value in specs.values():
        artifact(value, check_artifacts)
    storage = row["storage"]
    if type(storage) is not dict or set(storage) != {"admission", "remaining_tiers", "additional_copy_bytes"}:
        raise ValueError("storage admission shape")
    artifact(storage["admission"], check_artifacts)
    reservation_bytes(storage["remaining_tiers"], storage["additional_copy_bytes"])
    if row["tier"] not in storage["remaining_tiers"]:
        raise ValueError("current tier omitted from remaining reserve")
    sorting = {"generations": 3, "fanin": 64, "record_cap": 2048, "chunk_records": (n + 63) // 64,
               "original_stream": "q4-original-checkpoint-ordered-controls-1", "normalized_stream": "q4-normalized-checkpoint-1"}
    if canonical(row["sorting"]) != canonical(sorting):
        raise ValueError("sorting contract changed")
    if check_artifacts:
        check_pins(row["source_sha256"])
        if require_ready:
            validate_production_readiness(read_artifact(row["readiness"], 65536), row["source_sha256"], row["build"]["sha256"])
    elif require_ready:
        raise ValueError("production admission cannot omit artifact checks")
    return row


def generated_admission(lock_path, lock_sha256, descriptor_path, descriptor_sha256, map_path, map_sha256, catalogue_path, catalogue_sha256, verify=True):
    row = {"schema": "q4-composite-generated-admission-1",
           "lock": {"path": str(Path(lock_path).resolve()), "sha256": hash_text(lock_sha256)},
           "descriptor": {"path": str(Path(descriptor_path).resolve()), "sha256": hash_text(descriptor_sha256)},
           "map": {"path": str(Path(map_path).resolve()), "sha256": hash_text(map_sha256)},
           "catalogue": {"path": str(Path(catalogue_path).resolve()), "sha256": hash_text(catalogue_sha256)}}
    for value in row.values():
        if isinstance(value, dict):
            artifact(value, verify)
    descriptor_raw = read_artifact(row["descriptor"], 65536) if verify else Path(descriptor_path).read_bytes()
    descriptor = validate_descriptor(descriptor_raw, verify)
    lock_raw = read_artifact(row["lock"], 65536) if verify else Path(lock_path).read_bytes()
    lock = validate_lock(lock_raw, lock_path, verify, False)
    common = ("tier", "N", "H", "L", "range_count", "workers", "deadline_seconds", "memory_bytes", "source_sha256", "build", "readiness", "preservation")
    if any(canonical(lock[name]) != canonical(descriptor[name]) for name in common) or lock["catalogue_pins"] != descriptor["catalogue"]["pins"]:
        raise ValueError("descriptor differs from initial locked inputs/parameters")
    if {key: descriptor["catalogue"][key] for key in ("path", "sha256")} != row["catalogue"]:
        raise ValueError("CLI catalogue differs from descriptor")
    row.update(actions=descriptor["actions"], templates=descriptor["templates"])
    raw = canonical(row)
    metadata_json(raw, {"schema", "lock", "descriptor", "map", "catalogue", "actions", "templates"})
    return raw


def verify_generated_admission(lock_path, **arguments):
    path = Path(lock_path).resolve().parent / "admission.json"
    raw = path.read_bytes()
    expected = generated_admission(lock_path, **arguments)
    if raw != expected:
        raise ValueError("generated admission differs from CLI-bound deterministic recipe")
    return hashlib.sha256(raw).hexdigest()


def _production_coordinator(payload, barrier):
    """Authorized only through a reviewed initial lock and actual Job barrier."""
    barrier.wait()
    check_entry_cache(payload["prefix"])
    lock_path = Path(payload["lock_path"]).resolve()
    lock_ref = {"path": str(lock_path), "sha256": payload["lock_sha256"]}
    lock = validate_lock(read_artifact(lock_ref, 65536), lock_path)
    directory = Path(lock["output"])
    payload["deadline"] = min(payload["deadline"], payload["attempt_deadline"])
    budget = AttemptBudget(directory, payload["deadline"], lock["N"] * 101376 + (2 << 30))
    running, finished, range_reports = {}, {}, {}
    started = time.monotonic()
    timings = {}
    try:
        budget.check(True)
        expected_sources = lock["source_sha256"]
        if any(expected_sources.get(path) != digest for path, digest in source_inventory().items()):
            raise ValueError("lock omits or changes a direct/transitive runtime source")
        storage = disk_admission(directory, lock["storage"]["remaining_tiers"], lock["storage"]["additional_copy_bytes"])
        write_new(directory / "inside-job-storage.json", canonical(storage))
        if storage["status"] != "PASS":
            raise OSError("remaining tiers and mandatory spare not admitted")
        build_bytes = read_artifact(lock["build"], 4 << 20)
        build = parse_json(build_bytes)
        binary = Path(build["binary"]).resolve()
        verify_build(binary, lock["build"]["path"], lock["build"]["sha256"], production=True)
        if any(expected_sources.get(str(Path(path).resolve())) != digest for path, digest in build["source_sha256"].items()):
            raise ValueError("build source inputs differ from locked runtime sources")
        identity = subprocess.run([str(binary), "--identity"], capture_output=True, env=runtime_environment(compiler=build["compiler"]),
                                  timeout=max(0.01, min(10, payload["deadline"] - time.monotonic())), cwd=ROOT)
        write_new(directory / "identity.stdout", identity.stdout)
        write_new(directory / "identity.stderr", identity.stderr)
        validate_native_identity(identity.returncode, identity.stdout)
        budget.check()
        catalogue_path = directory / "catalogue.bin"
        export = export_catalogue(lock["catalogue_npz"]["path"], lock["catalogue_npz"]["sha256"], catalogue_path)
        if export["pins"] != lock["catalogue_pins"]:
            raise ValueError("accepted catalogue factory pins differ from lock")
        catalogue, _, _ = admit_export(lock["catalogue_npz"]["path"], lock["catalogue_npz"]["sha256"], catalogue_path, export["sha256"])
        action_path, template_path = directory / "actions.bin", directory / "templates.bin"
        write_new(action_path, actions_wire())
        templates = export_templates(catalogue, template_path, budget.check)
        ref = lambda path: {"path": str(Path(path).resolve()), "sha256": sha(path)}
        descriptor = {key: lock[key] for key in ("tier", "N", "H", "L", "range_count", "workers", "deadline_seconds",
                       "memory_bytes", "source_sha256", "build", "readiness", "preservation")}
        descriptor.update(schema="q4-composite-input-descriptor-2", catalogue=dict(ref(catalogue_path), pins=export["pins"]),
                          actions=ref(action_path), templates=ref(template_path))
        descriptor_raw = canonical(descriptor)
        validate_descriptor(descriptor_raw)
        descriptor_path = directory / "descriptor.json"
        write_new(descriptor_path, descriptor_raw)
        descriptor_hash = hashlib.sha256(descriptor_raw).hexdigest()
        mapping = prepare_map(catalogue, lock["tier"], descriptor_hash, descriptor["actions"]["sha256"], directory / "preparation", budget.check)
        # Release Python catalogue/export arrays before32 native readers start.
        del catalogue
        admission_arguments = {"lock_sha256": payload["lock_sha256"], "descriptor_path": descriptor_path, "descriptor_sha256": descriptor_hash,
                               "map_path": mapping["path"], "map_sha256": mapping["sha256"], "catalogue_path": catalogue_path,
                               "catalogue_sha256": export["sha256"]}
        admission_raw = generated_admission(lock_path, **admission_arguments)
        write_new(directory / "admission.json", admission_raw)
        admission_hash = hashlib.sha256(admission_raw).hexdigest()
        verify_generated_admission(lock_path, **admission_arguments)
        timings["preparation_export_admission_seconds"] = time.monotonic() - started
        with MapIndex(mapping["path"], mapping["sha256"], descriptor_hash, descriptor["actions"]["sha256"]) as admitted_map:
            multiplicities, owned = validate_all_aliases(admitted_map, budget.check)
            complete = bytearray(admitted_map.header["G_execution"])
            reduction = Reduction()
            next_range = 0
            evolution_started = time.monotonic()
            while next_range < 64 or running:
                budget.check()
                while next_range < 64 and len(running) < 32:
                    index = next_range
                    next_range += 1
                    folder = directory / ("range-%02d" % index)
                    folder.mkdir()
                    command = [str(binary), "--run-range", "--descriptor", str(descriptor_path), "--descriptor-sha256", descriptor_hash,
                               "--lock", str(lock_path), "--lock-sha256", payload["lock_sha256"], "--map", mapping["path"],
                               "--map-sha256", mapping["sha256"], "--catalogue", str(catalogue_path), "--catalogue-sha256", export["sha256"],
                               "--range", str(index), "--output", str(folder / "range.bin")]
                    process = subprocess.Popen(command, cwd=ROOT, env=runtime_environment(compiler=build["compiler"]),
                                               stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                    write_new(folder / "launch.json", canonical({"range": index, "pid": process.pid, "argv": command,
                              "binary_sha256": build["binary_sha256"], "admission_sha256": admission_hash}))
                    running[index] = (process, BoundedLogs(process, folder), folder)
                for index, (process, logs, folder) in tuple(running.items()):
                    logs.check()
                    code = process.poll()
                    if code is None:
                        continue
                    logs.finish(payload["deadline"])
                    width = lock["N"] // 64
                    header = {"schema": "q4-composite-range-2", "tier": lock["tier"], "range": index, "start": index * width,
                              "stop": (index + 1) * width, "lock_sha256": payload["lock_sha256"], "input_descriptor_sha256": descriptor_hash,
                              "executable_sha256": build["binary_sha256"], "group_map_sha256": mapping["sha256"], "H": 304, "L": 64,
                              "kernel_wire": WIRE_ID, "observation_encoding": "frozen-python-case-result-1", "compression": CODEC}
                    if not (folder / "range.bin").is_file():
                        raise ValueError("native range failed before producing any authenticated bytes")
                    report = reduce_range(folder / "range.bin", header, admitted_map, owned[index], complete, reduction.add, budget.check)
                    report["exit_code"] = code
                    report["native_accounting"] = decode_native_terminal((folder / "stdout").read_bytes(), report)
                    report["stderr_sha256"] = sha(folder / "stderr")
                    write_new(folder / "reduction.json", canonical(report))
                    range_reports[index] = report
                    finished[index] = process.pid
                    del running[index]
                    if code or report["status"] != "COMPLETE":
                        raise ValueError("incomplete native range prevents tier completion")
                time.sleep(0.005)
            timings["native_execution_stream_reduction_seconds"] = time.monotonic() - evolution_started
            finalizing = time.monotonic()
            globally_covered = 0
            for index in range(lock["N"]):
                if index % 256 == 0:
                    budget.check()
                globally_covered += bool(complete[admitted_map.original(index)["group"]])
            if (not all(complete) or globally_covered != lock["N"] or reduction.groups != admitted_map.header["G_execution"]
                    or reduction.originals != lock["N"] or sum(multiplicities) != lock["N"] or len(range_reports) != 64):
                raise ValueError("unresolved original aliases or missing group/range")
            check_pins(expected_sources)
            verify_loaded_imports()
            verify_build(binary, lock["build"]["path"], lock["build"]["sha256"], production=True)
            validate_descriptor(read_artifact({"path": str(descriptor_path), "sha256": descriptor_hash}, 65536))
            if sha(descriptor_path) != descriptor_hash or sha(mapping["path"]) != mapping["sha256"]:
                raise ValueError("descriptor/map changed after exact reduction")
            if verify_generated_admission(lock_path, **admission_arguments) != admission_hash:
                raise ValueError("generated admission changed after workers")
            if any(_pid_alive(pid) for pid in finished.values()):
                raise ValueError("completed native worker remains alive")
            budget.check(True)
            report = {"schema": "q4-composite-tier-result-2", "status": "COMPLETE_SCOPED_EXECUTION", "canonical_adoption": False,
                      "tier": lock["tier"], "N": lock["N"], "G_state": admitted_map.header["G_state"], "G_execution": admitted_map.header["G_execution"],
                      "O": admitted_map.header["O"], "all_original_aliases_resolved": globally_covered,
                      "reported_executed_ticks": sum(r["native_accounting"]["reported_executed_ticks"] for r in range_reports.values()),
                      "primary_executed_ticks": sum(r["native_accounting"]["primary_executed_ticks"] for r in range_reports.values()),
                      "certificate_replay_ticks": sum(r["native_accounting"]["certificate_replay_ticks"] for r in range_reports.values()),
                      "verified_durable_ticks": sum(r["verified_durable_ticks"] for r in range_reports.values()),
                      "range_receipts": [ref(directory / ("range-%02d" % index) / "reduction.json") for index in range(64)],
                      "lock": lock_ref, "descriptor": ref(descriptor_path), "map": {"path": mapping["path"], "sha256": mapping["sha256"]},
                      "admission_sha256": admission_hash, "timings": timings, "reductions": reduction.report(),
                      "support_cache_used": False, "actual_loaded_modules": verify_loaded_imports()}
            timings["scientific_finalization_seconds"] = time.monotonic() - finalizing
            write_new(directory / "scientific-result.json", canonical(report))
            budget.check(True)
    except BaseException as error:
        # Metadata only. No reconstruction, certificate search, or deferred
        # scientific reduction is performed after deadline/worker failure.
        write_new(directory / "coordinator-failure.json", canonical({"schema": "q4-composite-coordinator-failure-1", "error": repr(error),
                  "elapsed_seconds": time.monotonic() - started, "completed_range_receipts": sorted(range_reports),
                  "started_worker_pids": {str(index): values[0].pid for index, values in running.items()}, "timings": timings}))
        raise


def _scientific_entry(lock_path, lock_sha256, prefix, attempt_started):
    check_entry_cache(prefix)
    if not 0 < attempt_started <= time.monotonic():
        raise ValueError("real monotonic attempt start required")
    parent_cleanup_job = MetadataProcessJob(cleanup=True)
    parent_job = MetadataProcessJob(SUPERVISOR_PARENT_CAP)
    lock_path = Path(lock_path).resolve()
    raw = read_artifact({"path": str(lock_path), "sha256": lock_sha256}, 65536)
    # Outer checks bind source and readiness before importing the supervisor;
    # the coordinator repeats all admission under the actual1800-second Job.
    lock = validate_lock(raw, lock_path)
    supervisor = verified_supervisor()
    remaining = 1800 - (time.monotonic() - attempt_started)
    if remaining <= 0:
        raise TimeoutError("deadline during complete static/source admission")
    result = supervisor.supervise(_production_coordinator, {"lock_path": str(lock_path), "lock_sha256": lock_sha256, "prefix": str(prefix),
                                  "attempt_deadline": attempt_started + 1800}, remaining, DESCENDANT_ALLOWANCE)
    result["scientific_interval_elapsed_seconds"] = time.monotonic() - attempt_started
    directory = Path(lock["output"])
    metadata_started = time.monotonic()
    ranges = []
    alive = []
    for index in range(64):
        folder = directory / ("range-%02d" % index)
        launch = folder / "launch.json"
        pid = parse_json(launch.read_bytes())["pid"] if launch.is_file() else None
        if pid is not None and _pid_alive(pid):
            alive.append(pid)
        retained = folder / "range.bin"
        reduction_path = folder / "reduction.json"
        if reduction_path.is_file():
            ranges.append({"range": index, "state": "RETAINED_PRIMARY_REDUCTION", "receipt": {"path": str(reduction_path), "sha256": sha(reduction_path)}})
        elif retained.is_file():
            with RangeStream(retained) as stream:
                while True:
                    frame = stream.next_frame()
                    if frame is None:
                        break
                    stream.commit(frame)  # Transport only, explicitly no scientific admission.
                prefix_record = stream.finish()
            ranges.append({"range": index, "state": "INTERRUPTED", "transport_prefix_only": prefix_record, "scientific_counts_verified": False})
        else:
            ranges.append({"range": index, "state": "UNSTARTED" if pid is None else "INTERRUPTED", "scientific_counts_verified": False})
    source_ok = True
    try:
        check_pins(lock["source_sha256"])
    except (OSError, ValueError):
        source_ok = False
    result.update(schema="q4-composite-supervised-attempt-2", lock_sha256=lock_sha256, ranges=ranges,
                  alive_descendants_after_cleanup=alive, source_unchanged=source_ok, canonical_adoption=False,
                  posttermination_metadata_seconds=time.monotonic() - metadata_started,
                  registered_aggregate_memory_bytes=8 << 30, outer_monitor_reserved_bytes=OUTER_MONITOR_CAP,
                  supervisor_parent_os_cap_bytes=SUPERVISOR_PARENT_CAP, descendant_allowance_bytes=DESCENDANT_ALLOWANCE,
                  supervisor_parent_peak_private_bytes=parent_job.peak_memory(), actual_supervisor_parent_os_assignment=parent_job.contains(os.getpid()),
                  actual_parent_cleanup_assignment=parent_cleanup_job.contains(os.getpid()))
    good = (result["exit_code"] == 0 and result["failure"] is None and result["actual_job_assignment"]
            and result["scientific_interval_elapsed_seconds"] <= 1800
            and not result["coordinator_alive_after_cleanup"] and not alive and source_ok and (directory / "scientific-result.json").is_file())
    result["status"] = "COMPLETE_SCOPED_EXECUTION" if good else "INCOMPLETE_RESOURCE" if result["failure"] else "INCOMPLETE_EXECUTION"
    write_new(directory / "supervision.json", canonical(result))
    if not good:
        raise ValueError("composite attempt incomplete; every raw artifact retained")
    return result


def retained_range_inventory(directory):
    """Bounded metadata-only inventory, including all unstarted ranges.

    This deliberately does not parse trajectories or hash unbounded range
    files after an outer watchdog. Existing primary receipts remain separate.
    """
    rows = []
    for index in range(64):
        folder = Path(directory) / ("range-%02d" % index)
        stream = folder / "range.bin"
        receipt = folder / "reduction.json"
        row = {"range": index, "state": "UNSTARTED" if not (folder / "launch.json").exists() else "INTERRUPTED",
               "range_path": str(stream), "retained_bytes": stream.stat().st_size if stream.is_file() else 0,
               "scientific_counts_verified_by_outer_metadata": False}
        if receipt.is_file():
            row["primary_receipt"] = {"path": str(receipt), "sha256": sha(receipt)}
            row["state"] = "RETAINED_PRIMARY_REDUCTION"
        rows.append(row)
    return rows


def launch_from_lock(lock_path, lock_sha256):
    attempt_started = time.monotonic()
    lock_path = Path(lock_path).resolve()
    # Read-only fail-closed admission precedes creating the new bootstrap.
    raw = read_artifact({"path": str(lock_path), "sha256": hash_text(lock_sha256)}, 65536)
    lock = validate_lock(raw, lock_path)
    directory = Path(lock["output"])
    envelope = directory / "bootstrap"
    envelope.mkdir(exist_ok=False)
    prefix = envelope / "empty-cache"
    prefix.mkdir()
    outer_cleanup_job = MetadataProcessJob(cleanup=True)
    outer_job = MetadataProcessJob(OUTER_MONITOR_CAP)
    outer_peak = _private_memory()
    if outer_peak > OUTER_MONITOR_CAP:
        raise MemoryError("fresh stdlib outer monitor exceeds its reserved memory cap")
    environment, removed = bootstrap_environment(prefix)
    command = [sys.executable, "-B", "-X", "pycache_prefix=" + str(prefix), str(Path(__file__).resolve()),
               "--scientific-entry", "--lock", str(lock_path), "--lock-sha256", lock_sha256, "--entry-sha256", sha(__file__),
               "--attempt-started", repr(attempt_started)]
    write_new(envelope / "launch.json", canonical({"argv": command, "removed_environment_names": removed,
              "interpreter": sys.executable, "interpreter_sha256": sha(sys.executable), "version": sys.version,
              "entry_sha256": sha(__file__), "effective_python_environment": {name: value for name, value in environment.items() if name.startswith("PYTHON")}}))
    # The inner frozen supervisor owns the deadline and complete Job cleanup;
    # an outer timeout must never orphan that supervisor and its descendants.
    process = subprocess.Popen(command, cwd=ROOT, env=environment, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if outer_job.contains(process.pid) or not outer_cleanup_job.contains(process.pid):
        process.kill()
        process.wait(timeout=5)
        raise ValueError("scientific supervisor failed to break away from the outer metadata cap")
    logs = BoundedLogs(process, envelope)
    monitored = monitor_supervisor(process, logs, attempt_started + 1800 + OUTER_METADATA_ALLOWANCE_SECONDS)
    outer_failure = monitored["outer_failure"]
    outer_peak = max(outer_peak, monitored["outer_monitor_peak_private_bytes"])
    write_new(envelope / "terminal.json", canonical({"exit_code": process.returncode, "stdout_sha256": sha(envelope / "stdout"),
              "stderr_sha256": sha(envelope / "stderr"), "cache_empty": not any(prefix.iterdir()), "outer_failure": outer_failure,
              "outer_monitor_peak_private_bytes": max(outer_peak, outer_job.peak_memory()), "outer_monitor_cap_bytes": OUTER_MONITOR_CAP,
              "outer_monitor_os_enforced": True, "metadata_allowance_seconds": OUTER_METADATA_ALLOWANCE_SECONDS,
              "bounded_emergency_cleanup_seconds": 7, "retained_ranges": retained_range_inventory(directory)}))
    if process.returncode or any(prefix.iterdir()) or outer_failure:
        raise ValueError("actual composite bootstrap/attempt failed; retained " + str(directory))
    return parse_json((directory / "supervision.json").read_bytes())


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    modes = parser.add_mutually_exclusive_group(required=True)
    modes.add_argument("--guard-entry", metavar="EMPTY_PREFIX")
    modes.add_argument("--bootstrap-control", metavar="NEW_DIRECTORY")
    modes.add_argument("--build", metavar="NEW_DIRECTORY")
    modes.add_argument("--export-catalogue", metavar="NEW_FILE")
    modes.add_argument("--supervisor-control", metavar="NEW_DIRECTORY")
    modes.add_argument("--control-entry", metavar="DIRECTORY")
    modes.add_argument("--control-descendant", metavar="KIND")
    modes.add_argument("--scientific-entry", action="store_true")
    modes.add_argument("--run", action="store_true")
    parser.add_argument("--catalogue")
    parser.add_argument("--catalogue-sha256")
    parser.add_argument("--checked", action="store_true")
    parser.add_argument("--control-kind", choices=("success", "deadline", "memory", "quota", "log", "outer_memory", "outer_loss", "supervisor_loss", "outer_timeout"))
    parser.add_argument("--entry-sha256")
    parser.add_argument("--lock")
    parser.add_argument("--lock-sha256")
    parser.add_argument("--attempt-started", type=float)
    args = parser.parse_args(argv)
    if args.entry_sha256 is not None and sha(__file__) != hash_text(args.entry_sha256):
        raise ValueError("actual entry source identity mismatch")
    if args.guard_entry:
        prefix = check_entry_cache(args.guard_entry)
        _oracle()
        check_entry_cache(prefix)
        sys.stdout.buffer.write(canonical({"schema": "q4-composite-entry-control-1", "status": "PASS", "prefix": str(prefix),
                                          "writes_disabled": sys.dont_write_bytecode, "source_sha256": source_inventory(), "registered_campaign": False}))
    elif args.bootstrap_control:
        bootstrap_guard(args.bootstrap_control)
    elif args.build:
        build_adapter(args.build, args.checked)
    elif args.export_catalogue:
        if sys.pycache_prefix is None:
            raise ValueError("catalogue export requires accepted fresh-prefix entry")
        check_entry_cache(sys.pycache_prefix)
        export_catalogue(args.catalogue, args.catalogue_sha256, args.export_catalogue)
    elif args.supervisor_control:
        supervisor_control(args.supervisor_control, args.control_kind)
    elif args.control_entry:
        if args.entry_sha256 is None or sys.pycache_prefix is None:
            raise ValueError("bounded control requires the accepted actual bootstrap")
        if args.control_kind == "outer_memory":
            _outer_control_entry(args.control_entry, sys.pycache_prefix)
        elif args.control_kind in ("outer_loss", "supervisor_loss", "outer_timeout"):
            _parent_loss_control_entry(args.control_entry, args.control_kind, sys.pycache_prefix)
        else:
            _run_supervisor_control_entry(args.control_entry, args.control_kind, sys.pycache_prefix)
    elif args.control_descendant:
        if args.entry_sha256 is None or sys.pycache_prefix is None:
            raise ValueError("descendant control bootstrap")
        check_entry_cache(sys.pycache_prefix)
        if args.control_descendant == "success":
            time.sleep(0.05)
        elif args.control_descendant == "outer-breakaway":
            proof = bytearray(160 << 20)
            time.sleep(0.1)
        elif args.control_descendant == "log":
            for _ in range(4096):
                sys.stdout.buffer.write(b"x" * 4096)
            sys.stdout.buffer.flush()
        else:
            time.sleep(30)
    elif args.scientific_entry:
        if args.entry_sha256 is None or sys.pycache_prefix is None or args.lock is None or args.lock_sha256 is None or args.attempt_started is None:
            raise ValueError("scientific entry requires the reviewed immutable lock and fresh bootstrap")
        _scientific_entry(args.lock, args.lock_sha256, sys.pycache_prefix, args.attempt_started)
    else:
        if args.lock is None or args.lock_sha256 is None:
            raise ValueError("registered production execution unavailable without the complete independent readiness and immutable initial lock")
        launch_from_lock(args.lock, args.lock_sha256)


if __name__ == "__main__":
    main()
