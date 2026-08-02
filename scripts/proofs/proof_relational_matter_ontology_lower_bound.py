"""Exact certificate for FTD-0741's relational-matter ontology lower bound."""

from __future__ import annotations

import hashlib
from fractions import Fraction as Q
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SOURCES = {
    "docs/theory/10_eft_program/derivations/THEOREM_MATTER_FIELD_MEMORY_KERNEL.md":
        "404A757087233C606A4D82D7522E1E96648D6EF16036EAC5492CD6AA76AB4726",
    "docs/theory/10_eft_program/derivations/THEOREM_POLARITY_SNAPSHOT_CURRENT_NONUNIQUENESS.md":
        "FCB92C607A6D8FFC15F384D7D8DCE9943EB733786AC546B5FC3041C59D2B9EC6",
    "docs/theory/10_eft_program/derivations/THEOREM_CONFIGURATION_SPACE_CARRIER_NECESSITY.md":
        "9FCD2E7AA89C8B38339D730B04AAD2A9797F40E3EDD08ACA3B5C9CFCB4996FBD",
    "docs/theory/10_eft_program/derivations/THEOREM_RELATIONAL_ENTRY_PRECEDES_ENERGETIC_BINDING_v1.md":
        "9595C2C83A271BAFB0A696C999C89B235B6CEF1EB57CEE2970A4839BFB9E6322",
}

checks: list[tuple[str, bool]] = []


def check(name: str, condition: bool) -> None:
    checks.append((name, bool(condition)))


for relative, expected in SOURCES.items():
    actual = hashlib.sha256((ROOT / relative).read_bytes()).hexdigest().upper()
    check(f"source hash: {Path(relative).name}", actual == expected)

# Oriented square incidence: every edge follows 0->1->2->3->0.
D = [
    [Q(-1), Q(0), Q(0), Q(1)],
    [Q(1), Q(-1), Q(0), Q(0)],
    [Q(0), Q(1), Q(-1), Q(0)],
    [Q(0), Q(0), Q(1), Q(-1)],
]
cycle = [Q(1), Q(1), Q(1), Q(1)]
div_cycle = [sum(row[j] * cycle[j] for j in range(4)) for row in D]
check("square cycle is divergence-free", div_cycle == [Q(0)] * 4)
check("square cycle is nonzero", any(value != 0 for value in cycle))
check("oriented circulation is nonzero", sum(cycle) == Q(4))

base_current = [Q(2), Q(-1), Q(3), Q(0)]
other_current = [base_current[i] + cycle[i] for i in range(4)]


def matvec(matrix: list[list[Q]], vector: list[Q]) -> list[Q]:
    return [sum(row[j] * vector[j] for j in range(len(vector))) for row in matrix]


check("same endpoint divergence", matvec(D, base_current) == matvec(D, other_current))
check("distinct currents", base_current != other_current)

# Exact memory-kernel witness: x'=y, y'=-x. Here A=D=0, B=1, C=-1.
A, B, C, field_D = Q(0), Q(1), Q(-1), Q(0)
K0 = B * C
check("memory kernel K0 is nonzero", K0 == Q(-1))
x = Q(0)
y_plus, y_minus = Q(1), Q(-1)
check("same matter/different field changes next matter",
      A * x + B * y_plus != A * x + B * y_minus)

# Verify the eliminated recurrence over one rotation orbit.
states = [(Q(1), Q(0))]
for _ in range(4):
    old_x, old_y = states[-1]
    states.append((A * old_x + B * old_y, C * old_x + field_D * old_y))
check("rotation is four-step recurrent", states[4] == states[0])
check("matter projection loses and revives", [s[0] for s in states] ==
      [Q(1), Q(0), Q(-1), Q(0), Q(1)])

# Fixed-source affine fibre F=(t,1-t); contraction to F0=(0,1).
for t in (Q(-3, 2), Q(0), Q(2, 5), Q(7, 3)):
    F = (t, Q(1) - t)
    check(f"affine source constraint t={t}", sum(F) == Q(1))
    for tau in (Q(0), Q(1, 3), Q(1)):
        H = (tau * F[0], Q(1) + tau * (F[1] - Q(1)))
        check(f"contraction preserves source t={t},tau={tau}", sum(H) == Q(1))

# Selected compact-pair boundary identities with well depth one.
def potential(d: Q) -> Q:
    return -Q(16) * (d - Q(3, 2)) ** 2 * (d - Q(3, 4))


def potential_prime(d: Q) -> Q:
    a = d - Q(3, 2)
    b = d - Q(3, 4)
    return -Q(16) * (Q(2) * a * b + a * a)


dc = Q(3, 2)
check("compact potential vanishes at entry", potential(dc) == 0)
check("compact force vanishes at entry", potential_prime(dc) == 0)
check("compact well minimum value", potential(Q(1)) == Q(-1))
kinetic = Q(1, 100)
check("moving entry energy is positive", kinetic + potential(dc) > 0)
pair_before, pair_after = Q(1, 100), Q(-1, 200)
field_gain = pair_before - pair_after
check("capture requires positive field gain", field_gain == Q(3, 200) and field_gain > 0)

failures = [name for name, passed in checks if not passed]
for name, passed in checks:
    print(f"{'PASS' if passed else 'FAIL'}  {name}")
print(f"\nFTD-0741: {len(checks) - len(failures)}/{len(checks)} checks passed")
if failures:
    raise SystemExit(1)
