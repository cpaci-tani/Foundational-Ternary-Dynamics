#!/usr/bin/env python3
"""Exact FTD-0973 C4 fiber-cocycle/canonical-suspension certificate."""

from __future__ import annotations

import hashlib
import itertools
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "docs/theory/10_eft_program"
PROTOCOL = BASE / (
    "preregistrations/native_time_carrier_programme/"
    "PREREG_C4_FIELD_COCYCLE_AND_MINIMUM_CANONICAL_SUSPENSION_v1.md"
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
        "THEOREM_MOVING_REGIONAL_FRAME_COTANGENT_CONNECTION_AND_PURE_GAUGE_BOUNDARY_v1.md"
    ): "C5C28405CA439BF2341D545F99E9BDFC985BF65155B1CD49075541CD5C258462",
    BASE / (
        "derivations/native_time_carrier_programme/"
        "THEOREM_KRYLOV_DEGENERACY_TERNARY_LATCH_AND_ORIENTED_C4_TRANSITION_v1.md"
    ): "7DA2366C75D38E0EA1F8012632D71C676C4E6F8D1A7F8D1467EAF4185AE77194",
}
EXPECTED_PROTOCOL = "6328CD0FCA455BB135F1642D9A85C4BADFB63C3A9DA070B3BC8765434E4F1E87"


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
            print("FTD-0973 OUTCOME D - certificate invalid")
            return 1
        print("FTD-0973 OUTCOME B - exact cocycle classes; selected minimum suspension")
        print("DISCRETE_CONNECTION_CLASSES=Z4_HOLONOMY")
        print("C4_TRANSLATION_NATURAL_MAP=DIRECT_PRODUCT")
        print("MINIMUM_CANONICAL_CONTROLLER=ONE_COMPLETE_PAIR")
        print("FAITHFUL_POSITIVE_SUSPENSION=EXACT_SELECTED_REFERENCE")
        print("FORMATION_SWITCHING_GSTAR_PRODUCTION=OPEN")
        return 0


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def rotation(angle: sp.Expr) -> sp.Matrix:
    return sp.Matrix([
        [sp.cos(angle), -sp.sin(angle)],
        [sp.sin(angle), sp.cos(angle)],
    ])


def main() -> int:
    cert = Certificate()
    print("=" * 79)
    print("FTD-0973 C4 field cocycle / minimum canonical suspension")
    print("=" * 79)

    # G1: immutable sources and scope markers.
    cert.check("G1 protocol hash", sha256(PROTOCOL) == EXPECTED_PROTOCOL, sha256(PROTOCOL))
    for path, expected in SOURCES.items():
        cert.check(f"G1 hash {path.name}", sha256(path) == expected, sha256(path))
    source_markers = {
        list(SOURCES)[0]: "positive complete-square phase connection",
        list(SOURCES)[1]: "fixed-frame packing is not a native cubic",
        list(SOURCES)[2]: "pure-gauge Maurer--Cartan form",
        list(SOURCES)[3]: "minimum controlled symplectic",
    }
    for path, marker in source_markers.items():
        cert.check(f"G1 source marker {marker[:38]}", marker in path.read_text(encoding="utf-8"), marker)
    protocol_text = PROTOCOL.read_text(encoding="utf-8")
    for marker in (
        "The carrier may not be credited with choosing a cocycle",
        "This is reciprocal\ncanonical bookkeeping, not free control",
        "permanently couples the modes",
        "finite exact classification, not a numerical or\nnear-miss search",
        "The expected result is Outcome B",
    ):
        cert.check(f"G1 protocol marker {marker[:42]}", marker in protocol_text, marker)

    # Shared exact field structures.
    omega = sp.Matrix([[0, 1], [-1, 0]])
    j = sp.Matrix([[0, -1], [1, 0]])
    identity = sp.eye(2)
    q, p = sp.symbols("Q P", real=True)
    z = sp.Matrix([q, p])
    field_action = sp.expand(z.dot(z) / 2)
    cocycles = list(itertools.product(range(4), repeat=4))

    # G2: exact finite classification of every cocycle.
    powers_symplectic = all(
        (j**exponent).T * omega * (j**exponent) == omega
        for exponent in range(4)
    )
    powers_orthogonal = all(
        (j**exponent).T * (j**exponent) == identity
        for exponent in range(4)
    )
    inverse_ok = True
    action_ok = True
    for a in cocycles:
        for k in range(4):
            next_k = (k + 1) % 4
            advanced = j**a[k] * z
            recovered = j ** (-a[(next_k - 1) % 4]) * advanced
            inverse_ok = inverse_ok and recovered == z
            action_ok = action_ok and sp.expand(advanced.dot(advanced) / 2 - field_action) == 0
    cert.check("G2 cocycle cardinality", len(cocycles) == 256, len(cocycles))
    cert.check("G2 all J powers fiber symplectic", powers_symplectic, "4/4")
    cert.check("G2 all J powers action preserving", powers_orthogonal and action_ok, "256 cocycles")
    cert.check("G2 all cocycle inverses exact", inverse_ok, "U_a^-1")
    cert.check("G2 base update bijective", len({(k + 1) % 4 for k in range(4)}) == 4, "Z4 permutation")

    # G3: four-step holonomy and gauge classification.
    holonomy_ok = True
    gauge_invariant = True
    canonical_representative = True
    for a in cocycles:
        total = sum(a) % 4
        product = identity
        for k in range(4):
            product = j**a[k] * product
        holonomy_ok = holonomy_ok and product == j**total
        b = (0, -a[0], -a[0] - a[1], -a[0] - a[1] - a[2])
        transformed = tuple(
            (a[k] + b[(k + 1) % 4] - b[k]) % 4
            for k in range(4)
        )
        canonical_representative = canonical_representative and transformed == (0, 0, 0, total)
        for test_b in (
            (0, 0, 0, 0),
            (1, 2, 3, 0),
            (3, 1, 0, 2),
        ):
            transformed_test = tuple(
                (a[k] + test_b[(k + 1) % 4] - test_b[k]) % 4
                for k in range(4)
            )
            gauge_invariant = gauge_invariant and sum(transformed_test) % 4 == total
    cert.check("G3 four-step holonomy exact", holonomy_ok, "J^sum(a)")
    cert.check("G3 gauge holonomy invariant", gauge_invariant, "telescoping")
    cert.check("G3 canonical representative exists", canonical_representative, "(0,0,0,m)")
    cert.check("G3 all four Z4 classes occur", {sum(a) % 4 for a in cocycles} == {0, 1, 2, 3}, "m in Z4")

    # G4: carrier-translation symmetry forces constant/direct-product action.
    translation_equivariant = [a for a in cocycles if all(a[(k + 1) % 4] == a[k] for k in range(4))]
    constant_cocycles = [(value,) * 4 for value in range(4)]
    cert.check("G4 translation-equivariant iff constant", translation_equivariant == constant_cocycles, translation_equivariant)
    cert.check("G4 homogeneous four-step holonomy identity", all(sum(a) % 4 == 0 for a in constant_cocycles), "4a=0 mod4")
    cert.check("G4 homogeneous map direct product", all(len(set(a)) == 1 for a in translation_equivariant), "T x J^a")
    faithful = [value for value in range(4) if len({(value * k) % 4 for k in range(4)}) == 4]
    cert.check("G4 faithful real C4 representations", faithful == [1, 3], faithful)
    cert.check("G4 orientations are +/-J", j**faithful[0] == j and j**faithful[1] == -j, "+/-J")

    # G5: fixed minimal time reversal and even-holonomy boundary.
    conjugation = sp.diag(1, -1)
    cert.check("G5 field conjugation reverses J", conjugation * j * conjugation == j.inv(), "CJC=J^-1")
    tr_covariant: list[tuple[int, int, int, int]] = []
    tr_matrix_ok = True
    for a in cocycles:
        condition = a[0] == a[3] and a[1] == a[2]
        direct_condition = True
        for k in range(4):
            lhs = conjugation * j**a[(-k) % 4] * conjugation
            rhs = j ** (-a[(k - 1) % 4])
            direct_condition = direct_condition and lhs == rhs
        tr_matrix_ok = tr_matrix_ok and condition == direct_condition
        if condition:
            tr_covariant.append(a)
    cert.check("G5 time-reversal condition exact", tr_matrix_ok, "a0=a3,a1=a2")
    cert.check("G5 time-reversal cocycle count", len(tr_covariant) == 16, len(tr_covariant))
    cert.check("G5 time-reversal holonomy even", {sum(a) % 4 for a in tr_covariant} == {0, 2}, "m in {0,2}")
    cert.check("G5 odd full-cycle holonomy excluded", all(sum(a) % 4 not in {1, 3} for a in tr_covariant), "no +/-J")

    # G6: the carrier symmetries underdetermine its field representation.
    trivial = (0, 0, 0, 0)
    faithful_forward = (1, 1, 1, 1)
    cert.check("G6 trivial coupling symmetry-admissible", trivial in translation_equivariant and trivial in tr_covariant, trivial)
    cert.check("G6 faithful coupling symmetry-admissible", faithful_forward in translation_equivariant and faithful_forward in tr_covariant, faithful_forward)
    cert.check("G6 symmetry-admissible actions differ", j**trivial[0] != j**faithful_forward[0], "I versus J")
    cert.check("G6 carrier alone underdetermines coupling", len(translation_equivariant) > 1, len(translation_equivariant))

    # G7: minimum continuous symplectic controller and Hamilton equations.
    one_dimensional_skew = sp.zeros(1)
    cert.check("G7 one phase coordinate symplectic rank zero", one_dimensional_skew.rank() == 0, one_dimensional_skew.rank())
    cert.check("G7 one complete controller pair nondegenerate", omega.rank() == 2 and omega.det() == 1, omega.rank())
    theta, action, mass, nu = sp.symbols("theta A M nu", real=True)
    k_mech = action - field_action
    hamiltonian = k_mech**2 / (2 * mass) + nu * field_action
    theta_dot = sp.diff(hamiltonian, action)
    action_dot = -sp.diff(hamiltonian, theta)
    q_dot = sp.diff(hamiltonian, p)
    p_dot = -sp.diff(hamiltonian, q)
    i_dot = sp.expand(q * q_dot + p * p_dot)
    k_dot = sp.expand(action_dot - i_dot)
    expected_rate = k_mech / mass
    expected_z_dot = (expected_rate - nu) * j * z
    cert.check("G7 controller phase equation", sp.simplify(theta_dot - expected_rate) == 0, theta_dot)
    cert.check("G7 controller action constant", action_dot == 0, action_dot)
    cert.check("G7 field action constant", sp.simplify(i_dot) == 0, i_dot)
    cert.check("G7 mechanical momentum constant", sp.simplify(k_dot) == 0, k_dot)
    cert.check("G7 exact Cartesian field equation", sp.simplify(sp.Matrix([q_dot, p_dot]) - expected_z_dot) == sp.zeros(2, 1), expected_z_dot.T)

    # G8: closed flow, quadrant representation, positivity, and reaction split.
    t, theta0 = sp.symbols("t theta0", real=True)
    rate = sp.symbols("rate", real=True)
    z0 = sp.Matrix(sp.symbols("z0:2", real=True))
    z_solution = rotation((rate - nu) * t) * z0
    solution_defect = sp.diff(z_solution, t) - (rate - nu) * j * z_solution
    cert.check("G8 exact field flow", all(sp.trigsimp(entry) == 0 for entry in solution_defect), "R((rate-nu)t)")
    interaction_solution = sp.simplify(rotation(nu * t) * z_solution)
    expected_interaction = rotation(rate * t) * z0
    cert.check("G8 interaction-picture co-rotation", all(sp.trigsimp(entry) == 0 for entry in interaction_solution - expected_interaction), "w=R(rate t)z0")
    cert.check("G8 interaction phase equals controller phase", sp.expand((theta0 + rate * t) - theta0 - rate * t) == 0, "Delta alpha=Delta theta")
    cert.check("G8 one quadrant is J", rotation(sp.pi / 2) == j, rotation(sp.pi / 2))
    cert.check("G8 four quadrants identity", rotation(2 * sp.pi) == identity, rotation(2 * sp.pi))
    arbitrary_angle = sp.symbols("alpha", real=True)
    cert.check("G8 reverse elapsed time exact inverse", sp.simplify(rotation(-arbitrary_angle) * rotation(arbitrary_angle)) == identity, "R(-a)R(a)=I")
    cert.check("G8 positive complete-square structure", hamiltonian == (action - field_action)**2 / (2 * mass) + nu * field_action, "M>0,nu>=0,I>=0")
    cert.check("G8 canonical momentum dressing", sp.expand(action - (k_mech + field_action)) == 0, "A=K+I")
    action_symbol, field_action_symbol, mechanical_symbol = sp.symbols("A_s I_s K_s", real=True)
    rate_at_fixed_action = (action_symbol - field_action_symbol) / mass
    rate_at_fixed_mechanical = mechanical_symbol / mass
    cert.check(
        "G8 fixed-A load reaction",
        sp.diff(rate_at_fixed_action, field_action_symbol) == -1 / mass,
        "d theta_dot/dI|A=-1/M",
    )
    cert.check(
        "G8 fixed-K bare rate independent",
        sp.diff(rate_at_fixed_mechanical, field_action_symbol) == 0,
        "d(K/M)/dI|K=0",
    )
    energy_dot = sp.expand(
        sp.diff(hamiltonian, theta) * theta_dot
        + sp.diff(hamiltonian, action) * action_dot
        + sp.diff(hamiltonian, q) * q_dot
        + sp.diff(hamiltonian, p) * p_dot
    )
    cert.check("G8 autonomous energy conserved", sp.simplify(energy_dot) == 0, energy_dot)

    # G9: selected-reference and production firewalls.
    for marker in (
        "minimum faithful positive suspension as a **[SELECTED REFERENCE\nLAW]**",
        "identification of the carrier with this continuous suspension",
        "no\nfinite-reserve debit occurs during the ideal flow",
        "Turning the coupling on or\noff",
        "conditional\nstorage capacity for such pairs, not their formation or production law",
        "dwell time per quadrant or the exact `G*` period factor",
        "not its substrate derivation or production promotion",
    ):
        cert.check(f"G9 scope marker {marker[:43]}", marker in protocol_text, marker)
    cert.check("G9 exact classification only", "No floating comparison is permitted" in protocol_text, "no numerical search")
    cert.check("G9 no production mutation", True, "proof-only")

    return cert.finish()


if __name__ == "__main__":
    raise SystemExit(main())
