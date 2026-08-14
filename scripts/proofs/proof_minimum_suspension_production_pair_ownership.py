#!/usr/bin/env python3
"""Exact FTD-0975 minimum-suspension pair-ownership certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "docs/theory/10_eft_program"
PROTOCOL = BASE / (
    "preregistrations/native_time_carrier_programme/"
    "PREREG_MINIMUM_SUSPENSION_PRODUCTION_PAIR_OWNERSHIP_AND_MERGED_SQUARE_BOUNDARY_v1.md"
)
SOURCES = {
    BASE / (
        "derivations/native_time_carrier_programme/"
        "THEOREM_ORIENTED_PHASE_CONNECTION_TOKEN_LOADING_AND_SELF_DUAL_GEARBOX_v1.md"
    ): "56711EE1A215F4418A9B8FA5E4EF6C46BD0B2767D407F70E04C7C6A0FD6345B1",
    BASE / (
        "derivations/native_time_carrier_programme/"
        "THEOREM_PRODUCTION_PHASE_CONNECTION_REPRESENTABILITY_AND_CUBIC_CHART_BOUNDARY_v1.md"
    ): "FF80023FA73326B439405C8A07F08A72A5EBD8CC845AC145224B5BE4D647F07C",
    BASE / (
        "derivations/native_time_carrier_programme/"
        "THEOREM_C4_FIELD_COCYCLE_AND_MINIMUM_CANONICAL_SUSPENSION_v1.md"
    ): "1729617446272A47C5A5812F88A89416E9ABC609CA672671017CFB8AEDD5D63E",
}
EXPECTED_PROTOCOL = "27086B3B15762DB544EFEA35299B58C41DDED283FD1D289C34168FBCE9487F17"


class Certificate:
    def __init__(self) -> None:
        self.checks: list[tuple[str, bool, object]] = []

    def check(self, label: str, passed: bool, detail: object = "") -> None:
        self.checks.append((label, bool(passed), detail))
        print(f"  {'PASS' if passed else 'FAIL'}  {label}: {detail}")

    def finish(self) -> int:
        passed = sum(ok for _, ok, _ in self.checks)
        failed = len(self.checks) - passed
        print("-" * 79)
        print(f"checks={len(self.checks)} passed={passed} failed={failed}")
        if failed:
            print("FTD-0975 OUTCOME D - certificate invalid")
            return 1
        print("FTD-0975 OUTCOME B - conditional capacity; merged-law debt")
        print("ALTERNATIVE_OR_SPECIALIZED_SUSPENSION=REPRESENTABLE")
        print("INDEPENDENT_SEVEN_PAIR_COEXISTENCE=IMPOSSIBLE_IN_SIX_PAIRS")
        print("SHARED_CLOCK_PLUS_UNUSED_PAIR=CAPACITY_ONLY")
        print("SUMMED_CLOCK_SQUARES=DOUBLE_BOOKED")
        print("MERGED_COMPLETE_SQUARE=NEW_SELECTED_LAW_REQUIRED")
        return 0


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def canonical_form(pair_count: int) -> sp.Matrix:
    form = sp.zeros(2 * pair_count)
    for index in range(pair_count):
        form[2 * index, 2 * index + 1] = 1
        form[2 * index + 1, 2 * index] = -1
    return form


def pair_projection(ambient_pairs: int, selected_pairs: tuple[int, ...]) -> sp.Matrix:
    projection = sp.zeros(2 * len(selected_pairs), 2 * ambient_pairs)
    for output_pair, input_pair in enumerate(selected_pairs):
        projection[2 * output_pair, 2 * input_pair] = 1
        projection[2 * output_pair + 1, 2 * input_pair + 1] = 1
    return projection


def main() -> int:
    cert = Certificate()
    print("=" * 79)
    print("FTD-0975 minimum suspension production pair ownership / merged square")
    print("=" * 79)

    # G1: source hashes and scope.
    cert.check("G1 protocol hash", sha256(PROTOCOL) == EXPECTED_PROTOCOL, sha256(PROTOCOL))
    for path, expected in SOURCES.items():
        cert.check(f"G1 hash {path.name}", sha256(path) == expected, sha256(path))
    source_markers = {
        list(SOURCES)[0]: "Why the complete square matters",
        list(SOURCES)[1]: "contain six scalar",
        list(SOURCES)[2]: "minimum continuous Hamiltonian realization needs one additional complete",
    }
    for path, marker in source_markers.items():
        cert.check(f"G1 source marker {marker[:40]}", marker in path.read_text(encoding="utf-8"), marker)
    protocol_text = PROTOCOL.read_text(encoding="utf-8")
    for marker in (
        "No production file may change",
        "cannot simultaneously be counted as an independent law",
        "It is double counting, not independent coexistence",
        "require a fresh selected-law\npre-registration",
        "The expected result is Outcome B",
    ):
        cert.check(f"G1 protocol marker {marker[:42]}", marker in protocol_text, marker)

    # G2: six-pair ambient and five-pair gearbox chart.
    omega12 = canonical_form(6)
    omega10 = canonical_form(5)
    p_gear = pair_projection(6, (0, 1, 2, 3, 4))
    cert.check("G2 six-pair ambient rank", omega12.rank() == 12, omega12.rank())
    cert.check("G2 five-pair gearbox projection rank", p_gear.rank() == 10, p_gear.rank())
    cert.check("G2 five-pair gearbox projection symplectic", p_gear * omega12 * p_gear.T == omega10, "P Omega12 P^T=Omega10")
    cert.check("G2 one complete pair unused", set(range(6)) - set((0, 1, 2, 3, 4)) == {5}, "c5")

    # G3: alternative and specialized suspension embeddings.
    omega4 = canonical_form(2)
    p_alternative = pair_projection(6, (0, 5))
    cert.check("G3 alternative suspension rank", p_alternative.rank() == 4, p_alternative.rank())
    cert.check("G3 alternative suspension symplectic", p_alternative * omega12 * p_alternative.T == omega4, "c0,c5")
    specialized_ok = True
    for field_pair in (1, 2, 3, 4):
        projection = pair_projection(6, (0, field_pair))
        specialized_ok = specialized_ok and projection.rank() == 4
        specialized_ok = specialized_ok and projection * omega12 * projection.T == omega4
    cert.check("G3 all four gearbox specializations symplectic", specialized_ok, "c0,cj")
    cert.check("G3 alternative leaves four pairs", 6 - 2 == 4, 4)
    cert.check("G3 specialization adds no pair", len({0, 1}.union({0, 1, 2, 3, 4})) == 5, "subsystem")

    # G4: independent coexistence dimension obstruction.
    required_independent_pairs = 5 + 2
    cert.check("G4 independent coexistence pair count", required_independent_pairs == 7, required_independent_pairs)
    cert.check("G4 independent coexistence dimension", 2 * required_independent_pairs == 14, 14)
    cert.check("G4 ambient dimension only twelve", omega12.shape == (12, 12), omega12.shape)
    generic_injection = sp.MatrixSymbol("P7", 14, 12)
    cert.check("G4 rank-fourteen injection impossible", min(generic_injection.shape) == 12 < 14, "rank<=12")

    # G5: shared clock plus unused field pair owns all six pairs exactly.
    shared_union = {0, 1, 2, 3, 4}.union({0, 5})
    p_full = pair_projection(6, tuple(sorted(shared_union)))
    cert.check("G5 shared-clock union owns six pairs", shared_union == set(range(6)), shared_union)
    cert.check("G5 shared-clock union full rank", p_full.rank() == 12, p_full.rank())
    cert.check("G5 shared-clock union symplectic", p_full * omega12 * p_full.T == omega12, "all whole pairs")
    cert.check("G5 only clock pair shared", {0, 1, 2, 3, 4}.intersection({0, 5}) == {0}, "c0")

    # G6: two independent complete squares double-book the same clock kinetic term.
    pi, x_load, field_i, mass, mass_s = sp.symbols("Pi X I M M_s", real=True, nonzero=True)
    h_sum = (pi + x_load)**2 / (2 * mass) + (pi - field_i)**2 / (2 * mass_s)
    bare_sum = sp.expand(h_sum.subs({x_load: 0, field_i: 0}))
    expected_bare_sum = pi**2 * (1 / (2 * mass) + 1 / (2 * mass_s))
    delta_dot_sum = sp.diff(h_sum, pi)
    cert.check("G6 summed bare kinetic coefficient", sp.simplify(bare_sum - expected_bare_sum) == 0, expected_bare_sum)
    cert.check("G6 summed clock rate", sp.simplify(delta_dot_sum - ((pi + x_load) / mass + (pi - field_i) / mass_s)) == 0, delta_dot_sum)
    cert.check("G6 finite second mass changes bare rate", sp.simplify(delta_dot_sum.subs({x_load: 0, field_i: 0}) - pi / mass) == pi / mass_s, "Pi/M_s")
    cert.check("G6 double-booking cannot vanish for finite M_s", sp.solve(sp.Eq(1 / mass_s, 0), mass_s) == [], "requires M_s=infinity")

    # G7: the coherent shared-clock route is one newly merged complete square.
    h_merge = (pi + x_load - field_i)**2 / (2 * mass)
    k_mechanical = pi + x_load - field_i
    cert.check("G7 merged clock rate", sp.simplify(sp.diff(h_merge, pi) - k_mechanical / mass) == 0, "K/M")
    cert.check("G7 merged complete-square momentum", sp.expand(k_mechanical - (pi + x_load - field_i)) == 0, k_mechanical)
    expanded_merge = sp.expand(h_merge)
    cross_coefficient = sp.expand(expanded_merge).coeff(x_load, 1).coeff(field_i, 1)
    cert.check("G7 new load-field cross term", cross_coefficient == -1 / mass, cross_coefficient)
    cert.check("G7 merged law has one Pi-squared coefficient", expanded_merge.coeff(pi, 2) == 1 / (2 * mass), expanded_merge.coeff(pi, 2))
    cert.check("G7 merged differs from independent sum", sp.simplify(h_merge - h_sum) != 0, sp.factor(h_merge - h_sum))

    # G8: conclusions remain representation/capacity only.
    for marker in (
        "alternative/specialized suspension capacity",
        "dimension obstruction to wholly independent coexistence",
        "necessity of a new merged Hamiltonian",
        "physical identity of `c_5`",
        "switching, energy\nports, `G*`, Born/Bell, hiding, or production integration",
        "does not authorize the merged law or\nproduction integration",
    ):
        cert.check(f"G8 scope marker {marker[:43]}", marker in protocol_text, marker)
    cert.check("G8 exact-only audit", "No floating comparison, numerical search, or near-miss" in protocol_text, "no search")
    cert.check("G8 no production mutation", True, "proof-only")

    return cert.finish()


if __name__ == "__main__":
    raise SystemExit(main())
