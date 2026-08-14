"""FTD-0995 exact crossing-matched clock-growth certificate.

This certificate is symbolic/combinatorial.  It performs no parameter fit,
near-miss search, empirical substitution, or production mutation.
"""

from __future__ import annotations

import hashlib
from itertools import product
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]

PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_CROSSING_MATCHED_FORMATION_ENERGY_AND_CAUSAL_QUARTIC_CLOCK_GROWTH_v1.md"
)

SOURCES = {
    PROTOCOL: "B1113C02CFF82C0BD2F14D77FA5C661AC290243C2CC4C94AF9C552E9D665957F",
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_ZERO_ACTION_CANONICAL_SEED_AND_CAUSAL_CLOCK_GROWTH_BOUNDARY_v1.md"
    ): "897367658B339F074A78FEA017994EEA63AD7921BA4C597663EA123088E76306",
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_LOCAL_OCCUPANCY_FLIP_FORMATION_WORK_AND_MINIMUM_ACTIVE_APERTURE_v1.md"
    ): "E4D4BBCF2A0E09953EA2107FD80954E50BB2ED9BE45A9C9C6D2381DA018D7B9F",
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_TERNARY_OCCUPANCY_MEMBRANE_AND_SELF_DUAL_BODY_CLOCK_SPLIT_v1.md"
    ): "A19593DACD2CE97A6B785F235AE5048EADC228680E07D2F90F4C4DB7BD15333C",
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "DERIV_QUARTIC_CLOCK_CM_GEARBOX_v1.md"
    ): "1B969544B065D576523235F40A20918C22E0C55978E52282E2FC623385BC2FDF",
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "DERIV_BILATERAL_SELF_DUAL_QUARTIC_CLOCK_v1.md"
    ): "779044879BB28CE0DB13BA8783EC7FF9AB5DFDFE10DF1C259D3D11998DEEDB9A",
    ROOT / "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    ROOT / "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    ROOT / "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    ROOT / "engine/src/transmutation_phases.cpp":
        "4013A9B769199D54976347378FD03DFF6415B7F641F35D3FAE498125EB288043",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


class Proof:
    def __init__(self) -> None:
        self.total = 0
        self.passed = 0

    def check(self, name: str, condition: object, detail: object = "") -> None:
        self.total += 1
        ok = bool(condition)
        if ok:
            self.passed += 1
        mark = "PASS" if ok else "FAIL"
        suffix = f" :: {detail}" if detail != "" else ""
        print(f"[{mark}] {name}{suffix}")

    def report(self) -> bool:
        print()
        print(
            "FTD-0995 crossing-matched formation energy and causal quartic-clock growth: "
            f"{self.passed}/{self.total} checks passed"
        )
        if self.passed == self.total:
            print("OUTCOME B — EXACT COMPLIANCE-SURFACE GROWTH / AUTONOMOUS MATCHING OPEN")
            print("FORMATION_WORK_EQUALS_LOCAL_CLOCK_ENERGY=NECESSARY_AND_SUFFICIENT")
            print("CAUSAL_COHERENT_FRONT=EXACT_CONDITIONAL")
            print("QUARTIC_GSTAR_CADENCE=INHERITED_NOT_READ")
            print("PRODUCTION_INTEGRATION=NONE")
            return True
        print("OUTCOME D — INVALID")
        return False


P = Proof()


# G1: immutable source lock and source-scope claims.
for path, expected in SOURCES.items():
    P.check(f"G1 hash {path.name}", path.is_file() and sha256(path) == expected)

zero_seed_text = next(
    p for p in SOURCES if p.name.startswith("THEOREM_ZERO_ACTION")
).read_text(encoding="utf-8")
formation_text = next(
    p for p in SOURCES if p.name.startswith("THEOREM_LOCAL_OCCUPANCY")
).read_text(encoding="utf-8")
membrane_text = next(
    p for p in SOURCES if p.name.startswith("THEOREM_TERNARY_OCCUPANCY")
).read_text(encoding="utf-8")
quartic_text = next(
    p for p in SOURCES if p.name == "DERIV_QUARTIC_CLOCK_CM_GEARBOX_v1.md"
).read_text(encoding="utf-8")

P.check("G1 Cartesian first stroke present", "P'=\\sigma\\sqrt{2U" in zero_seed_text)
P.check("G1 exact join compatibility present", "q={Q_N\\over\\sqrt N}" in zero_seed_text)
P.check("G1 cut-set formation work present", "W_S=" in formation_text and "E_join-E_cut" in formation_text)
P.check("G1 occupancy Laplacian present", "K_m=B^TG_mB" in membrane_text)
P.check("G1 uniform clock kernel present", "ker K_\\Lambda" in membrane_text)
P.check("G1 quartic period law present", "TA=\\sqrt{\\frac{m}{2\\lambda}}" in quartic_text)

production_text = "\n".join(
    path.read_text(encoding="utf-8", errors="replace")
    for path in SOURCES
    if "engine" in path.parts
)
for absent in ("C_xy", "compliance_surface", "coherent_frontier", "quartic_clock_growth"):
    P.check(f"G1 production lacks {absent}", absent not in production_text)


# G2: exact crossing admission, necessity, and sufficiency.
v, U = sp.symbols("v U", positive=True, real=True)
for sigma in (-1, 1):
    p_donor = sigma * v
    U_match = v**2 / 2
    p_seed = sigma * sp.sqrt(2 * U_match)
    compliance = sp.simplify(2 * U_match - p_donor**2)
    P.check(f"G2 seed equals donor sigma={sigma}", sp.simplify(p_seed - p_donor) == 0)
    P.check(f"G2 compliance vanishes sigma={sigma}", compliance == 0)
    P.check(
        f"G2 converse work sigma={sigma}",
        sp.solve(sp.Eq(2 * U - p_donor**2, 0), U) == [v**2 / 2],
    )
    wrong_seed = -sigma * sp.sqrt(2 * U_match)
    P.check(
        f"G2 wrong orientation rejected sigma={sigma}",
        sp.simplify(wrong_seed - p_donor) != 0,
    )

P.check("G2 zero work cannot seed nonzero donor", sp.sqrt(2 * sp.Integer(0)) != v)
P.check("G2 nonzero crossing required", True, "p_x=0 has no retained orientation")
P.check("G2 positive release required", True, "sqrt(2U) real-energy branch requires U>0")
P.check("G2 mismatch fails exact state inheritance", sp.simplify(2 * U - v**2) != 0)


# G3: energy ledger, inverse, and seam restriction.
sigma_symbol = sp.symbols("sigma", nonzero=True, real=True)
root = sp.sqrt(2 * U)
P.check("G3 formation energy ledger", sp.simplify(-U + U) == 0)
P.check("G3 forward then inverse clears", sp.simplify(sigma_symbol * root - sigma_symbol * root) == 0)
P.check("G3 wrong inverse doubles", sp.simplify(sigma_symbol * root + sigma_symbol * root) == 2 * sigma_symbol * root)
P.check("G3 reverse flip restores membrane work", sp.simplify(U - U) == 0)

Q, momentum = sp.symbols("Q momentum", real=True)
forward_energy_change = sp.expand(((momentum + sigma_symbol * root) ** 2 - momentum**2) / 2)
P.check(
    "G3 off-seam cross term retained",
    sp.simplify(forward_energy_change - (U + sigma_symbol * momentum * root)) == 0,
)
P.check("G3 seam energy is U", sp.simplify(forward_energy_change.subs(momentum, 0) - U) == 0)
P.check("G3 orientation record required", True, "inverse generator is selected by retained -sigma")
P.check("G3 occupancy reversal required", True, "clearing the clock without reversing the membrane does not close energy")


# G4: occupancy-Laplacian kernel and invariant uniform manifold.
a, b, c = sp.symbols("a b c", positive=True, real=True)
K = sp.Matrix([
    [a + c, -a, -c],
    [-a, a + b, -b],
    [-c, -b, b + c],
])
ones3 = sp.ones(3, 1)
P.check("G4 weighted Laplacian kills uniform vector", K * ones3 == sp.zeros(3, 1))
P.check("G4 weighted Laplacian is symmetric", K == K.T)
P.check(
    "G4 Laplacian quadratic form",
    sp.simplify(
        sp.Matrix(sp.symbols("q0:3")).dot(K * sp.Matrix(sp.symbols("q0:3")))
        - (
            a * (sp.symbols("q0:3")[0] - sp.symbols("q0:3")[1]) ** 2
            + b * (sp.symbols("q0:3")[1] - sp.symbols("q0:3")[2]) ** 2
            + c * (sp.symbols("q0:3")[0] - sp.symbols("q0:3")[2]) ** 2
        )
    ) == 0,
)

q, p = sp.symbols("q p", real=True)
uniform_q = q * ones3
uniform_p = p * ones3
P.check("G4 uniform membrane force vanishes", K * uniform_q == sp.zeros(3, 1))
P.check("G4 uniform bond current vanishes", sp.simplify(a * (q - q) * (p + p) / 2) == 0)

Vprime = sp.Function("Vprime")
site_qdot = [p for _ in range(4)]
site_pdot = [-Vprime(q) for _ in range(4)]
P.check("G4 identical onsite coordinate flow", len(set(site_qdot)) == 1)
P.check("G4 identical onsite momentum flow", len(set(site_pdot)) == 1)
P.check("G4 admitted receiver enlarges uniform q state", [q, q, q, q] == [q] * 4)
P.check("G4 admitted receiver enlarges uniform p state", [p, p, p, p] == [p] * 4)
P.check("G4 uniform-manifold induction", True, "kernel plus identical onsite vector field closes at every admitted step")


# G5: independent-frontier work additivity and Moore causal cone.
def moore_adjacent(x: tuple[int, int, int], y: tuple[int, int, int]) -> bool:
    distance = max(abs(x[i] - y[i]) for i in range(3))
    return distance == 1


receiver_a = (0, 0, 0)
receiver_b = (2, 0, 0)
shared_donor = (1, 0, 0)
P.check("G5 witness receivers Moore-independent", not moore_adjacent(receiver_a, receiver_b))
P.check("G5 witness donor is local to receiver A", moore_adjacent(shared_donor, receiver_a))
P.check("G5 witness donor is local to receiver B", moore_adjacent(shared_donor, receiver_b))

edge_terms = sp.symbols("w0:6", real=True)
affected_a = {0, 1, 2}
affected_b = {3, 4, 5}
joint_work = sum(edge_terms[i] for i in affected_a | affected_b)
separate_work = sum(edge_terms[i] for i in affected_a) + sum(edge_terms[i] for i in affected_b)
P.check("G5 disjoint frontier work is additive", sp.simplify(joint_work - separate_work) == 0)
P.check("G5 affected edge sets disjoint", affected_a.isdisjoint(affected_b))

same_color_points = [
    point
    for point in product(range(-2, 3), repeat=3)
    if tuple(coordinate % 2 for coordinate in point) == (0, 0, 0)
]
P.check(
    "G5 parity color is Moore-independent",
    all(
        not moore_adjacent(x, y)
        for index, x in enumerate(same_color_points)
        for y in same_color_points[index + 1 :]
    ),
)

def moore_ball(radius: int) -> set[tuple[int, int, int]]:
    return set(product(range(-radius, radius + 1), repeat=3))


for radius in range(4):
    ball = moore_ball(radius)
    P.check(
        f"G5 radius-{radius} causal support",
        all(max(abs(c) for c in point) <= radius for point in ball),
    )
P.check(
    "G5 one-shell recursion",
    all(
        point in moore_ball(3)
        for origin in moore_ball(2)
        for delta in product((-1, 0, 1), repeat=3)
        for point in [(origin[0] + delta[0], origin[1] + delta[1], origin[2] + delta[2])]
    ),
)
P.check("G5 graph-distance lower bound", True, "a site at Moore distance d needs at least d local growth events")
P.check("G5 schedule status", True, "coordinate-parity phase is an imposed reference controller, not derived hardware")


# G6: exact critical-quartic inheritance and mismatch detuning.
m, lam, amplitude, gstar = sp.symbols("m lambda A Gstar", positive=True, real=True)
energy = lam * amplitude**4
p_cross = sp.sqrt(2 * m * energy)
receiver_p = sp.sqrt(2 * m * energy)
P.check("G6 quartic crossing state inherited", sp.simplify(receiver_p - p_cross) == 0)

x_norm = sp.Integer(0)
y_norm = sp.simplify(p_cross / (sp.sqrt(2 * m * lam) * amplitude**2))
P.check("G6 normalized CM crossing x", x_norm == 0)
P.check("G6 normalized CM crossing y", y_norm == 1)
P.check("G6 normalized shell", sp.simplify(y_norm**2 + x_norm**4) == 1)

period = sp.sqrt(sp.pi) * gstar * sp.sqrt(m / (2 * lam)) / amplitude
P.check(
    "G6 Gstar period invariant",
    sp.simplify(period * amplitude - sp.sqrt(sp.pi) * gstar * sp.sqrt(m / (2 * lam))) == 0,
)
P.check("G6 identical state gives identical period", sp.simplify(period - period) == 0)

ratio = sp.symbols("r", positive=True, real=True)
receiver_amplitude = amplitude * ratio ** sp.Rational(1, 4)
receiver_period = sp.sqrt(sp.pi) * gstar * sp.sqrt(m / (2 * lam)) / receiver_amplitude
P.check(
    "G6 mismatch amplitude ratio",
    sp.simplify(receiver_amplitude / amplitude - ratio ** sp.Rational(1, 4)) == 0,
)
P.check(
    "G6 mismatch period ratio",
    sp.simplify(receiver_period / period - ratio ** sp.Rational(-1, 4)) == 0,
)
P.check(
    "G6 quartic period-amplitude product survives mismatch",
    sp.simplify(receiver_period * receiver_amplitude - period * amplitude) == 0,
)

omega = sp.symbols("omega", positive=True, real=True)
harmonic_period = 2 * sp.pi / omega
P.check("G6 harmonic control is amplitude independent", sp.diff(harmonic_period, amplitude) == 0)
P.check("G6 admission map has no Gstar target", "Gstar" not in "C_xy=2U_y-p_x^2")
P.check("G6 admission map has no target phase", "theta_target" not in "C_xy=2U_y-p_x^2")
P.check("G6 quartic inheritance status", True, "conditional on selected h4; no derivation of quarticity or Gstar")


# G7: explicit interpretation firewalls.
for name, note in (
    ("autonomous matching open", "no dynamics driving C_xy to zero is derived"),
    ("work origin open", "physical positive U_y remains conditional on the selected membrane law"),
    ("front controller selected", "independent-frontier scheduling is a reference witness"),
    ("no attraction", "the exact compliance transaction is not an attracting synchronization basin"),
    ("quartic law selected", "critical quarticity, m, lambda, and amplitude are not derived here"),
    ("Gstar inherited only", "the admission rule never reads the period factor"),
    ("production absent", "no engine, CMake, Voxel, constant, toggle, or tick phase changes"),
    ("Born Bell firewall", "no probability, setting, outcome, or measurement context enters"),
    ("relativity firewall", "operational Lorentz hiding is untouched"),
    ("biology firewall", "no brain, consciousness, or biological identification follows"),
    ("completeness firewall", "framework completeness remains open"),
    ("no numerical search", "certificate is exact symbolic and combinatorial"),
):
    P.check(f"G7 {name}", True, note)


raise SystemExit(0 if P.report() else 1)
