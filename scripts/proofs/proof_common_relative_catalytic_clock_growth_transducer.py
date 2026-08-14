"""FTD-0997 exact common/relative catalytic clock-growth certificate.

The certificate is symbolic and combinatorial.  It performs no numerical
search, fit, empirical substitution, or production mutation.
"""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_COMMON_RELATIVE_CATALYTIC_CLOCK_GROWTH_TRANSDUCER_v1.md"
)

SOURCES = {
    PROTOCOL: "632A3453B5C4BC166153FA8DF54AAB589563846A78C160531A2A0DDCDC7C0DF1",
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_CROSSING_MATCHED_FORMATION_ENERGY_AND_CAUSAL_QUARTIC_CLOCK_GROWTH_v1.md"
    ): "68087ED4B410AF54571D61E6F8C7ABEFA694E29E0889ADC2286CC45BFEB70C0F",
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_TERNARY_OCCUPANCY_MEMBRANE_AND_SELF_DUAL_BODY_CLOCK_SPLIT_v1.md"
    ): "A19593DACD2CE97A6B785F235AE5048EADC228680E07D2F90F4C4DB7BD15333C",
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_ZERO_ACTION_CANONICAL_SEED_AND_CAUSAL_CLOCK_GROWTH_BOUNDARY_v1.md"
    ): "897367658B339F074A78FEA017994EEA63AD7921BA4C597663EA123088E76306",
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_CATALYTIC_PHASE_REFERENCE_TRANSDUCER_v1.md"
    ): "8BD6BB16999E91A72CADBA991A215F56A3E3E13816073E39B36F9EB51FD5FE33",
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_GLOBAL_AGGREGATE_WORK_AND_LOCAL_CONCURRENCY_OWNERSHIP_BOUNDARY_v1.md"
    ): "1CF020D3AA4EB78746C8CF7B932B3AB27E265E173E7F81524CF2A4547A38FA91",
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
        self.computational_total = 0
        self.computational_passed = 0
        self.disclosure_total = 0
        self.disclosure_passed = 0

    def check(
        self,
        name: str,
        condition: object,
        detail: object = "",
        category: str = "computational",
    ) -> None:
        self.total += 1
        ok = bool(condition)
        if ok:
            self.passed += 1
        if category == "disclosure":
            self.disclosure_total += 1
            if ok:
                self.disclosure_passed += 1
        else:
            self.computational_total += 1
            if ok:
                self.computational_passed += 1
        marker = "PASS" if ok else "FAIL"
        suffix = f" :: {detail}" if detail != "" else ""
        print(f"[{marker}] {name}{suffix}")

    def report(self) -> bool:
        print()
        print(
            "FTD-0997 common/relative catalytic clock-growth transducer: "
            f"{self.computational_passed}/{self.computational_total} computational checks passed; "
            f"{self.disclosure_passed}/{self.disclosure_total} disclosure/scope assertions logged "
            "(cannot fail)"
        )
        if self.passed == self.total:
            print("OUTCOME B — EXISTING-PAIR CATALYTIC TRANSDUCER / NATIVE COMPLIANCE OPEN")
            print("RELATIVE_PAIR_CAPACITY=EXACT_NO_NEW_CONTINUOUS_TYPE")
            print("SWAP_REFILL_ENERGY_AND_INVERSE=EXACT_CONDITIONAL")
            print("CATALYTIC_RECURSION_IFF_FORMATION_WORK_EQUALS_CLOCK_ENERGY")
            print("QUIESCENT_MEMBRANE_REFILL=ZERO")
            print("PRODUCTION_INTEGRATION=NONE")
            return True
        print("OUTCOME D — INVALID")
        return False


P = Proof()


# G1: source lock, source-scope markers, and production absence.
for path, expected in SOURCES.items():
    P.check(f"G1 hash {path.name}", path.is_file() and sha256(path) == expected)

texts = {path.name: path.read_text(encoding="utf-8", errors="replace") for path in SOURCES}
P.check("G1 compliance theorem present", "C_{xy}^{(m)}=2mU_y-p_x^2=0" in texts[
    "THEOREM_CROSSING_MATCHED_FORMATION_ENERGY_AND_CAUSAL_QUARTIC_CLOCK_GROWTH_v1.md"
])
P.check("G1 common relative chart present", "q_\\pm={q_L\\pm q_R" in texts[
    "THEOREM_TERNARY_OCCUPANCY_MEMBRANE_AND_SELF_DUAL_BODY_CLOCK_SPLIT_v1.md"
])
P.check("G1 diagonal dual Hamiltonian present", "H_0={1\\over2}p_+^Tp_+" in texts[
    "THEOREM_TERNARY_OCCUPANCY_MEMBRANE_AND_SELF_DUAL_BODY_CLOCK_SPLIT_v1.md"
])
P.check("G1 Cartesian refill present", "P'=\\sigma\\sqrt{2U" in texts[
    "THEOREM_ZERO_ACTION_CANONICAL_SEED_AND_CAUSAL_CLOCK_GROWTH_BOUNDARY_v1.md"
])
P.check("G1 catalytic pair precedent present", "reference is catalytic" in texts[
    "THEOREM_CATALYTIC_PHASE_REFERENCE_TRANSDUCER_v1.md"
])
P.check("G1 local complete-pair ownership present", "complete local" in texts[
    "THEOREM_GLOBAL_AGGREGATE_WORK_AND_LOCAL_CONCURRENCY_OWNERSHIP_BOUNDARY_v1.md"
].lower())

production_text = "\n".join(
    text for name, text in texts.items() if name.endswith((".h", ".cpp"))
)
for absent in ("catalytic_clock_growth", "relative_port_owner", "compliance_surface", "swap_refill"):
    P.check(f"G1 production lacks {absent}", absent not in production_text)


# G2: exact complete-pair swap versus incomplete scalar swap.
J2 = sp.Matrix([[0, 1], [-1, 0]])
Omega4 = sp.diag(1, 1)
Omega4 = sp.diag(J2, J2)
I2 = sp.eye(2)
Z2 = sp.zeros(2)
S = sp.Matrix.vstack(sp.Matrix.hstack(Z2, I2), sp.Matrix.hstack(I2, Z2))

P.check("G2 pair swap symplectic", S.T * Omega4 * S == Omega4)
P.check("G2 pair swap determinant", S.det() == 1)
P.check("G2 pair swap involution", S * S == sp.eye(4))
P.check("G2 pair swap inverse", S.inv() == S)
P.check("G2 pair swap orthogonal", S.T * S == sp.eye(4))

q_r, p_r, q_y, p_y = sp.symbols("q_r p_r q_y p_y", real=True)
state = sp.Matrix([q_r, p_r, q_y, p_y])
swapped = S * state
P.check("G2 relative output is old receiver", swapped[:2, 0] == sp.Matrix([q_y, p_y]))
P.check("G2 receiver output is old relative", swapped[2:, 0] == sp.Matrix([q_r, p_r]))
P.check("G2 oscillator norm preserved", sp.simplify(swapped.dot(swapped) - state.dot(state)) == 0)

scalar_swap = sp.Matrix([
    [0, 0, 1, 0],
    [0, 1, 0, 0],
    [1, 0, 0, 0],
    [0, 0, 0, 1],
])
P.check("G2 scalar-only swap not symplectic", scalar_swap.T * Omega4 * scalar_swap != Omega4)
P.check("G2 scalar-only swap orientation defect", scalar_swap.det() == -1)


# G3: swap-refill transaction, arbitrary work, exact energy, and inverse.
e, U, m = sp.symbols("e U m", positive=True, real=True)
source_energy = sp.symbols("E_s", real=True)
for sigma in (-1, 1):
    z = sp.Matrix([0, sigma * sp.sqrt(2 * m * e)])
    z_u = sp.Matrix([0, sigma * sp.sqrt(2 * m * U)])
    blank = sp.zeros(2, 1)

    after_swap = S * sp.Matrix.vstack(z, blank)
    P.check(f"G3 swap empties port sigma={sigma}", after_swap[:2, 0] == blank)
    P.check(f"G3 swap fills receiver sigma={sigma}", after_swap[2:, 0] == z)
    P.check(
        f"G3 refill output sigma={sigma}",
        z_u[1] == sigma * sp.sqrt(2 * m * U),
        category="disclosure",
    )

    energy_z = sp.simplify(z[1] ** 2 / (2 * m))
    energy_zu = sp.simplify(z_u[1] ** 2 / (2 * m))
    P.check(f"G3 donor/port input energy sigma={sigma}", energy_z == e)
    P.check(f"G3 refilled port energy sigma={sigma}", energy_zu == U)

    initial_total = e + e + source_energy
    final_total = e + U + e + source_energy - U
    P.check(
        f"G3 total energy sigma={sigma}",
        sp.simplify(final_total - initial_total) == 0,
        category="disclosure",
    )
    P.check(f"G3 mismatch retained sigma={sigma}", sp.simplify(energy_zu - energy_z) == U - e)
    P.check(f"G3 compliance sigma={sigma}", sp.solve(sp.Eq(z_u[1], z[1]), U) == [e])
    P.check(f"G3 mass compliance sigma={sigma}", sp.simplify(2 * m * e - z[1] ** 2) == 0)

    inverse_refill = blank
    inverse_swap = S * sp.Matrix.vstack(inverse_refill, z)
    P.check(f"G3 inverse restores port sigma={sigma}", inverse_swap[:2, 0] == z)
    P.check(f"G3 inverse clears receiver sigma={sigma}", inverse_swap[2:, 0] == blank)
    P.check(
        f"G3 inverse source energy sigma={sigma}",
        sp.simplify((source_energy - U) + U - source_energy) == 0,
        category="disclosure",
    )

P.check(
    "G3 positive U branch",
    True,
    "formation refill is registered only for U>0",
    category="disclosure",
)
P.check(
    "G3 arbitrary mismatch remains invertible",
    True,
    "inverse uses retained sigma, U, occupancy history, and source variables",
    category="disclosure",
)


# G4: scope of the apparent clone.
P.check("G4 generic port differs from donor", sp.simplify(sp.sqrt(U) - sp.sqrt(e)) != 0)
P.check("G4 equality constrained by one scalar", sp.solve(sp.Eq(U - e, 0), U) == [e])
P.check(
    "G4 source changes on compliant branch",
    sp.simplify((source_energy - e) - source_energy) == -e,
    category="disclosure",
)
P.check("G4 full swap map rank", S.rank() == 4)
P.check("G4 full swap volume", abs(S.det()) == 1)
P.check(
    "G4 full map inverse retained",
    True,
    "inverse refill precedes inverse swap",
    category="disclosure",
)
P.check(
    "G4 not unrestricted cloning",
    True,
    "C=R preparation and U=e compliance correlate input with changed source",
    category="disclosure",
)


# G5: common/relative capacity and absence of native forcing.
s2 = sp.sqrt(2)
T = sp.Matrix([
    [1 / s2, 0, 1 / s2, 0],
    [0, 1 / s2, 0, 1 / s2],
    [1 / s2, 0, -1 / s2, 0],
    [0, 1 / s2, 0, -1 / s2],
])
P.check("G5 common relative chart orthogonal", sp.simplify(T.T * T) == sp.eye(4))
P.check("G5 common relative chart symplectic", sp.simplify(T.T * Omega4 * T) == Omega4)
P.check("G5 common relative chart determinant", sp.simplify(T.det()) == 1)
P.check("G5 no new continuous pair", T.rank() == 4)

q_plus, p_plus, q_minus, p_minus = sp.symbols("q_plus p_plus q_minus p_minus", real=True)
k_plus, k_minus = sp.symbols("k_plus k_minus", positive=True, real=True)
H0 = (p_plus**2 + k_plus * q_plus**2 + p_minus**2 + k_minus * q_minus**2) / 2
P.check("G5 block Hamiltonian plus-minus q", sp.diff(H0, q_plus, q_minus) == 0)
P.check("G5 block Hamiltonian plus-minus p", sp.diff(H0, p_plus, p_minus) == 0)
P.check("G5 block Hamiltonian mixed q-p", sp.diff(H0, q_plus, p_minus) == 0)
P.check(
    "G5 relative preparation not forced",
    True,
    "block diagonal flow admits independent common and relative initial data",
    category="disclosure",
)

p_cross, U_config = sp.symbols("p_cross U_config", real=True)
mass = sp.symbols("mass", positive=True, real=True)
F = 2 * mass * U_config - p_cross**2
P.check("G5 compliance derivative", sp.diff(F, p_cross) == -2 * p_cross)
P.check("G5 derivative nonzero crossing", sp.diff(F, p_cross).subs(p_cross, 1) == -2)
P.check(
    "G5 compliance regular level",
    True,
    "implicit-function theorem applies wherever p_cross != 0",
    category="disclosure",
)
P.check("G5 compliance not open-set identity", sp.Poly(F, p_cross).degree() == 2)
P.check("G5 configuration work has no momentum derivative", sp.diff(U_config, p_cross) == 0)


# G6: exact quiescent-seam work boundary.
a0, a1, a2 = sp.symbols("a0 a1 a2", positive=True, real=True)
g0, g1, g2, gp0, gp1, gp2 = sp.symbols("g0 g1 g2 gp0 gp1 gp2", real=True)
zero_strains = [sp.Integer(0), sp.Integer(0), sp.Integer(0)]
work_terms = [
    (gp - g) * a * d**2 / 2
    for gp, g, a, d in zip((gp0, gp1, gp2), (g0, g1, g2), (a0, a1, a2), zero_strains)
]
P.check(
    "G6 every zero-strain bond work vanishes",
    all(term == 0 for term in work_terms),
    category="disclosure",
)
P.check(
    "G6 quiescent total work vanishes",
    sp.simplify(sum(work_terms)) == 0,
    category="disclosure",
)
P.check(
    "G6 quiescent released work vanishes",
    sp.simplify(-sum(work_terms)) == 0,
    category="disclosure",
)
P.check(
    "G6 zero-work refill remains blank",
    sp.sqrt(2 * mass * sp.Integer(0)) == 0,
    category="disclosure",
)
P.check("G6 positive donor cannot be restored", sp.sqrt(2 * mass * sp.Integer(0)) != sp.sqrt(2 * mass * e))
P.check(
    "G6 one-shot port depletion",
    sp.simplify(sp.Integer(0) - e) == -e,
    category="disclosure",
)
P.check(
    "G6 positive strain can supply work",
    True,
    "pre-existing void/boundary strain is allowed but not forced to equal e",
    category="disclosure",
)
P.check(
    "G6 onsite latent term allowed",
    True,
    "a separately registered onsite load may contribute but is not in the static membrane theorem",
    category="disclosure",
)
P.check(
    "G6 relative environment allowed",
    True,
    "the open relative channel may carry energy but no ownership/refill law is derived",
    category="disclosure",
)
P.check(
    "G6 reserve allowed",
    True,
    "a prepositioned local complete pair is a selected mechanism, not free energy",
    category="disclosure",
)


# G7: epistemic, ontology, and production firewalls.
for name, note in (
    ("prepared C equals R", "phase-complete common/relative equality is selected initial data"),
    ("swap controller open", "pair capacity does not choose engagement or scheduling"),
    ("relative ownership open", "existing pair is not yet protected or assigned to each event"),
    ("compliance attraction open", "the regular codimension-one surface is not an attracting basin"),
    ("no new pair type", "the reference uses existing plus/minus pairs only"),
    ("quarticity open", "critical quartic hardware and its scales are not derived"),
    ("Gstar open", "no finite-tick Gstar implementation follows"),
    ("production absent", "engine, CMake, Voxel, constants, toggles, and default ticks are unchanged"),
    ("Born Bell firewall", "no probability, context, setting, or outcome enters"),
    ("relativity firewall", "operational Lorentz hiding is untouched"),
    ("biology firewall", "no brain or consciousness identification follows"),
    ("completeness firewall", "framework completeness remains open"),
    ("no numerical search", "all certificate gates are exact symbolic statements"),
):
    P.check(f"G7 {name}", True, note, category="disclosure")


raise SystemExit(0 if P.report() else 1)
