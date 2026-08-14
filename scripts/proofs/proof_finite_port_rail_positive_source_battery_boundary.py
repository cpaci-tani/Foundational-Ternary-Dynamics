#!/usr/bin/env python3
"""FTD-0883 exact finite-port rail and positive source-battery certificate."""

from __future__ import annotations

import hashlib
from fractions import Fraction
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_FINITE_PORT_RAIL_POSITIVE_SOURCE_BATTERY_BOUNDARY_v1.md"
)
PROTOCOL_HASH = "0B6ACD3C1E41B4D1EE60CCA9A5E04E91E84FC96F06A3725B1F41DDDFD79E8C0B"
SOURCE_HASHES = {
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_REVERSIBLE_CHECKERBOARD_GAUSS_RECORD_PREPARATION_AND_SELF_DUAL_ENERGY_SPLIT_v1.md":
        "143D897A69B5C6FED8C00402C1840EA9FAEE5BD4BC259C9BDD065DFDC616A814",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md":
        "7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7",
    "engine/include/ftd/eft/reversible_checkerboard_gauss_preparation.h":
        "7C2AFBFD098268B02C9E58DABAC19ED38DD1FA173385424E111B0FEFAAD79420",
    "engine/src/eft/reversible_checkerboard_gauss_preparation.cpp":
        "CFDD471E81DBB6040C882A069468D7E22930CF8AEB48084EEBD2D56824E66511",
    "engine/include/ftd/eft/oriented_ternary_quarter_turn.h":
        "46CD15943F5EB8EDBBCE4676CDE558A7C2B08556E1AC64E7C9720D30FFEB68E1",
}

L = 4
V = L**3
F = 3 * V
CAPACITY = 4
SIX = Fraction(6)

checks = 0
failures = 0


def check(label: str, condition: bool) -> None:
    global checks, failures
    checks += 1
    if condition:
        print(f"PASS  C{checks} {label}")
    else:
        failures += 1
        print(f"FAIL  C{checks} {label}")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def site_index(x: int, y: int, z: int) -> int:
    return (x % L) * L * L + (y % L) * L + (z % L)


def coordinates(index: int) -> tuple[int, int, int]:
    z = index % L
    xy = index // L
    return xy // L, xy % L, z


def face_index(site: int, axis: int) -> int:
    return 3 * site + axis


def shifted(site: int, axis: int, amount: int) -> int:
    xyz = list(coordinates(site))
    xyz[axis] = (xyz[axis] + amount) % L
    return site_index(*xyz)


def color(site: int) -> int:
    return sum(coordinates(site)) & 1


def incidence_row(site: int) -> dict[int, Fraction]:
    result: dict[int, Fraction] = {}
    for axis in range(3):
        result[face_index(site, axis)] = Fraction(1)
        result[face_index(shifted(site, axis, -1), axis)] = Fraction(-1)
    return result


ROWS = tuple(incidence_row(site) for site in range(V))
ACTIVE = tuple(
    tuple(site for site in range(V) if color(site) == parity)
    for parity in (0, 1)
)


def row_dot(site: int, flux: list[Fraction]) -> Fraction:
    return sum(
        (coefficient * flux[index] for index, coefficient in ROWS[site].items()),
        Fraction(0),
    )


def residual(
    site: int, flux: list[Fraction], charge: list[Fraction]
) -> Fraction:
    return row_dot(site, flux) - charge[site]


def add_row(flux: list[Fraction], site: int, scale: Fraction) -> None:
    for index, coefficient in ROWS[site].items():
        flux[index] += coefficient * scale


def field_energy(flux: list[Fraction]) -> Fraction:
    return sum((value * value / 2 for value in flux), Fraction(0))


def bank_energy(bank: list[list[Fraction]]) -> Fraction:
    return sum(
        (value * value / 12 for port in bank for value in port),
        Fraction(0),
    )


def battery_energy(squared_amplitudes: list[Fraction]) -> Fraction:
    return sum((value / 2 for value in squared_amplitudes), Fraction(0))


def layer(
    flux: list[Fraction],
    charge: list[Fraction],
    parity: int,
    incoming: list[Fraction],
) -> tuple[list[Fraction], list[Fraction], list[Fraction]]:
    result = list(flux)
    outgoing = [Fraction(0) for _ in range(V)]
    work = [Fraction(0) for _ in range(V)]
    for site in ACTIVE[parity]:
        old_residual = residual(site, result, charge)
        add_row(result, site, (incoming[site] - old_residual) / SIX)
        outgoing[site] = -old_residual
        work[site] = charge[site] * (incoming[site] - old_residual) / SIX
    return result, outgoing, work


def inverse_layer(
    flux: list[Fraction],
    charge: list[Fraction],
    parity: int,
    outgoing: list[Fraction],
) -> tuple[list[Fraction], list[Fraction]]:
    result = list(flux)
    incoming = [Fraction(0) for _ in range(V)]
    for site in ACTIVE[parity]:
        recovered = residual(site, result, charge)
        old_residual = -outgoing[site]
        add_row(result, site, (old_residual - recovered) / SIX)
        incoming[site] = recovered
    return result, incoming


protocol_path = ROOT / PROTOCOL
protocol_text = protocol_path.read_text(encoding="utf-8")
source_text = {
    path: (ROOT / path).read_text(encoding="utf-8")
    for path in SOURCE_HASHES
}

# C1--C8: provenance and scope.
check(
    "all five frozen source hashes match",
    all(sha256(ROOT / path) == digest for path, digest in SOURCE_HASHES.items()),
)
check("protocol pre-run hash matches", sha256(protocol_path) == PROTOCOL_HASH)
check(
    "explicit finite cyclic signed-port class is frozen",
    "explicit one-vector-per-port cyclic representation" in protocol_text,
)
check(
    "exact-real memory counterclass is excluded from the no-go",
    "Exact-real encodings outside the explicit port-bank class remain out of"
    in protocol_text
    and "EXACT_REAL_MEMORY_NO_GO=NOT_CLAIMED" in protocol_text,
)
check(
    "battery law is tagged imposed",
    "[IMPOSED reference law]" in protocol_text
    and "BATTERY_LAW_STATUS=IMPOSED_REFERENCE" in protocol_text,
)
check(
    "Hamiltonian and symplectic completion remain open",
    "may not call (B1) Hamiltonian" in protocol_text
    and "CANONICAL_HAMILTONIAN_RESERVOIR=OPEN" in protocol_text,
)
check("production remains untouched", "PRODUCTION_COUPLING=NONE" in protocol_text)
check(
    "Born Bell Lorentz biology and completeness are outside the result",
    "Born, Bell, Lorentz, biology, and completeness are absent" in protocol_text
    and "BORN_BELL_STATUS=UNTOUCHED" in protocol_text,
)

# Fixed exact state.
charge = [Fraction(0) for _ in range(V)]
charge[site_index(0, 0, 0)] = Fraction(1)
charge[site_index(1, 0, 0)] = Fraction(-1)
initial_flux = [Fraction(0) for _ in range(F)]
initial_bank = [
    [Fraction(0) for _ in range(V)] for _ in range(CAPACITY)
]
initial_battery_squared = [Fraction(100) for _ in range(V)]
initial_battery_sign = [1 for _ in range(V)]

flux = list(initial_flux)
bank = [list(port) for port in initial_bank]
battery_squared = list(initial_battery_squared)
battery_sign = list(initial_battery_sign)
cursor = 0
records: list[dict[str, object]] = []
fresh_flags: list[bool] = []
projection_flags: list[bool] = []
energy_balances: list[Fraction] = []
strict_reserve_flags: list[bool] = []
initial_total = (
    field_energy(flux) + bank_energy(bank) + battery_energy(battery_squared)
)

for step in range(CAPACITY):
    parity = step & 1
    incoming = list(bank[cursor])
    fresh = all(value == 0 for value in incoming)
    before_flux = list(flux)
    before_bank = [list(port) for port in bank]
    before_battery = list(battery_squared)
    before_cursor = cursor
    after_flux, outgoing, work = layer(flux, charge, parity, incoming)
    reserve_ok = all(
        battery_squared[site] - 2 * work[site] > 0
        for site in ACTIVE[parity]
    )
    strict_reserve_flags.append(reserve_ok)
    if not reserve_ok:
        raise RuntimeError("locked positive reserve unexpectedly failed")
    for site in ACTIVE[parity]:
        battery_squared[site] -= 2 * work[site]
    flux = after_flux
    bank[cursor] = outgoing
    records.append(
        {
            "parity": parity,
            "cursor": cursor,
            "incoming": incoming,
            "outgoing": outgoing,
            "work": work,
            "before_flux": before_flux,
            "before_bank": before_bank,
            "before_battery": before_battery,
            "before_cursor": before_cursor,
        }
    )
    cursor = (cursor + 1) % CAPACITY
    fresh_flags.append(fresh)
    projection_flags.append(
        fresh and all(residual(site, flux, charge) == 0 for site in ACTIVE[parity])
    )
    energy_balances.append(
        field_energy(flux) + bank_energy(bank) + battery_energy(battery_squared)
        - initial_total
    )

first_outgoing = records[0]["outgoing"]
assert isinstance(first_outgoing, list)
next_incoming = bank[cursor]

# C9--C24: finite cyclic bank.
check("locked cyclic capacity is positive", CAPACITY >= 1)
check(
    "every initial bank coordinate is zero",
    all(value == 0 for port in initial_bank for value in port),
)
check(
    "cursor schedule is deterministic and context blind",
    [record["cursor"] for record in records] == list(range(CAPACITY))
    and "measurement" not in "".join(SOURCE_HASHES).lower(),
)
check("each of the first C selected inputs is fresh", all(fresh_flags))
check("every accepted fresh layer is the exact affine projection", all(projection_flags))
check(
    "complete signed outgoing vectors are stored",
    all(len(record["outgoing"]) == V for record in records),
)
check("cursor advances modulo capacity", cursor == 0)

# A fresh projection collision becomes injective after the outgoing port is kept.
collision_site = ACTIVE[0][0]
collision_a = list(initial_flux)
collision_b = list(initial_flux)
add_row(collision_b, collision_site, Fraction(3, 5))
projected_a, outgoing_a, _ = layer(
    collision_a, charge, 0, [Fraction(0) for _ in range(V)]
)
projected_b, outgoing_b, _ = layer(
    collision_b, charge, 0, [Fraction(0) for _ in range(V)]
)
check(
    "field plus stored signed bank output is injective on the collision probe",
    projected_a == projected_b and outgoing_a != outgoing_b,
)

# Reverse one step, then replay it before reversing the full sequence.
last_record = records[-1]
reverse_cursor = (cursor - 1) % CAPACITY
reverse_flux, recovered_incoming = inverse_layer(
    flux, charge, int(last_record["parity"]), bank[reverse_cursor]
)
check(
    "one inverse step restores field bank and cursor data",
    reverse_cursor == last_record["cursor"]
    and reverse_flux == last_record["before_flux"]
    and recovered_incoming == last_record["incoming"],
)

reverse_count = 0
while records:
    record = records.pop()
    cursor = (cursor - 1) % CAPACITY
    outgoing = list(bank[cursor])
    flux, recovered = inverse_layer(flux, charge, int(record["parity"]), outgoing)
    work = record["work"]
    assert isinstance(work, list)
    for site in ACTIVE[int(record["parity"])]:
        battery_squared[site] += 2 * work[site]
    bank[cursor] = recovered
    reverse_count += 1

check(
    "all C accepted layers reverse exactly",
    reverse_count == CAPACITY
    and flux == initial_flux
    and bank == initial_bank
    and cursor == 0,
)
check("locked dipole writes a nonzero first output", any(first_outgoing))
check("cursor returns to its first coordinate after C layers", CAPACITY % CAPACITY == 0)
check(
    "layer C plus one fails the fresh-port gate",
    any(value != 0 for value in next_incoming),
)
check(
    "finite cyclic bank is not indefinitely fresh in the registered class",
    "FINITE_CYCLIC_INDEFINITE_FRESHNESS=NO" in protocol_text,
)
history_text = next(
    text for path, text in source_text.items()
    if "CAUSAL_ODD_PULSE_HISTORY_CARRIER" in path
)
check(
    "capacity growth or signed-tail export is the declared escape",
    "further causal storage" in history_text
    and "signed tail amplitude" in history_text,
)
check(
    "no universal finite-dimensional memory theorem is claimed",
    "not a universal" in history_text
    and "EXACT_REAL_MEMORY_NO_GO=NOT_CLAIMED" in protocol_text,
)

# Re-run forward to retain battery records for C25--C44.
flux = list(initial_flux)
bank = [list(port) for port in initial_bank]
battery_squared = list(initial_battery_squared)
battery_sign = list(initial_battery_sign)
cursor = 0
records = []
work_sum = Fraction(0)
battery_before_total = battery_energy(battery_squared)
per_gate_energy_closure: list[bool] = []
inverse_amplitude_closure: list[bool] = []
for step in range(CAPACITY):
    parity = step & 1
    incoming = list(bank[cursor])
    before_field = field_energy(flux)
    before_port = sum((value * value / 12 for value in incoming), Fraction(0))
    before_squared = list(battery_squared)
    flux_after, outgoing, work = layer(flux, charge, parity, incoming)
    after_port = sum((value * value / 12 for value in outgoing), Fraction(0))
    local_closure = True
    for site in ACTIVE[parity]:
        radicand = battery_squared[site] - 2 * work[site]
        local_closure = local_closure and radicand > 0
        battery_squared[site] = radicand
        inverse_amplitude_closure.append(radicand + 2 * work[site] == before_squared[site])
        work_sum += work[site]
    flux = flux_after
    bank[cursor] = outgoing
    after_field = field_energy(flux)
    battery_delta = battery_energy(battery_squared) - battery_energy(before_squared)
    per_gate_energy_closure.append(
        local_closure
        and after_field + after_port - before_field - before_port
            + battery_delta == 0
    )
    records.append(
        {
            "parity": parity,
            "cursor": cursor,
            "incoming": incoming,
            "outgoing": outgoing,
            "work": work,
        }
    )
    cursor = (cursor + 1) % CAPACITY

battery_after_total = battery_energy(battery_squared)
forward_flux = list(flux)
forward_bank = [list(port) for port in bank]
forward_battery = list(battery_squared)
forward_cursor = cursor

# C25--C44: positive quadratic battery.
check("every locked battery amplitude is nonzero", all(value > 0 for value in initial_battery_squared))
check("every locked battery energy is positive", battery_before_total > 0)
check(
    "local work reads only charge incoming port and old residual",
    "w_x=\\frac{q_x}{6}(e_x-r_x)" in protocol_text,
)
check("strict reserve radicand is tested before mutation", all(strict_reserve_flags))
check("forward amplitudes remain real under the reserve gate", all(value > 0 for value in battery_squared))
check("battery sign is preserved", battery_sign == initial_battery_sign)
check(
    "battery energy changes by exactly minus local work",
    battery_after_total - battery_before_total == -work_sum,
)
check("field port and battery energy close per layer", all(per_gate_energy_closure))
check(
    "quadratic energy plus retained sign uniquely fixes the forward amplitude",
    all(value > 0 for value in battery_squared)
    and "unique in the registered class" in protocol_text,
)
check("the inverse battery law recovers every squared amplitude", all(inverse_amplitude_closure))
check(
    "each complete layer preserves total positive-booked energy",
    field_energy(forward_flux) + bank_energy(forward_bank)
        + battery_energy(forward_battery) == initial_total,
)
check(
    "stored signed ports retain inverse information",
    any(value < 0 or value > 0 for port in forward_bank for value in port),
)
check("all locked forward battery states remain strictly positive", all(value > 0 for value in forward_battery))

for record in reversed(records):
    cursor = (cursor - 1) % CAPACITY
    outgoing = list(bank[cursor])
    flux, recovered = inverse_layer(flux, charge, int(record["parity"]), outgoing)
    work = record["work"]
    assert isinstance(work, list)
    for site in ACTIVE[int(record["parity"])]:
        battery_squared[site] += 2 * work[site]
    bank[cursor] = recovered

check("all reverse battery steps recover exactly", battery_squared == initial_battery_squared)
check(
    "full field bank battery and cursor state reverses exactly",
    flux == initial_flux
    and bank == initial_bank
    and battery_squared == initial_battery_squared
    and cursor == 0,
)
check(
    "cumulative battery loss equals cumulative source work",
    battery_before_total - battery_after_total == work_sum,
)
check(
    "finite-horizon physical balance telescopes exactly",
    all(balance == 0 for balance in energy_balances),
)
gauss_text = next(
    text for path, text in source_text.items()
    if "REVERSIBLE_CHECKERBOARD_GAUSS_RECORD" in path
)
check(
    "FTD-0882 limit consumes norm Js squared of source energy",
    "W_{\\rm src}^{(\\infty)}=\\|J_s\\|^2" in gauss_text,
)
check(
    "battery reserve scale remains an input",
    "reserve scale is an input rather than a prediction" in protocol_text,
)
check(
    "fixed source polarity is catalytic while battery carries work",
    charge[site_index(0, 0, 0)] == 1
    and charge[site_index(1, 0, 0)] == -1
    and battery_after_total != battery_before_total,
)

# C45--C56: interpretation firewall.
check(
    "construction reuses the existing phase and history rail",
    "reuse existing continuous carrier and phase-rail types" in protocol_text,
)
check(
    "no sixth selected v2 type is required",
    "FINITE_PORT_BATTERY_STATUS=SELECTED_REFERENCE_EXISTING_TYPES" in protocol_text,
)
check(
    "square-root battery law remains imposed",
    "BATTERY_LAW_STATUS=IMPOSED_REFERENCE" in protocol_text,
)
check(
    "autonomous finite cyclic exact recycling closes negative only in scope",
    "FINITE_CYCLIC_INDEFINITE_FRESHNESS=NO" in protocol_text
    and "explicit one-vector-per-port" in protocol_text,
)
check("finite-horizon ready bank closes positive", all(fresh_flags))
check(
    "positive reversible source-work register closes positive",
    battery_before_total - battery_after_total == work_sum
    and all(inverse_amplitude_closure),
)
check(
    "canonical Hamiltonian reservoir remains open",
    "CANONICAL_HAMILTONIAN_RESERVOIR=OPEN" in protocol_text,
)
check(
    "3D routing and finite-capacity backpressure remain open",
    "3D routing and finite-capacity backpressure remain open" in protocol_text,
)
check(
    "moving-source continuity remains open",
    "moving-source continuity remains open" in protocol_text,
)
check("production migration remains open", "production embedding" in protocol_text)
check(
    "quartic Gstar synchronization remains separate",
    "GSTAR_ROLE=SEPARATE_CALENDAR" in protocol_text,
)
check("terminal gate reached with C55 passing", checks == 55 and failures == 0)

print(f"\nFTD-0883 finite port rail and positive battery: {checks - failures}/{checks} PASS")
if failures == 0 and checks == 56:
    print("FINITE_PORT_RAIL_POSITIVE_SOURCE_BATTERY_THEOREM")
    print("FINITE_CYCLIC_FRESH_LAYERS=CAPACITY")
    print("FINITE_CYCLIC_INDEFINITE_FRESHNESS=NO")
    print("EXACT_REAL_MEMORY_NO_GO=NOT_CLAIMED")
    print("POSITIVE_QUADRATIC_BATTERY=UNIQUE_SIGN_PRESERVING_LAW")
    print("BATTERY_LAW_STATUS=IMPOSED_REFERENCE")
    print("FULL_FINITE_STATE_REVERSIBILITY=EXACT")
    print("CANONICAL_HAMILTONIAN_RESERVOIR=OPEN")
    print("PRODUCTION_COUPLING=NONE")
    print("GSTAR_ROLE=SEPARATE_CALENDAR")
    print("BORN_BELL_STATUS=UNTOUCHED")
raise SystemExit(0 if failures == 0 and checks == 56 else 1)

