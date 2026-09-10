"""Bounded independent Q2 fixtures for the selected native carrier-current gate.

Only recovery_credit_exchange_q2.transition evolves the oracle. Sparse Q4
types/validation describe the output; neither sparse nor dense evolution is
called. No particle identity or mechanical energy/momentum is inferred.
"""
from dataclasses import dataclass
import hashlib
from itertools import product
import json
from pathlib import Path
import re
import struct
import subprocess

from . import recovery_credit_exchange_q2 as Q
from . import sparse_credit_exchange_q4 as S

MAGIC = "FTD-CARRIER-CURRENT-FIXTURES-1"
CASE_COUNT = 243
TRANSACTION_COUNT = 11491
L = 16
ROOT = Path(__file__).resolve().parents[2]
EVENT_NAMES = ("moves", "redirects", "capacity_holds", "attempt_marks",
               "attempt_expiries", "onsite_alignments", "credit_exchanges")
EVENT_WIDTHS = (9, 5, 3, 3, 2, 5, 13)
EVENT_CAPACITIES = (2, 2, 1, 2, 4, 2, 2)


@dataclass(frozen=True)
class Case:
    id: str
    kind: str
    heading: int = 0
    order: int = 0
    eta: int = 0
    permutation: int = 0
    origin: int = 0
    reached: int = 0


@dataclass(frozen=True)
class Fixture:
    case: Case
    states: tuple
    events: tuple
    quotients: tuple
    anchors: tuple


def cases():
    rows = [Case(f"seam-h{h}-o{o}-e{e}-a{a}", "seam", h, o, e, a)
            for h, o, e, a in product(range(6), range(2), range(2), range(6))]
    rows += [Case(f"origin-{b}", "origin", origin=b) for b in range(8)]
    rows += [Case(f"reached-{p}", "continuation", reached=p) for p in range(19)]
    rows += [Case(f"{kind}-h{h}-o{o}-e{e}", kind, h, o, e)
             for kind, h, o, e in product(
                 ("straight-outward", "bent-opposed", "bent-transverse"),
                 range(6), range(2), range(2))]
    assert len(rows) == CASE_COUNT and len({c.id for c in rows}) == CASE_COUNT
    return tuple(rows)


def _vector(case, channel):
    out = [0, 0, 0]
    out[Q.AXES[case.permutation][channel // 2]] = 1 if channel % 2 == 0 else -1
    return tuple(out)


def _direction(case, channel):
    return 2 * Q.AXES[case.permutation][channel // 2] + channel % 2 + 1


def _add(point, vector):
    return tuple((a + b) % L for a, b in zip(point, vector))


def _color(case, point):
    return tuple(((case.origin >> a) & 1) ^ (point[axis] % 2)
                 for a, axis in enumerate(Q.AXES[case.permutation]))


def _offset(case):
    axis = (Q.AXES[case.permutation][case.heading // 2] + 1) % 3
    return tuple(8 if a == axis else 0 for a in range(3))


def preparation(case):
    """Prescribed ordinal-zero record, before the real reset transaction."""
    h, t = case.heading, 2 * ((case.heading // 2 + 1) % 3)
    path, velocities, credits = (h,), (h, h), (1, 0)
    if case.kind == "straight-outward":
        path, velocities, credits = (h, h), (h ^ 1, h), (0, 0)
    elif case.kind == "bent-opposed":
        path, velocities, credits = (h, t), (h ^ 1, t), (0, 0)
    elif case.kind == "bent-transverse":
        path, velocities, credits = (h, t), (t ^ 1, h), (0, 0)
    anchor = [4, 4, 4]
    if case.kind == "seam":
        anchor[Q.AXES[case.permutation][h // 2]] = 15 if h % 2 == 0 else 0
    anchor = tuple(anchor)
    if case.order:
        for channel in path:
            anchor = _add(anchor, _vector(case, channel))
        path = tuple(channel ^ 1 for channel in reversed(path))
        velocities, credits = velocities[::-1], credits[::-1]
    q = Q.QuotientState(0, _color(case, anchor), Q.Record(path, velocities, credits, (0, 0)))
    return Q.normalized(q), anchor


def _component(case, q, anchor):
    point, edges = anchor, []
    epsilon = 1 if case.eta == 0 else -1
    for channel in q.record.path:
        target = _add(point, _vector(case, channel))
        owner = point if channel % 2 == 0 else target
        edges.append(S.Edge(S.site_index(L, owner), Q.AXES[case.permutation][channel // 2],
                            epsilon * (1 if channel % 2 == 0 else -1)))
        point = target
    carriers = tuple(S.Carrier(S.site_index(L, p), slot ^ case.eta,
                              _direction(case, q.record.velocities[slot]),
                              q.record.credits[slot], q.record.attempted[slot])
                     for slot, p in enumerate((anchor, point)))
    return carriers, tuple(edges)


def lift(case, q, anchor, tick):
    """Sparse complete lift of two actual copies; no unrepresented spectator."""
    Q.normalized(q)
    if q.stage != tick % 19 or q.color != _color(case, anchor):
        raise ValueError("quotient clock/background mismatch")
    c0, e0 = _component(case, q, anchor)
    c1, e1 = _component(case, q, _add(anchor, _offset(case)))
    state = S.SparseQ4State(L, tick, tuple(sorted(c0 + c1)), tuple(sorted(e0 + e1)),
                            8 * case.permutation + case.origin, case.eta)
    S.validate(state)
    return state


def _event_rows(case, before_q, after_q, anchor, operation):
    """Translate the independent quotient's decision into complete event rows."""
    rows = [[] for _ in EVENT_NAMES]
    for point in (anchor, _add(anchor, _offset(case))):
        carriers, edges = _component(case, before_q, point)
        after_point = point
        # Source displacement can be read from the path quotient decision:
        # operation 4/5 moves the unique scheduled endpoint along its old heading.
        logical_slot = (before_q.stage - 7) // 6 if before_q.stage >= 7 else None
        if operation in (4, 5) and logical_slot == 0:
            after_point = _add(point, _vector(case, before_q.record.velocities[0]))
        new_carriers, new_edges = _component(case, after_q, after_point)
        if before_q.stage == 0:
            for carrier in carriers:
                if carrier.attempted:
                    rows[4].append((1 if carrier.slot == 0 else -1, carrier.site))
            if operation == 2:
                by_slot = {c.slot: c for c in carriers}
                rows[5].append((carriers[0].site, 1 if case.eta == 0 else -1,
                                by_slot[0].direction, by_slot[1].direction, new_carriers[0].direction))
        elif operation == 3:
            donor_slot = before_q.record.credits.index(1)
            donor, recipient = carriers[donor_slot], carriers[donor_slot ^ 1]
            edge = edges[0]
            rows[6].append((edge.owner, edge.axis, 1 if donor.slot == 0 else -1, donor.site,
                            1 if recipient.slot == 0 else -1, recipient.site, 1, 0, 0, 1,
                            donor.direction, recipient.direction, new_carriers[0].direction))
        elif operation in (4, 5, 6, 7):
            carrier, new_carrier = carriers[logical_slot], new_carriers[logical_slot]
            epsilon = 1 if carrier.slot == 0 else -1
            axis, negative = divmod(carrier.direction - 1, 2)
            owner = carrier.site if not negative else S.shift(L, carrier.site, axis, -1)
            if operation in (4, 5):
                old_flux = {(e.owner, e.axis): e.q for e in edges}
                new_flux = {(e.owner, e.axis): e.q for e in new_edges}
                rows[0].append((epsilon, carrier.site, new_carrier.site, owner, axis,
                                old_flux.get((owner, axis), 0), new_flux.get((owner, axis), 0),
                                carrier.credit, new_carrier.credit))
                reason = 0
            else:
                reason = 3 if operation == 6 else 4
                rows[1].append((epsilon, carrier.site, carrier.direction, new_carrier.direction, reason))
            rows[3].append((epsilon, carrier.site, reason))
    # Native resets follow (site, physical slot); transport follows owned edges.
    rows[0].sort(key=lambda r: (r[3], r[4]))
    for index in (1, 3):
        def owned_key(row):
            source = row[1]
            carrier = next(c for c in lift(case, before_q, anchor, before_q.stage).carriers
                           if c.site == source and c.slot == (0 if row[0] == 1 else 1))
            axis, negative = divmod(carrier.direction - 1, 2)
            return (S.shift(L, source, axis, -1) if negative else source, axis)
        rows[index].sort(key=owned_key)
    rows[4].sort(key=lambda r: (r[1], -r[0]))
    rows[5].sort()
    rows[6].sort(key=lambda r: (r[0], r[1]))
    return tuple(tuple(group) for group in rows)


def advance(case, q, anchor, tick):
    after, displacement, _, operation = Q.transition(q)
    physical_displacement = [0, 0, 0]
    for a, axis in enumerate(Q.AXES[case.permutation]):
        physical_displacement[axis] = displacement[a]
    new_anchor = _add(anchor, physical_displacement)
    events = _event_rows(case, q, after, anchor, operation)
    return after, new_anchor, lift(case, after, new_anchor, tick + 1), events


def trace(case):
    q, anchor = preparation(case)
    tick = 0
    if case.kind == "continuation":
        for _ in range(1 + case.reached):
            q, anchor, _, _ = advance(case, q, anchor, tick)
            tick += 1
    count = 1 if case.kind == "continuation" else 39 if case.kind in ("seam", "origin") else 77
    states, events, quotients, anchors = [lift(case, q, anchor, tick)], [], [q], [anchor]
    for _ in range(count):
        q, anchor, state, event = advance(case, q, anchor, tick)
        tick += 1
        states.append(state)
        events.append(event)
        quotients.append(q)
        anchors.append(anchor)
    return Fixture(case, tuple(states), tuple(events), tuple(quotients), tuple(anchors))


def fixtures():
    for case in cases():
        yield trace(case)


def native_state_wire(state):
    S.validate(state)
    if not 4 <= state.L <= 64:
        raise ValueError("outside native size domain")
    out = bytearray(b"FTD-Q4-NATIVE-1\n" + b"".join(bytes.fromhex(x) for x in
                    (S.RULE_HASH, S.FRAME_HASH, S.DENSE_ENCODING)))
    out += struct.pack("<HBBB3x", state.L, state.origin_code, state.charge_frame, len(state.edges))
    for c in state.carriers:
        out += struct.pack("<IBBBB", c.site, c.slot, c.direction, c.credit, c.attempted)
    for e in state.edges:
        out += struct.pack("<IBb2x", e.owner, e.axis, e.q)
    out += bytes(8 * (4 - len(state.edges)))
    tick = format(state.microtick, "x").encode("ascii")
    out += struct.pack("<Q", len(tick)) + tick
    return bytes(out)


def native_event_wire(events):
    if len(events) != 7:
        raise ValueError("seven complete event groups required")
    out = bytearray(bytes(len(group) for group in events) + b"\0")
    for group, width, capacity in zip(events, EVENT_WIDTHS, EVENT_CAPACITIES):
        if len(group) > capacity or any(len(row) != width for row in group):
            raise ValueError("event shape/capacity mismatch")
        for row in group:
            if any(type(value) is not int for value in row):
                raise ValueError("exact integer event words required")
            out += struct.pack("<" + "i" * width, *row)
    return bytes(out)


def write_fixtures(path, records=None):
    records = fixtures() if records is None else records
    count = transactions = 0
    with Path(path).open("x", encoding="ascii", newline="\n") as stream:
        stream.write(f"{MAGIC}\n{CASE_COUNT}\n")
        prescribed = cases()
        for fixture in records:
            if count >= CASE_COUNT or fixture.case != prescribed[count]:
                raise ValueError("fixture case identity/order differs from frozen matrix")
            expected_count = 1 if fixture.case.kind == "continuation" else 39 if fixture.case.kind in ("seam", "origin") else 77
            if len(fixture.events) != expected_count or len(fixture.states) != expected_count + 1:
                raise ValueError("fixture transaction inventory differs from frozen matrix")
            stream.write(f"CASE {fixture.case.id} {len(fixture.events)}\n")
            stream.write(native_state_wire(fixture.states[0]).hex() + "\n")
            for state, events in zip(fixture.states[1:], fixture.events):
                stream.write(native_state_wire(state).hex() + " " + native_event_wire(events).hex() + "\n")
            count += 1
            transactions += len(fixture.events)
    if count != CASE_COUNT or transactions != TRANSACTION_COUNT:
        raise ValueError("fixture matrix incomplete")
    if Path(path).stat().st_size > 10 * 1024 * 1024:
        raise ValueError("fixture stream exceeds frozen size bound")
    return {"cases": count, "transactions": transactions,
            "fixture_sha256": hashlib.sha256(Path(path).read_bytes()).hexdigest(),
            "fixture_bytes": Path(path).stat().st_size}


def validate_fixture_file(path):
    """Check every row against the frozen independent construction, not labels."""
    path = Path(path)
    if path.stat().st_size > 10 * 1024 * 1024:
        raise ValueError("fixture stream exceeds frozen size bound")
    with path.open("r", encoding="ascii", newline="") as stream:
        if stream.readline() != MAGIC + "\n" or stream.readline() != str(CASE_COUNT) + "\n":
            raise ValueError("fixture magic/count mismatch")
        transactions = 0
        for fixture in fixtures():
            if stream.readline() != f"CASE {fixture.case.id} {len(fixture.events)}\n":
                raise ValueError("fixture case identity/order/count mismatch")
            if stream.readline() != native_state_wire(fixture.states[0]).hex() + "\n":
                raise ValueError("fixture initial state differs from independent preparation")
            for state, events in zip(fixture.states[1:], fixture.events):
                expected = native_state_wire(state).hex() + " " + native_event_wire(events).hex() + "\n"
                if stream.readline() != expected:
                    raise ValueError("fixture successor/event differs from independent transition")
                transactions += 1
        if stream.read(1):
            raise ValueError("trailing fixture data")
    if transactions != TRANSACTION_COUNT:
        raise ValueError("fixture transaction count mismatch")
    return {"cases": CASE_COUNT, "transactions": transactions}


def source_hashes():
    paths = [Path(__file__), Path(Q.__file__), Path(S.__file__), Path(S.dense.__file__),
             ROOT / "engine/docs/CONTRACT_NATIVE_CARRIER_CURRENT_GATE_V1.md",
             ROOT / "scripts/phi_v2_lattice/native/sparse_q4_kernel_v1.hpp",
             ROOT / "scripts/phi_v2_lattice/native/sparse_q4_kernel_v1.cpp",
             ROOT / "engine/strict/carrier_observer.h",
             ROOT / "engine/strict/carrier_observer.cpp",
             ROOT / "engine/tests/test_native_carrier_current.cpp",
             ROOT / "scripts/tests/phi_v2_lattice/test_native_carrier_current_fixtures.py"]
    return {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}


def run_parity(probe, fixture_path):
    probe = Path(probe).resolve()
    if not probe.is_file():
        raise FileNotFoundError("native carrier-current test executable required: " + str(probe))
    before = source_hashes()
    binary_hash = hashlib.sha256(probe.read_bytes()).hexdigest()
    fixture_path = Path(fixture_path).resolve()
    fixture_hash = hashlib.sha256(fixture_path.read_bytes()).hexdigest()
    inventory = validate_fixture_file(fixture_path)
    if fixture_hash != hashlib.sha256(fixture_path.read_bytes()).hexdigest():
        raise RuntimeError("fixture changed during validation")
    result = subprocess.run([str(probe), "--fixtures", str(Path(fixture_path).resolve())],
                            capture_output=True, text=True, timeout=60, check=False)
    receipt = {"schema": "native-carrier-current-parity-1", "law": Q.LAW_ID,
               "canonical_adoption": False, "source_sha256": before,
               "binary_sha256": binary_hash, "fixture_sha256": fixture_hash,
               **inventory,
               "returncode": result.returncode, "stdout": result.stdout, "stderr": result.stderr}
    if (before != source_hashes() or binary_hash != hashlib.sha256(probe.read_bytes()).hexdigest()
            or fixture_hash != hashlib.sha256(fixture_path.read_bytes()).hexdigest()):
        raise RuntimeError("direct source, binary or fixture changed during native parity")
    totals = re.findall(r"^FTD_CARRIER_CURRENT_FIXTURES_PASS cases=(\d+) transactions=(\d+)\r?$",
                        result.stdout, flags=re.MULTILINE)
    if result.returncode or totals != [(str(inventory["cases"]), str(inventory["transactions"]))]:
        raise RuntimeError(json.dumps(receipt, sort_keys=True))
    receipt["status"] = "PASS"
    return receipt


def main():
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--probe", type=Path)
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    report = {"schema": "native-carrier-current-fixtures-1", "source_sha256": source_hashes(),
              "native_parity": "unavailable", **write_fixtures(args.output)}
    if args.probe:
        report["native_parity"] = run_parity(args.probe, args.output)
    if args.receipt:
        with args.receipt.open("x", encoding="ascii") as stream:
            json.dump(report, stream, sort_keys=True, indent=2)
            stream.write("\n")
    print(json.dumps(report, sort_keys=True))


if __name__ == "__main__":
    main()
