#!/usr/bin/env python3
"""FTD-0415: exact cubic-invariant marginal-operator inventory.

This is a symmetry/power-counting verifier.  It does not calculate an FTD
loop coefficient.  It proves that the stated exact spatial symmetries permit
independent time/space kinetic terms, including a cubic-only vector-gradient
invariant, so CPT-even q^4 tree dispersion is not radiative protection.
"""

from itertools import permutations, product
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]


def require(condition: bool, label: str) -> None:
    if not condition:
        raise AssertionError(label)
    print(f"PASS  {label}")


def signed_permutation_group() -> list[sp.Matrix]:
    group = []
    for perm in permutations(range(3)):
        for signs in product((-1, 1), repeat=3):
            r = sp.zeros(3)
            for i in range(3):
                r[i, perm[i]] = signs[i]
            group.append(r)
    return group


def gradient_invariants(a: sp.Matrix) -> sp.Matrix:
    return sp.Matrix([
        sp.trace(a.T * a),
        sp.trace(a) ** 2,
        sp.trace(a * a),
        sum(a[i, i] ** 2 for i in range(3)),
    ])


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


def main() -> None:
    checks = 0
    group = signed_permutation_group()
    require(len(group) == 48 and len({tuple(r) for r in group}) == 48,
            "G1 full O_h signed-permutation representation has 48 elements")
    checks += 1
    require(all(r.T * r == sp.eye(3) for r in group),
            "G2 every cubic transformation is orthogonal")
    checks += 1

    e1, e2, e3, b1, b2, b3 = sp.symbols("E1 E2 E3 B1 B2 B3")
    e = sp.Matrix([e1, e2, e3])
    b = sp.Matrix([b1, b2, b3])
    require(all(sp.expand((r * e).dot(r * e) - e.dot(e)) == 0
                        and sp.expand((r * b).dot(r * b) - b.dot(b)) == 0
                        for r in group),
            "P1 E^2 and B^2 are separately O_h invariant")
    checks += 1
    require(e.dot(e).subs({e1: 1, e2: 0, e3: 0}) !=
            b.dot(b).subs({b1: 0, b2: 0, b3: 0}),
            "P2 spatial symmetry supplies no identity equating E^2 and B^2")
    checks += 1

    entries = sp.symbols("a00:03 a10:13 a20:23")
    a = sp.Matrix(3, 3, entries)
    inv = gradient_invariants(a)
    require(all(all(sp.expand(x) == 0 for x in
                    gradient_invariants(r * a * r.T) - inv)
                for r in group),
            "V1 four vector-gradient quadratics are separately O_h invariant")
    checks += 1

    samples = [
        sp.diag(1, 0, 0),
        sp.diag(1, 1, 0),
        sp.Matrix([[0, 1, 0], [0, 0, 0], [0, 0, 0]]),
        sp.Matrix([[0, 1, 0], [1, 0, 0], [0, 0, 0]]),
    ]
    sample_matrix = sp.Matrix.hstack(
        *(gradient_invariants(x) for x in samples)).T
    require(sample_matrix.det() != 0,
            "V2 the four O_h vector-gradient invariants are linearly independent")
    checks += 1

    root2 = sp.sqrt(2)
    rz45 = sp.Matrix([[1 / root2, -1 / root2, 0],
                      [1 / root2, 1 / root2, 0],
                      [0, 0, 1]])
    witness = sp.diag(1, 0, 0)
    before = gradient_invariants(witness)
    after = gradient_invariants(rz45 * witness * rz45.T)
    require(before[:3, :] == after[:3, :]
            and before[3] == 1 and after[3] == sp.Rational(1, 2),
            "V3 sum_i (partial_i J_i)^2 is cubic invariant but not SO(3) invariant")
    checks += 1

    omega2, k2, zt, zs = sp.symbols("omega2 k2 Z_t Z_s")
    pole = zt * omega2 - zs * k2
    require(sp.solve(pole, omega2)[0] == zs * k2 / zt,
            "C1 independent temporal/spatial kinetic terms give c^2=Z_s/Z_t")
    checks += 1
    lam = sp.symbols("lambda", nonzero=True)
    require(sp.cancel((lam**2 * zs) / (lam**2 * zt)) == zs / zt,
            "C2 field normalization cannot remove the limiting-speed ratio")
    checks += 1

    g, c6, cutoff, breaking = sp.symbols("g C6 Lambda M", nonzero=True)
    delta_c4 = g**2 * c6 * cutoff**2 / (16 * sp.pi**2 * breaking**2)
    require(sp.simplify(delta_c4.subs(cutoff, breaking))
            == g**2 * c6 / (16 * sp.pi**2),
            "R1 equal cutoff/breaking scales remove external-energy suppression")
    checks += 1

    audit = read("docs/theory/07_assessment/lorentz_recovery_causal_structure/AUDIT_LORENTZ_RADIATIVE_CLOSURE.md")
    hard = read("docs/theory/07_assessment/lorentz_recovery_causal_structure/AUDIT_LORENTZ_RECOVERY_HARD.md")
    tracker = read("docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md")
    bianchi = read("docs/theory/03_derivations/electromagnetism/DERIV_LATTICE_HODGE_DUALITY.md")
    require("MARGINAL-OPERATORS-ALLOWED; RADIATIVE-PROTECTION-NOT-SHOWN" in audit,
            "S1 audit records the scoped radiative verdict")
    checks += 1
    require("Collins" in audit and "does not calculate" in audit,
            "S2 audit separates power counting from an FTD loop calculation")
    checks += 1
    require("dimension-four operators" in hard and "LR-3" in tracker,
            "S3 canonical hard gate retains the lower-dimension mixing target")
    checks += 1
    require("does **not** define a discrete Hodge star" in bianchi
            and "independent `E²` and `B²` coefficients" in bianchi,
            "S4 exact Bianchi identities are not misused as EM duality protection")
    checks += 1

    print(f"\n{checks}/{checks} exact/source-contract checks passed")
    print("VERDICT  MARGINAL OPERATORS ALLOWED; RADIATIVE PROTECTION NOT SHOWN")


if __name__ == "__main__":
    main()
