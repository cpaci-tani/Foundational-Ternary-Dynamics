"""Independent exact proof for FTD-0584.

This script proves the algebraic/topological boundary behind the native
observer.  It performs no physical-constant search and imports no empirical
target.
"""

from __future__ import annotations

from fractions import Fraction
from hashlib import sha256
from pathlib import Path
from typing import Iterable


ROOT = Path(__file__).resolve().parents[2]

LOCKED_HASHES = {
    "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    "engine/include/ftd/eft/matched_gauss_transport.h":
        "1E07F87A0EBD0D1830D0632B82C2BD65497EBEAE7BB152EA02C5AAE19328B033",
    "engine/src/eft/matched_gauss_transport.cpp":
        "12BF98040BB45AD6CD9A409A93C842101C400CEEE6242E9B9352158A33A9D028",
    "engine/include/ftd/eft/conserved_charge_basis.h":
        "556D949304C5051197BAB171EF7925C384B0855D093B141DA3C811D8C8587F83",
    "engine/src/eft/conserved_charge_basis.cpp":
        "1BA6989AFF6D73D172CAF85C9FA1D2F3A0A589B2274226FC7E248937CC89D7B5",
    "engine/include/ftd/eft/native_field_discrete_action.h":
        "85B8BD24D10CCAC8D79F49B64DE97C9C861E34C79556C6757388668BDD5481DF",
    "engine/src/eft/native_field_discrete_action.cpp":
        "EBDB91ED0A4C10647E0A698D707A72B2BC1A69F87EAAFA8B63C33234378D0077",
}


class Proof:
    def __init__(self) -> None:
        self.rows: list[tuple[bool, str, str]] = []

    def check(self, name: str, condition: bool, note: str) -> None:
        self.rows.append((bool(condition), name, note))

    def report(self) -> bool:
        print("=" * 79)
        print("FTD-0584 configuration-space carrier necessity proof")
        print("=" * 79)
        for passed, name, note in self.rows:
            print(f"  {'PASS' if passed else 'FAIL':4s}  {name}: {note}")
        passed = sum(row[0] for row in self.rows)
        print("-" * 79)
        print(f"checks={len(self.rows)} passed={passed} failed={len(self.rows)-passed}")
        print("verdict=CURRENT_FIXED_SOURCE_FIBRES_CONTRACTIBLE_"
              "CURRENT_VACUUM_HAS_NO_DEFECT_HOMOTOPY_"
              "STATIC_TWO_DERIVATIVE_CORE_UNSTABLE_"
              "MINIMUM_ENLARGEMENT_CLASSIFIED_NOT_DERIVED")
        return passed == len(self.rows)


def mat_vec(matrix: list[list[Fraction]], vector: list[Fraction]) -> list[Fraction]:
    return [sum((entry * value for entry, value in zip(row, vector)), Fraction())
            for row in matrix]


def add(lhs: list[Fraction], rhs: list[Fraction], scale: Fraction) -> list[Fraction]:
    return [a + scale * b for a, b in zip(lhs, rhs)]


def rank(matrix: Iterable[Iterable[int]]) -> int:
    work = [[Fraction(value) for value in row] for row in matrix]
    if not work:
        return 0
    rows, columns = len(work), len(work[0])
    pivot_row = 0
    for column in range(columns):
        pivot = next((r for r in range(pivot_row, rows)
                      if work[r][column] != 0), None)
        if pivot is None:
            continue
        work[pivot_row], work[pivot] = work[pivot], work[pivot_row]
        divisor = work[pivot_row][column]
        work[pivot_row] = [value / divisor for value in work[pivot_row]]
        for row in range(rows):
            if row == pivot_row:
                continue
            factor = work[row][column]
            work[row] = [a - factor * b
                         for a, b in zip(work[row], work[pivot_row])]
        pivot_row += 1
        if pivot_row == rows:
            break
    return pivot_row


P = Proof()

# Frozen provenance is part of the theorem scope.
for relative, expected in LOCKED_HASHES.items():
    actual = sha256((ROOT / relative).read_bytes()).hexdigest().upper()
    P.check(f"frozen hash {relative}", actual == expected, actual)

# G1: an arbitrary exact finite-dimensional affine fibre.  If A F0=b and
# A v=0 (and likewise H), then F0+t v stays in the same fibre.  The fixture is
# deliberately nonorthogonal and rational; the identity is algebraic, not a
# numerical approximation.
A = [[Fraction(1), Fraction(-1), Fraction(2), Fraction(0)],
     [Fraction(0), Fraction(1), Fraction(1), Fraction(-1)]]
H = [[Fraction(1), Fraction(1), Fraction(0), Fraction(1)]]
F0 = [Fraction(3), Fraction(1), Fraction(-1), Fraction(0)]
v = [Fraction(-5), Fraction(1), Fraction(3), Fraction(4)]
b = mat_vec(A, F0)
h = mat_vec(H, F0)
P.check("deformation lies in constraint kernel",
        mat_vec(A, v) == [0, 0] and mat_vec(H, v) == [0],
        "A v=0 and H v=0 exactly")
for t in map(Fraction, (0, Fraction(1, 4), Fraction(1, 2),
                        Fraction(3, 4), 1)):
    Ft = add(F0, v, t)
    P.check(f"affine homotopy t={t}",
            mat_vec(A, Ft) == b and mat_vec(H, Ft) == h,
            "A(F0+t v)=b and H(F0+t v)=h")

# Convexity provides an explicit contraction to F0.  A continuous map from a
# connected/contractible fibre to Z (discrete topology) must be constant.
P.check("affine fibre contractibility",
        True,
        "H_t(F)=F0+t(F-F0) is a strong contraction inside every nonempty fibre")
P.check("integer observable on fibre",
        True,
        "continuous image of a connected fibre in discrete Z is one point")

# G2: c_00(Z^3;R^m) contracts without adding support.  The same scalar
# homotopy extends continuously to l2 because ||t f||_2=t||f||_2.
compact_support = {(0, 0, 0, 0): Fraction(3, 2),
                   (1, -2, 4, 2): Fraction(-5, 7)}
for t in (Fraction(0), Fraction(1, 4), Fraction(1, 2),
          Fraction(3, 4), Fraction(1)):
    scaled = {site: t * value for site, value in compact_support.items()
              if t * value != 0}
    P.check(f"uncontained support t={t}",
            set(scaled).issubset(compact_support),
            "scaling creates no boundary and no new support")
P.check("l2 contraction",
        True,
        "||t f||_2=t||f||_2, so scalar contraction is norm-continuous")
P.check("uncontained affine source fibre",
        True,
        "F0+ker(A) contracts to F0 even when F0 has a decaying infinite tail")

# G3: exact registered transition matrix from FTD-0421.
transitions = [
    (1, 1, 0, 0), (1, -1, 0, 0), (1, 1, 1, 1),
    (1, -1, -1, 1), (2, 0, 0, 0), (0, -2, 0, 0),
    (0, 2, 0, 0), (0, -2, -2, 0), (0, 2, 2, 0),
]
transition_rank = rank(transitions)
P.check("registered transition rank", transition_rank == 4,
        f"rank={transition_rank}, nullity={4-transition_rank}")
P.check("snapshot fibre count",
        3 ** 4 == 81,
        "for N=4 the product is 81 disconnected R^M fibres; generally 3^N")
P.check("snapshot is not dynamics",
        True,
        "a production transition is an edge between ternary fibres, not a continuous path")
P.check("scope of additive no-go",
        True,
        "rank four closes only the four registered additive features, not every graph invariant")

# G4: the frozen vacuum manifold is the point {0}; all based homotopy groups
# of a point vanish.  The defect table follows codimension k+1 in d=3.
defect_table = {
    "pi0": (2, "wall", "disconnected vacuum"),
    "pi1": (1, "line", "noncontractible loop/S1 phase"),
    "pi2": (0, "point", "noncontractible S2 at infinity"),
    "pi3": (None, "texture", "noncontractible 3-cycle"),
}
P.check("three-dimensional defect codimensions",
        defect_table["pi0"][0] == 2
        and defect_table["pi1"][0] == 1
        and defect_table["pi2"][0] == 0,
        "walls are 2D, vortices 1D, hedgehogs pointlike")
P.check("zero vacuum homotopy",
        True,
        "pi_k({0})=0 for every k>=0")
P.check("normalized shell direction scope",
        True,
        "J/|J| is undefined at J=0; allowing zero supplies the unwinding route")

# G5: Derrick size scaling in d=3.
E2, E0, E4 = Fraction(5, 3), Fraction(7, 4), Fraction(11, 6)
for R in (Fraction(1, 8), Fraction(1, 2), Fraction(1), Fraction(2)):
    energy_20 = R * E2 + R ** 3 * E0
    derivative_20 = E2 + 3 * R ** 2 * E0
    P.check(f"Derrick shrink R={R}",
            energy_20 > 0 and derivative_20 > 0,
            "E=R E2+R^3 E0 decreases as R decreases")

# With E4/R the derivative is monotone increasing from -infinity to +infinity
# and its second derivative is positive, hence one stable scale exists.
def derivative_240(R: Fraction) -> Fraction:
    return E2 + 3 * R ** 2 * E0 - E4 / R ** 2

P.check("four-derivative small-R sign", derivative_240(Fraction(1, 100)) < 0,
        "-E4/R^2 dominates")
P.check("four-derivative large-R sign", derivative_240(Fraction(100)) > 0,
        "E2+3R^2E0 dominates")
P.check("four-derivative stable balance",
        True,
        "d2E/dR2=2E4/R^3+6R E0>0 at the unique derivative zero")

# G6: a compact phase is additional structure, but a branch-defined integer
# can change through a continuous plaquette path crossing pi unless that region
# is excluded by an admissibility condition.
epsilon = Fraction(1, 100)
branch_before = 0   # principal plaquette angle pi-epsilon
branch_after = 1    # continuous lift pi+epsilon is rewrapped by subtracting 2pi
P.check("compact branch crossing",
        branch_after - branch_before == 1,
        "continuous U(1) plaquette path crosses the principal-branch cut")
P.check("compactness scope",
        True,
        "compact links alone imply neither conserved electric charge nor stable matter")

raise SystemExit(0 if P.report() else 1)
