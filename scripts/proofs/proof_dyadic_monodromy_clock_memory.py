"""Exact certificate for a dyadic monodromy clock with sheet memory.

This verifier separates two dyadic constructions that are easy to conflate.

High-harmonic tower::

    h_j = z_0 ** (2**j)

Every high harmonic is a function of the carrier.  Adding one therefore adds
no independent state.

Compatible root tower::

    z_j = z_(j+1) ** 2

For fixed ``z_0``, depth ``K`` has ``2**K`` compatible lifts.  Analytic
continuation through one carrier loop sends sheet ``m`` to ``m + 1`` modulo
``2**K``.  Consequently the lifted depth-K state has minimal recurrence
``2**K`` carrier loops, and each new root level both doubles the recurrence
horizon and adds one sheet bit.

Those sheet bits encode epoch modulo ``2**K``.  Because carrier monodromy is a
single transitive cycle, one tower cannot also preserve a nonconstant payload:
written sheet and elapsed cycles occur only through their sum.  The verifier
therefore includes both that exact no-go and a selected two-tower repair, in
which a payload lift is compared with a separately co-transported reference
lift.

The root tower is a deliberately selected mathematical model.  This script
does not establish that the unmodified FTD substrate creates, protects, or
reads such modes.  The causal fast-mode ceiling is conditional on additionally
requiring a spatially supported mode to resolve a declared number of causal
intervals per cycle.  No fitted constants, empirical targets, or numerical
near-miss searches occur here.
"""

from __future__ import annotations

import argparse
import csv
import json
from dataclasses import dataclass
from itertools import product
from math import gcd
from pathlib import Path

import sympy as sp


MAX_CERTIFIED_DEPTH = 10


@dataclass(frozen=True)
class Gate:
    """One deterministic certificate row."""

    status: str
    name: str
    detail: str
    passed: bool


def high_harmonic_exponents(depth: int) -> tuple[int, ...]:
    """Return the exponents in ``(z_0, z_0^2, ..., z_0^(2^depth))``."""

    return tuple(2**j for j in range(depth + 1))


def root_sheet_signature(depth: int, sheet: int) -> tuple[int, ...]:
    """Encode a compatible lift by its residues at root levels 1..depth.

    At level ``j`` the phase is

        (theta + 2*pi*(sheet mod 2**j)) / 2**j.

    Forgetting the last residue is exactly the covering projection from depth
    ``j`` to depth ``j-1``.
    """

    modulus = 2**depth
    reduced_sheet = sheet % modulus
    return tuple(reduced_sheet % (2**j) for j in range(1, depth + 1))


def monodromy(depth: int, sheet: int, carrier_loops: int = 1) -> int:
    """Apply carrier-loop monodromy to a depth-K sheet address."""

    return (sheet + carrier_loops) % (2**depth)


def root_of_unity_order(exponent: int, modulus: int) -> int:
    """Return the exact order of ``exp(2*pi*i*exponent/modulus)``."""

    return modulus // gcd(exponent, modulus)


def gate_high_harmonics() -> Gate:
    z = sp.symbols("z")
    passed = True
    for depth in range(MAX_CERTIFIED_DEPTH + 1):
        exponents = high_harmonic_exponents(depth)
        modes = tuple(z**exponent for exponent in exponents)
        passed &= exponents[0] == 1
        passed &= all(
            sp.expand(modes[j + 1] - modes[j] ** 2) == 0
            for j in range(depth)
        )
        # On every finite carrier Z/PZ, adding deterministic powers leaves the
        # joint state count exactly P because the first coordinate is q itself.
        # This is an exhaustive finite shadow of the fiber-cardinality-one
        # statement for z -> (z,z^2,...,z^(2^K)).
        for period in range(1, 65):
            signatures = {
                tuple((exponent * phase) % period for exponent in exponents)
                for phase in range(period)
            }
            passed &= len(signatures) == period
    return Gate(
        "[EXACT]",
        "high harmonics add no independent state",
        "h_j=z_0^(2^j); joint capacity equals the carrier capacity",
        bool(passed),
    )


def gate_root_sheet_count() -> Gate:
    passed = True
    for depth in range(MAX_CERTIFIED_DEPTH + 1):
        signatures = {
            root_sheet_signature(depth, sheet) for sheet in range(2**depth)
        }
        passed &= len(signatures) == 2**depth
        if depth > 0:
            for sheet in range(2**depth):
                passed &= root_sheet_signature(depth, sheet)[:-1] == (
                    root_sheet_signature(depth - 1, sheet)
                )
    return Gate(
        "[EXACT | SELECTED MODEL]",
        "compatible root-tower sheet count",
        "depth K has exactly 2^K lifts over one carrier phase",
        bool(passed),
    )


def gate_monodromy() -> Gate:
    passed = all(
        monodromy(depth, sheet) == (sheet + 1) % (2**depth)
        for depth in range(MAX_CERTIFIED_DEPTH + 1)
        for sheet in range(2**depth)
    )
    return Gate(
        "[EXACT | SELECTED MODEL]",
        "carrier-loop monodromy",
        "theta -> theta+2*pi sends sheet m -> m+1 mod 2^K",
        passed,
    )


def gate_minimal_recurrence() -> Gate:
    passed = True
    for depth in range(MAX_CERTIFIED_DEPTH + 1):
        period = 2**depth
        for sheet in range(period):
            passed &= monodromy(depth, sheet, period) == sheet
            passed &= all(
                monodromy(depth, sheet, loops) != sheet
                for loops in range(1, period)
            )
    return Gate(
        "[EXACT | SELECTED MODEL]",
        "depth-K minimal recurrence",
        "the lifted state first returns after exactly 2^K carrier loops",
        bool(passed),
    )


def gate_root_level_doubling() -> Gate:
    passed = all(
        2 ** (depth + 1) == 2 * 2**depth
        and len(
            {
                sheet
                for sheet in range(2 ** (depth + 1))
                if sheet % (2**depth) == old_sheet
            }
        )
        == 2
        for depth in range(MAX_CERTIFIED_DEPTH)
        for old_sheet in range(2**depth)
    )
    return Gate(
        "[EXACT | SELECTED MODEL]",
        "root-level doubling",
        "each old sheet has two lifts and P_(K+1)=2 P_K",
        passed,
    )


def gate_branch_word_bijection() -> Gate:
    """Verify that K sequential binary root choices address the 2^K sheets."""

    passed = True
    for depth in range(MAX_CERTIFIED_DEPTH + 1):
        words = tuple(product((0, 1), repeat=depth))
        sheets = {
            sum(bit << level for level, bit in enumerate(word)) for word in words
        }
        passed &= len(words) == 2**depth
        passed &= sheets == set(range(2**depth))
    return Gate(
        "[EXACT | SELECTED MODEL]",
        "branch-word/sheet bijection",
        "K sequential root choices address K synchronized sheet bits",
        bool(passed),
    )


def gate_prefix_capacity() -> Gate:
    passed = all(
        sum(2**depth for depth in range(max_depth + 1))
        == 2 ** (max_depth + 1) - 1
        for max_depth in range(MAX_CERTIFIED_DEPTH + 1)
    )
    return Gate(
        "[EXACT | SELECTED MODEL]",
        "variable-depth prefix capacity",
        "disjoint depths 0..M contain sum 2^K=2^(M+1)-1 states",
        passed,
    )


def gate_ternary_capacity() -> Gate:
    passed = all(
        len(set(product((-1, 0, 1), repeat=mode_count))) == 3**mode_count
        for mode_count in range(0, 8)
    )
    return Gate(
        "[EXACT]",
        "independent ternary mode capacity",
        "M independent coefficients in {-1,0,+1} have 3^M words",
        passed,
    )


def gate_permanent_write_obstruction() -> Gate:
    depth_start, writes = sp.symbols(
        "K_start writes", integer=True, nonnegative=True
    )
    depth_end = depth_start + writes
    only_recurrent_write_count = sp.solve(
        sp.Eq(depth_end, depth_start), writes, domain=sp.S.Integers
    )
    symbolic_pass = only_recurrent_write_count == [0]

    # Synthetic append-only histories independently check the rank argument:
    # if any write occurs, the full state cannot equal its earlier state because
    # its depth coordinate has changed.
    history_pass = True
    for start in range(6):
        for write_count in range(6):
            can_recur = (start + write_count) == start
            history_pass &= can_recur == (write_count == 0)
    return Gate(
        "[EXACT | SELECTED MODEL]",
        "permanent writes obstruct full-state recurrence",
        "K_end=K_start+W equals K_start iff W=0",
        bool(symbolic_pass and history_pass),
    )


def gate_single_tower_payload_no_go() -> Gate:
    """Show that one transitive tower cannot hold a cycle-invariant payload."""

    passed = True
    for depth in range(MAX_CERTIFIED_DEPTH + 1):
        modulus = 2**depth
        orbit = tuple(monodromy(depth, 0, loops) for loops in range(modulus))
        passed &= set(orbit) == set(range(modulus))
        passed &= len(set(orbit)) == modulus
        if depth > 0:
            # Written sheet 0 after one cycle is observationally identical to
            # written sheet 1 after zero cycles.  In general only m+n is seen.
            passed &= monodromy(depth, 0, 1) == monodromy(depth, 1, 0)

        # Propagating equality R(m+1)=R(m) around this single orbit forces all
        # values to equal R(0); there is only one monodromy orbit class.
        invariant_labels = [None] * modulus
        invariant_labels[0] = 0
        for loops in range(1, modulus):
            invariant_labels[orbit[loops]] = invariant_labels[orbit[loops - 1]]
        passed &= set(invariant_labels) == {0}
    return Gate(
        "[EXACT NO-GO | SELECTED MODEL]",
        "single-tower payload/epoch confounding",
        "m_observed=m_written+n mod 2^K; invariant payloads are constant",
        bool(passed),
    )


def gate_relational_reference_repair() -> Gate:
    """Verify the selected two-tower repair for a stable relative payload."""

    reference_symbol, payload_symbol, loops_symbol = sp.symbols(
        "r u n", integer=True
    )
    passed = sp.expand(
        (payload_symbol + loops_symbol)
        - (reference_symbol + loops_symbol)
        - (payload_symbol - reference_symbol)
    ) == 0
    for depth in range(8):
        modulus = 2**depth
        for reference in range(modulus):
            for payload in range(modulus):
                relative = (payload - reference) % modulus
                for loops in {0, 1, modulus - 1, modulus, modulus + 1}:
                    transported_reference = monodromy(depth, reference, loops)
                    transported_payload = monodromy(depth, payload, loops)
                    passed &= (
                        transported_payload - transported_reference
                    ) % modulus == relative
    return Gate(
        "[EXACT | SELECTED TWO-TOWER MODEL]",
        "relational payload repair",
        "co-transported payload/reference lifts preserve (u-r) mod 2^K",
        bool(passed),
    )


def gate_quartic_composition() -> Gate:
    energy, rho, g_star = sp.symbols("E rho G_star", positive=True)
    depth = sp.symbols("K", integer=True, nonnegative=True)
    quartic_period = (
        sp.sqrt(sp.pi)
        * g_star
        * (2 * energy) ** sp.Rational(-1, 4)
        / rho
    )
    lifted_period = 2**depth * quartic_period
    passed = sp.simplify(lifted_period / quartic_period - 2**depth) == 0
    return Gate(
        "[EXACT | SELECTED MODEL]",
        "quartic-carrier/root-tower composition",
        "T_4=sqrt(pi)G*(2E)^(-1/4)/rho implies T_K=2^K T_4",
        bool(passed),
    )


def gate_root_lift_occupancy_boundary() -> Gate:
    """A uniform root phase retains arcsine, not quartic, occupancy."""

    # For x=cos(theta) under uniform phase, E[x^(2n)] = C(2n,n)/4^n.
    arcsine_second = sp.binomial(2, 1) / 4
    arcsine_fourth = sp.binomial(4, 2) / 4**2
    quartic_fourth = sp.Rational(1, 3)
    passed = (
        arcsine_second == sp.Rational(1, 2)
        and arcsine_fourth == sp.Rational(3, 8)
        and arcsine_fourth != quartic_fourth
    )
    return Gate(
        "[EXACT BOUNDARY | SELECTED MODEL]",
        "root lifts do not manufacture quartic occupancy",
        "uniform Re(z_K) has <x^4>=3/8, not the quartic value 1/3",
        bool(passed),
    )


def gate_floquet_orders() -> Gate:
    # After one carrier loop, root level j is multiplied by
    # mu_j=exp(2*pi*i/2^j).  The exact order of zeta_N^a is N/gcd(a,N).
    passed = all(
        root_of_unity_order(1, 2**level) == 2**level
        for level in range(MAX_CERTIFIED_DEPTH + 1)
    )
    return Gate(
        "[EXACT | SELECTED MODEL]",
        "root-level Floquet orders",
        "mu_j=exp(2*pi*i/2^j) has exact order 2^j",
        passed,
    )


def gate_discrete_tick_commensurability() -> Gate:
    # If the carrier itself takes P_0 integer substrate ticks, the lifted
    # depth-K recurrence takes 2^K P_0 ticks.  Without rational/integer carrier
    # alignment, continuous closure alone supplies no exact sampled recurrence.
    passed = all(
        2**depth * base_ticks == base_ticks * 2**depth
        for depth in range(MAX_CERTIFIED_DEPTH + 1)
        for base_ticks in range(1, 17)
    )
    return Gate(
        "[CONDITIONAL | SELECTED MODEL]",
        "discrete-tick recurrence hierarchy",
        "if P_0 is an integer-tick carrier period, P_K=2^K P_0 ticks",
        passed,
    )


def gate_conditional_causal_ceiling() -> Gate:
    energy, rho, speed_ratio, g_star = sp.symbols(
        "E rho u G_star", positive=True
    )
    intervals_per_cycle = sp.symbols("nu", positive=True, integer=True)
    level = sp.symbols("k", integer=True, nonnegative=True)

    # FTD-0771's conditional edge/clock fraction.  A forward high harmonic has
    # period T_4/2^k, hence consumes 2^k times as much phase per causal edge.
    d_4 = (
        rho
        * (2 * energy) ** sp.Rational(1, 4)
        / (speed_ratio * sp.sqrt(sp.pi) * g_star)
    )
    d_fast = 2**level * d_4
    ceiling_ratio = sp.simplify(1 / (intervals_per_cycle * d_4))
    expected_ratio = (
        speed_ratio
        * sp.sqrt(sp.pi)
        * g_star
        / (
            intervals_per_cycle
            * rho
            * (2 * energy) ** sp.Rational(1, 4)
        )
    )
    symbolic_pass = sp.simplify(ceiling_ratio - expected_ratio) == 0
    symbolic_pass &= (
        sp.simplify(
            d_fast
            * speed_ratio
            * sp.sqrt(sp.pi)
            * g_star
            / (rho * (2 * energy) ** sp.Rational(1, 4))
            - 2**level
        )
        == 0
    )

    # Exact synthetic boundary: d_4=1/16 and nu=2 admit k<=3, while k=4
    # is the first rejected fast harmonic.  These rationals test the inequality;
    # they are not asserted FTD values.
    synthetic_d4 = sp.Rational(1, 16)
    synthetic_nu = 2
    admissible = [
        k
        for k in range(8)
        if synthetic_nu * 2**k * synthetic_d4 <= 1
    ]
    synthetic_pass = admissible == [0, 1, 2, 3]

    # The inverse/root tower moves in the opposite direction:
    # d_root,j=d_4/2^j.  Adding memory levels therefore relaxes rather than
    # tightens this fast-mode ceiling.
    root_direction_pass = all(
        synthetic_d4 / 2 ** (j + 1) < synthetic_d4 / 2**j
        for j in range(8)
    )
    return Gate(
        "[CONDITIONAL | SELECTED MODEL]",
        "causal ceiling for forward high harmonics",
        "nu*2^k*d_4<=1, so 2^k<=u*sqrt(pi)G*/[nu*rho*(2E)^(1/4)]",
        bool(symbolic_pass and synthetic_pass and root_direction_pass),
    )


def build_gate_table() -> tuple[Gate, ...]:
    """Build the certificate in a stable, human-auditable order."""

    return (
        gate_high_harmonics(),
        gate_root_sheet_count(),
        gate_monodromy(),
        gate_minimal_recurrence(),
        gate_root_level_doubling(),
        gate_branch_word_bijection(),
        gate_prefix_capacity(),
        gate_ternary_capacity(),
        gate_permanent_write_obstruction(),
        gate_single_tower_payload_no_go(),
        gate_relational_reference_repair(),
        gate_quartic_composition(),
        gate_root_lift_occupancy_boundary(),
        gate_floquet_orders(),
        gate_discrete_tick_commensurability(),
        gate_conditional_causal_ceiling(),
    )


def write_artifacts(gates: tuple[Gate, ...], output_dir: Path) -> None:
    """Write deterministic audit artifacts for the selected synthetic model."""

    output_dir.mkdir(parents=True, exist_ok=True)

    with (output_dir / "gate_table.csv").open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(
            handle,
            fieldnames=("result", "epistemic_status", "gate", "statement"),
        )
        writer.writeheader()
        for gate in gates:
            writer.writerow(
                {
                    "result": "PASS" if gate.passed else "FAIL",
                    "epistemic_status": gate.status,
                    "gate": gate.name,
                    "statement": gate.detail,
                }
            )

    demo_depth = 4
    with (output_dir / "monodromy_strobe.csv").open(
        "w", newline="", encoding="utf-8"
    ) as handle:
        writer = csv.DictWriter(
            handle,
            fieldnames=("carrier_cycles", "base_phase", "sheet", "full_return"),
        )
        writer.writeheader()
        for loops in range(2**demo_depth + 1):
            writer.writerow(
                {
                    "carrier_cycles": loops,
                    "base_phase": 0,
                    "sheet": monodromy(demo_depth, 0, loops),
                    "full_return": loops > 0
                    and monodromy(demo_depth, 0, loops) == 0,
                }
            )

    with (output_dir / "branch_words.csv").open(
        "w", newline="", encoding="utf-8"
    ) as handle:
        writer = csv.DictWriter(handle, fieldnames=("branch_word", "sheet"))
        writer.writeheader()
        for word in product((0, 1), repeat=demo_depth):
            writer.writerow(
                {
                    "branch_word": "".join(str(bit) for bit in word),
                    "sheet": sum(bit << level for level, bit in enumerate(word)),
                }
            )

    summary = {
        "certificate": "DYADIC_MONODROMY_CLOCK_MEMORY",
        "result": "PASS" if all(gate.passed for gate in gates) else "FAIL",
        "passed": sum(gate.passed for gate in gates),
        "total": len(gates),
        "synthetic_demo_depth": demo_depth,
        "synthetic_full_return_carrier_cycles": 2**demo_depth,
        "scope": {
            "finite_cover_math": "EXACT",
            "root_tower": "SELECTED MODEL",
            "single_tower_payload": "EXACT NO-GO",
            "two_tower_payload_repair": "SELECTED MODEL",
            "causal_ceiling": "CONDITIONAL",
            "native_ftd_bridge": "OPEN",
        },
    }
    with (output_dir / "summary.json").open("w", encoding="utf-8") as handle:
        json.dump(summary, handle, indent=2, sort_keys=True)
        handle.write("\n")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output-dir",
        type=Path,
        help="optional directory for deterministic CSV/JSON audit artifacts",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    gates = build_gate_table()
    print("Dyadic monodromy clock-memory exact certificate")
    print("result | epistemic status | gate | exact statement")
    print("--- | --- | --- | ---")
    for gate in gates:
        result = "PASS" if gate.passed else "FAIL"
        print(f"{result} | {gate.status} | {gate.name} | {gate.detail}")

    passed = sum(gate.passed for gate in gates)
    assert passed == len(gates), f"expected {len(gates)} gates, passed {passed}"
    print(f"DYADIC_MONODROMY_CLOCK_MEMORY: {passed}/{len(gates)} PASS")
    if args.output_dir is not None:
        write_artifacts(gates, args.output_dir)
        print(f"ARTIFACTS: {args.output_dir.resolve()}")
    print("[SELECTED MODEL] The compatible square-root tower is chosen, not derived from P1--P5.")
    print("[EXACT NO-GO] One tower confounds written payload with elapsed carrier cycles.")
    print("[SELECTED MODEL] A stable payload needs a separately co-transported reference lift.")
    print("[CONDITIONAL] The causal ceiling requires spatial support and a declared resolution gate.")
    print("[OPEN] Native FTD mode birth, persistence, readout, and energy closure are not established.")


if __name__ == "__main__":
    main()
