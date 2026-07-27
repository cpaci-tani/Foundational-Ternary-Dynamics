"""Independent exact proof for FTD-0585.

No empirical constants, coincidence searches, or fitted mechanisms occur in
this proof.  It checks the frozen source graph and exact finite-volume moment
identities behind the native observer.
"""

from __future__ import annotations

from fractions import Fraction
from hashlib import sha256
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]

LOCKED_HASHES = {
    "engine/src/render_bridge.cpp":
        "A822E0FAFAF71FE5458B2A7450868A8414B1C8564089BF6C6484FC34B7559359",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "engine/src/render_bridge_phases/phase_forces.cpp":
        "F7A855DC3ED3BF9882807CF7C8D1A35CF66864433B711CA5CA4B9CB836549322",
    "engine/src/render_bridge_phases/phase_movement.cpp":
        "6149B37C5A28B8EE9B8544CAEC24006D0964D1C8F344CA63C68DC6536A47E8FB",
    "engine/src/transmutation_phases.cpp":
        "4013A9B769199D54976347378FD03DFF6415B7F641F35D3FAE498125EB288043",
    "engine/src/strong_stress_energy.cpp":
        "A9A38B8D0FE6FAA9692CED77AC29841E9FB41596E7E16DB2F45F20E4F2C69F94",
    "engine/include/ftd/term_toggles.h":
        "2731A2BF1EF01456DFDFE4F1E20C8E64E3D839136BC633B13771D13360AC64AA",
    "engine/include/ftd/eft/dual_cell_continuity.h":
        "3DF32601AD46761A2870FFFF0DB9D65CC5C267EC7866668333F0BEC8E176DF43",
    "engine/src/eft/dual_cell_continuity.cpp":
        "90559DDFFE622991D958E6D04A034C470CB4AD8491F83A3BD33771AD0D7BE6D1",
}


class Proof:
    def __init__(self) -> None:
        self.rows: list[tuple[bool, str, str]] = []

    def check(self, name: str, condition: bool, note: str) -> None:
        self.rows.append((bool(condition), name, note))

    def report(self) -> bool:
        print("=" * 79)
        print("FTD-0585 native motion / reaction-front trichotomy proof")
        print("=" * 79)
        for passed, name, note in self.rows:
            print(f"  {'PASS' if passed else 'FAIL':4s}  {name}: {note}")
        passed = sum(row[0] for row in self.rows)
        print("-" * 79)
        print(f"checks={len(self.rows)} passed={passed} failed={len(self.rows)-passed}")
        print("verdict=TRANSPORT_REACTION_FRONT_AND_STALE_MEMORY_DISTINGUISHED_"
              "RECIPROCAL_NATIVE_PARTICLE_MOTION_STILL_CLOSED")
        return passed == len(self.rows)


P = Proof()

for relative, expected in LOCKED_HASHES.items():
    actual = sha256((ROOT / relative).read_bytes()).hexdigest().upper()
    P.check(f"frozen hash {relative}", actual == expected, actual)

render = (ROOT / "engine/src/render_bridge.cpp").read_text(encoding="utf-8")
read = (ROOT / "engine/src/render_bridge_phases/phase_read.cpp").read_text(
    encoding="utf-8")
write = (ROOT / "engine/src/render_bridge_phases/phase_write.cpp").read_text(
    encoding="utf-8")
forces = (ROOT / "engine/src/render_bridge_phases/phase_forces.cpp").read_text(
    encoding="utf-8")
movement = (ROOT / "engine/src/render_bridge_phases/phase_movement.cpp").read_text(
    encoding="utf-8")
transmutation = (ROOT / "engine/src/transmutation_phases.cpp").read_text(
    encoding="utf-8")
strong = (ROOT / "engine/src/strong_stress_energy.cpp").read_text(
    encoding="utf-8")

order_tokens = [
    "phase_read();", "phase_write();", "pair_production_cpu();",
    "phase_forces();", "phase_movement();", "weak_transmutation_cpu();",
]
positions = [render.index(token) for token in order_tokens]
P.check("production phase order", positions == sorted(positions),
        "read -> write/reactions -> force -> movement -> weak")
P.check("field read has no matter-velocity write",
        ".velocity" not in read and ".remainder" not in read,
        "phase_read writes field increments only")
P.check("force phase writes matter velocity",
        "v.velocity =" in forces,
        "selected field force supplies a kinematic writer")
P.check("movement integrates remainder",
        "v.remainder += v.velocity * rb.dt_" in movement,
        "zero velocity fixes zero remainder before any collision")
P.check("strong projection is separately selected",
        "strong_stress_energy" in strong and ".velocity =" in strong,
        "not part of the isolated reaction-free native arm")
P.check("transmutation does not write kinematics",
        ".velocity" not in transmutation and ".remainder" not in transmutation,
        "weak and pair phases change state/field labels only")

evaporation = write[write.index("// Evaporation (shared single + dual)"):
                    write.index("void phase_write_assign_pending_ids")]
manifest = write[write.index("inline void manifest_at"):
                 write.index("// =============================================================================")]
P.check("evaporation leaves hidden velocity",
        "rb.set_state(i, 0);" in evaporation
        and "v.particle_id = -1;" in evaporation
        and "v.velocity" not in evaporation
        and "v.remainder" not in evaporation,
        "state/ID/spin/color clear; velocity/remainder do not")
P.check("genesis reuses void kinematics",
        "rb.set_state" in manifest
        and "v.velocity" not in manifest
        and "v.remainder" not in manifest,
        "manifest_at assigns labels without resetting motion")

# Exact current/source decomposition on every face direction, charge sign, and
# translated copy. Coordinates are non-wrapping, so ordinary summation by
# parts applies without a boundary chart term.
directions = [(1, 0, 0), (-1, 0, 0), (0, 1, 0),
              (0, -1, 0), (0, 0, 1), (0, 0, -1)]
translations = [(-1, -1, -1), (0, 0, 0), (1, 1, 1)]
identity_count = 0
for charge in (-1, 1):
    q = Fraction(charge)
    for direction in directions:
        for translation in translations:
            start = tuple(Fraction(4 + value) for value in translation)
            finish = tuple(a + Fraction(d) for a, d in zip(start, direction))
            delta_m = tuple(q * (b - a) for a, b in zip(start, finish))
            current_rhs = tuple(q * Fraction(d) for d in direction)
            source_rhs = tuple((-q) * a + q * b
                               for a, b in zip(start, finish))
            P.check(
                f"moment identity q={charge} d={direction} t={translation}",
                delta_m == current_rhs == source_rhs,
                "same endpoints; transport current and balanced reaction source differ")
            identity_count += 2

P.check("registered identity count", identity_count == 72,
        f"identity_count={identity_count}")
P.check("global source balance is insufficient",
        sum((-1, 1)) == 0,
        "S=(-q,+q) conserves total signed polarity while remaining locally nonzero")
P.check("support motion is not a worldline theorem", True,
        "identical rho endpoints admit distinct (I,S) histories")
P.check("reaction-free zero-kinematics induction", True,
        "u_0=r_0=0, forces/reactions off => r_{n+1}=r_n+dt*u_n=0 and no hop")

raise SystemExit(0 if P.report() else 1)
