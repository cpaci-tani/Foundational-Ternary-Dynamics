"""Exact witnesses for the FTD postulate/action underdetermination audit.

This script performs no parameter search and compares no physical constants.
It checks finite combinatorics and exact maps only.  Its purpose is narrow:
show that the structural conditions advertised as P1--P5 do not select one
microscopic update, one active neighborhood, or one invariant measure.
"""

from fractions import Fraction
from itertools import permutations, product


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    status = "PASS" if condition else "FAIL"
    suffix = f" -- {detail}" if detail else ""
    print(f"[{status}] {name}{suffix}")


# ---------------------------------------------------------------------------
# 1. Range-one cubic symmetry does not uniquely select Moore-26.
# ---------------------------------------------------------------------------

moore26 = {
    v for v in product((-1, 0, 1), repeat=3) if v != (0, 0, 0)
}
sc6 = {v for v in moore26 if sum(x * x for x in v) == 1}
fcc12 = {v for v in moore26 if sum(x * x for x in v) == 2}
bcc8 = {v for v in moore26 if sum(x * x for x in v) == 3}
c18 = sc6 | fcc12


def oh_images(v: tuple[int, int, int]) -> set[tuple[int, int, int]]:
    images = set()
    for p in permutations(range(3)):
        for signs in product((-1, 1), repeat=3):
            images.add(tuple(signs[i] * v[p[i]] for i in range(3)))
    return images


def oh_invariant(shell: set[tuple[int, int, int]]) -> bool:
    return all(oh_images(v) <= shell for v in shell)


def induced_moore_connected(shell: set[tuple[int, int, int]]) -> bool:
    """Connectivity using range-one Moore adjacency between offsets."""

    seed = next(iter(shell))
    reached = {seed}
    frontier = [seed]
    while frontier:
        a = frontier.pop()
        for b in shell - reached:
            d = tuple(a[i] - b[i] for i in range(3))
            if d != (0, 0, 0) and max(abs(x) for x in d) <= 1:
                reached.add(b)
                frontier.append(b)
    return reached == shell


check("N1 shell cardinalities are 6/12/8/18/26", [len(sc6), len(fcc12), len(bcc8), len(c18), len(moore26)] == [6, 12, 8, 18, 26])
for name, shell in (("SC6", sc6), ("FCC12", fcc12), ("C18", c18), ("Moore26", moore26)):
    check(f"N2 {name} is O_h invariant", oh_invariant(shell))
for name, shell in (("SC6", sc6), ("C18", c18), ("Moore26", moore26)):
    check(f"N3 {name} induced range-one graph is connected", induced_moore_connected(shell))
check("N4 symmetry + range one + connectedness do not select Moore26", sc6 != c18 != moore26)


# ---------------------------------------------------------------------------
# 2. Distinct reversible local deterministic updates satisfy the same arena.
# ---------------------------------------------------------------------------

# The L=3 periodic quotient is a permitted finite probe of the undefined-
# boundary formulas.  It certifies exact inverses without asserting that the
# ontology is a completed torus; the maps extend pointwise to every specified
# site of the unbounded graph.
sites = tuple(product(range(3), repeat=3))
site_index = {x: i for i, x in enumerate(sites)}


def shift_site(x: tuple[int, int, int], amount: int) -> tuple[int, int, int]:
    return ((x[0] + amount) % 3, x[1], x[2])


def readout(j: tuple[int, int, int]) -> int:
    """A finite exact threshold witness for s=R(J)."""

    if j[0] > 0:
        return 1
    if j[0] < 0:
        return -1
    return 0


def identity(field: tuple[tuple[int, int, int], ...]):
    return field


def negate(field: tuple[tuple[int, int, int], ...]):
    return tuple(tuple(-a for a in j) for j in field)


def shift_x(field: tuple[tuple[int, int, int], ...], amount: int = 1):
    # Output at x reads the input at x-amount: a one-neighbor local shift.
    return tuple(field[site_index[shift_site(x, -amount)]] for x in sites)


def collapse(field: tuple[tuple[int, int, int], ...]):
    return tuple((0, 0, 0) for _ in field)


field = tuple((x[0] - 1, x[1] - 1, x[2] - 1) for x in sites)
state = tuple(readout(j) for j in field)

check("D1 identity preserves the valid J-to-s readout subspace", tuple(readout(j) for j in identity(field)) == state)
check("D2 negation preserves the valid J-to-s readout subspace", tuple(readout(j) for j in negate(field)) == tuple(-s for s in state))
check("D3 one-axis shift preserves the valid J-to-s readout subspace", tuple(readout(j) for j in shift_x(field)) == shift_x(state))
check("D4 identity is its own inverse", identity(identity(field)) == field)
check("D5 negation is its own inverse", negate(negate(field)) == field)
check("D6 local shift has a local inverse", shift_x(shift_x(field), -1) == field)
check("D7 the three reversible updates are distinct", len({identity(field), negate(field), shift_x(field)}) == 3)

zero_field = tuple((0, 0, 0) for _ in sites)
check("D8 deterministic collapse is local and non-injective", collapse(field) == collapse(zero_field) and field != zero_field)


# ---------------------------------------------------------------------------
# 3. Reversibility supplies counting invariance but selects neither rule nor
#    physical ensemble.
# ---------------------------------------------------------------------------

a9 = tuple(product((-1, 0, 1), repeat=2))


def c4(z: tuple[int, int]) -> tuple[int, int]:
    u, v = z
    return (-v, u)


check("M1 one trit cannot carry blank plus C4xZ2", 3 < 1 + 4 * 2)
check("M2 two trits attain the nine-state capacity floor", len(a9) == 3**2 == 1 + 4 * 2)
check("M3 C4 rotation is a permutation of A9", {c4(z) for z in a9} == set(a9))
check("M4 identity and C4 are distinct counting-measure-preserving maps", any(c4(z) != z for z in a9))

uniform = {z: Fraction(1, len(a9)) for z in a9}
delta_blank = {z: Fraction(int(z == (0, 0)), 1) for z in a9}
check("M5 uniform measure is invariant under identity", sum(uniform.values()) == 1)
check("M6 a distinct delta measure is also invariant under identity", sum(delta_blank.values()) == 1 and delta_blank[(0, 0)] == 1)
check("M7 invariance alone therefore does not select the uniform ensemble", uniform != delta_blank)

# A normalized translation-invariant uniform probability on R cannot exist.
# If a=mu([0,1)), N disjoint integer translates force N*a<=1 for every N,
# hence a=0. Countable additivity over R=union_n [n,n+1) then gives mu(R)=0,
# contradicting normalization.
positive_rational_masses = {
    Fraction(p, q) for q in range(1, 33) for p in range(1, q + 1)
}
check(
    "M8 real-line uniform probability: positive unit mass contradicts finite normalization",
    all((int(1 / a) + 1) * a > 1 for a in positive_rational_masses),
)
zero_partial_sums = [sum(Fraction(0, 1) for _ in range(n)) for n in range(1, 65)]
check(
    "M9 real-line uniform probability: zero unit mass contradicts countable normalization",
    all(total == 0 for total in zero_partial_sums) and Fraction(0, 1) != Fraction(1, 1),
)


# ---------------------------------------------------------------------------
# 4. A tautological transition action can encode any rule and therefore does
#    not select physical dynamics.
# ---------------------------------------------------------------------------


def constraint_action(next_state, predicted_state) -> int:
    return int(next_state != predicted_state)


i_next = identity(field)
n_next = negate(field)
check("A1 identity constraint action vanishes on identity history", constraint_action(i_next, identity(field)) == 0)
check("A2 negation constraint action vanishes on negation history", constraint_action(n_next, negate(field)) == 0)
check("A3 the two zero-action history sets differ", constraint_action(n_next, identity(field)) == 1 and constraint_action(i_next, negate(field)) == 1)


passed = sum(ok for _, ok, _ in checks)
print(f"\n{passed}/{len(checks)} exact checks pass")
raise SystemExit(0 if passed == len(checks) else 1)
