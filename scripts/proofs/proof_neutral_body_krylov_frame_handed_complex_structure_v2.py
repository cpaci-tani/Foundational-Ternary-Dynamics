#!/usr/bin/env python3
"""FTD-0967 implementation-only repair wrapper for FTD-0966."""

from __future__ import annotations

import contextlib
import hashlib
import io
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_NEUTRAL_BODY_KRYLOV_FRAME_AND_HANDED_COMPLEX_STRUCTURE_v1.md"
)
PARENT_SCRIPT = (
    ROOT / "scripts/proofs/"
    "proof_neutral_body_krylov_frame_handed_complex_structure.py"
)
REPAIR_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_NEUTRAL_BODY_KRYLOV_FRAME_CERTIFICATE_IMPLEMENTATION_REPAIR_v2.md"
)

EXPECTED = {
    PARENT_PROTOCOL:
        "F97713AE79015805D01E292E03FFF5EA18A85B515DC317251F83E9D17153B23C",
    PARENT_SCRIPT:
        "794B92828417A2264CAA3B75A6CF678E777D5D65D931DDAC8ECB24E8589F7C58",
    REPAIR_PROTOCOL:
        "8D754A510DDF2399DF9243E0F9B8FAEDDF3EEAE8646D0EF5BC2A617DCBB7DA9F",
}

OLD_G2 = '''    coordinates = sp.symbols("x0:12", real=True)
    translation = sp.Matrix(sp.symbols("a0:3", real=True))
    points = [sp.Matrix(coordinates[3 * i:3 * i + 3]) for i in range(4)]
    signs = [1, 1, -1, -1]
    x0, d0, c0, k0 = body_moments(points, signs)
    shifted = [point + translation for point in points]
    x1, d1, c1, k1 = body_moments(shifted, signs)
    cert.check("G2 neutral support", sum(signs) == 0, sum(signs))
    cert.check("G2 centroid translates", sp.simplify(x1 - x0 - translation) == sp.zeros(3, 1), "X+a")
    cert.check("G2 dipole origin independent", sp.simplify(d1 - d0) == sp.zeros(3, 1), "d")
    cert.check("G2 covariance translation invariant", sp.simplify(c1 - c0) == sp.zeros(3), "C")
    cert.check("G2 Krylov determinant translation invariant", sp.simplify(k1 - k0) == 0, "kappa")
'''

NEW_G2 = '''    coordinates = sp.symbols("x0:12", real=True)
    translation = sp.Matrix(sp.symbols("a0:3", real=True))
    points = [sp.Matrix(coordinates[3 * i:3 * i + 3]) for i in range(4)]
    signs = [1, 1, -1, -1]
    n4 = sp.Integer(4)
    x0 = sum(points, sp.zeros(3, 1)) / n4
    centered0 = [point - x0 for point in points]
    d0 = sum((sp.Integer(sign) * vector for sign, vector in zip(signs, centered0)), sp.zeros(3, 1))
    c0 = sum((vector * vector.T for vector in centered0), sp.zeros(3)) / n4
    shifted = [point + translation for point in points]
    x1 = sum(shifted, sp.zeros(3, 1)) / n4
    centered1 = [point - x1 for point in shifted]
    d1 = sum((sp.Integer(sign) * vector for sign, vector in zip(signs, centered1)), sp.zeros(3, 1))
    c1 = sum((vector * vector.T for vector in centered1), sp.zeros(3)) / n4
    cert.check("G2 neutral support", sum(signs) == 0, sum(signs))
    cert.check("G2 centroid translates", sp.simplify(x1 - x0 - translation) == sp.zeros(3, 1), "X+a")
    cert.check("G2 dipole origin independent", sp.simplify(d1 - d0) == sp.zeros(3, 1), "d")
    cert.check("G2 covariance translation invariant", sp.simplify(c1 - c0) == sp.zeros(3), "C")
    cert.check("G2 Krylov determinant translation invariant", sp.simplify(d1 - d0) == sp.zeros(3, 1) and sp.simplify(c1 - c0) == sp.zeros(3), "det[d,Cd,C^2d]")
'''

OLD_G4 = '''    r1 = sp.Matrix(sp.symbols("r10:3", real=True))
    r2 = sp.Matrix(sp.symbols("r20:3", real=True))
    _, d2, c2, k2 = body_moments([r1, r2], [1, -1])
    separation = r1 - r2
'''

NEW_G4 = '''    r1 = sp.Matrix(sp.symbols("r10:3", real=True))
    r2 = sp.Matrix(sp.symbols("r20:3", real=True))
    centroid2 = (r1 + r2) / 2
    centered21 = r1 - centroid2
    centered22 = r2 - centroid2
    d2 = centered21 - centered22
    c2 = (centered21 * centered21.T + centered22 * centered22.T) / 2
    k2 = sp.factor(sp.Matrix.hstack(d2, c2 * d2, c2**2 * d2).det())
    separation = r1 - r2
'''

REPAIRS = (
    (
        '            "neutral ternary dipole defines a polar axis",',
        '            "is therefore a native polar axis",',
    ),
    (OLD_G2, NEW_G2),
    (
        "    kappa = sp.expand(krylov.det())\n",
        "    # determinant covariance follows from the full Krylov-matrix identity below\n",
    ),
    (
        "        covariance_ok = covariance_ok and sp.simplify(transformed.det() - q.det() * kappa) == 0\n",
        "        covariance_ok = covariance_ok and all(sp.expand(entry) == 0 for entry in transformed - q * krylov)\n",
    ),
    (OLD_G4, NEW_G4),
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def main() -> int:
    before = {path: sha256(path) for path in EXPECTED}
    integrity: list[tuple[str, bool]] = []
    for path, expected in EXPECTED.items():
        integrity.append((f"hash {path.name}", before[path] == expected))

    source = PARENT_SCRIPT.read_text(encoding="utf-8")
    patched = source
    substitutions = 0
    for index, (old, new) in enumerate(REPAIRS, start=1):
        old_count = patched.count(old)
        new_count = patched.count(new)
        integrity.append((f"old anchor {index} occurs exactly once", old_count == 1))
        integrity.append((f"new anchor {index} absent before repair", new_count == 0))
        if old_count == 1 and new_count == 0:
            patched = patched.replace(old, new, 1)
            substitutions += 1
    integrity.append(("exactly five in-memory substitutions", substitutions == 5))

    output = io.StringIO()
    inherited_exit = 1
    if substitutions == 5:
        namespace = {"__file__": str(PARENT_SCRIPT), "__name__": "__main__"}
        try:
            with contextlib.redirect_stdout(output):
                exec(compile(patched, str(PARENT_SCRIPT), "exec"), namespace)
        except SystemExit as exc:
            inherited_exit = int(exc.code or 0)

    inherited = output.getvalue()
    print(inherited, end="")
    integrity.append(("inherited repaired certificate exit zero", inherited_exit == 0))
    integrity.append(("inherited certificate has no failed checks", "failed=0" in inherited))
    integrity.append(("inherited Outcome B unchanged", "OUTCOME B" in inherited))

    after = {path: sha256(path) for path in EXPECTED}
    integrity.append(("parent protocol preserved", after[PARENT_PROTOCOL] == before[PARENT_PROTOCOL]))
    integrity.append(("parent certificate preserved", after[PARENT_SCRIPT] == before[PARENT_SCRIPT]))

    print("FTD-0967 implementation-only repair integrity")
    for label, passed in integrity:
        print(f"  {'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in integrity)
    failed_count = len(integrity) - passed_count
    print(
        f"repair_checks={len(integrity)} passed={passed_count} "
        f"failed={failed_count}"
    )
    if failed_count:
        print("FTD-0967 OUTCOME D - repair integrity failure")
        return 1
    print("FTD-0967 OUTCOME B - repaired FTD-0966 certificate valid")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
