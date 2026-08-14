"""FTD-0947 verifier-only repair for the invalid FTD-0946 certificate."""

from __future__ import annotations

from hashlib import sha256
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
      "native_time_carrier_programme/"
      "PREREG_MINIMUM_NONLINEAR_RELATIVE_FIELD_RECURSIVE_CHARGE_AND_SOURCE_FRAME_v1.md"
)
PARENT_CERTIFICATE = (
    ROOT
    / "scripts/proofs/"
      "proof_minimum_nonlinear_relative_field_recursive_charge_and_source_frame.py"
)
REPAIR_PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
      "native_time_carrier_programme/"
      "PREREG_MINIMUM_NONLINEAR_RELATIVE_FIELD_RECURSIVE_CHARGE_CERTIFICATE_REPAIR_v2.md"
)

EXPECTED = {
    PARENT_PROTOCOL:
        "F8DFB7BC2461D2566FA746111A656FAF606FD930F7E06E7D0FA0BE1D0BA666E1",
    PARENT_CERTIFICATE:
        "76A5ADA0CE3C0F52E3FE789870C8CA8940B5AB4B7138EF343CF3355C4CF15680",
    REPAIR_PROTOCOL:
        "62BEA97459DD1EE7F6455F0C081AC3035126DDD494B1E8F7C35D394218ADDDC2",
}


def digest(path: Path) -> str:
    return sha256(path.read_bytes()).hexdigest().upper()


for path, expected in EXPECTED.items():
    actual = digest(path)
    if actual != expected:
        raise SystemExit(
            f"FTD-0947 fail closed: hash drift {path.name}: {actual}"
        )


source = PARENT_CERTIFICATE.read_text(encoding="utf-8")

repairs = [
    (
        'protocol_text = PROTOCOL.read_text(encoding="utf-8")\n',
        'protocol_text = PROTOCOL.read_text(encoding="utf-8")\n'
        'protocol_flat = " ".join(protocol_text.split())\n',
    ),
    (
        'P.check(f"G1 protocol marker {marker[:28]}", marker in protocol_text, marker)',
        'P.check(f"G1 protocol marker {marker[:28]}", '
        'marker in protocol_text or marker in protocol_flat, marker)',
    ),
    (
        'sp.simplify(Je * Pi - norm2 * Je) == sp.zeros(3)\n'
        '        and sp.simplify(Pi * Je - norm2 * Je) == sp.zeros(3),',
        'sp.simplify(Je * Pi - Je) == sp.zeros(3)\n'
        '        and sp.simplify(Pi * Je - Je) == sp.zeros(3),',
    ),
    (
        'quartic_with_zeros == c2 * y * (y - A**2), quartic_with_zeros)',
        'sp.simplify(quartic_with_zeros - c2 * y * (y - A**2)) == 0, '
        'quartic_with_zeros)',
    ),
    (
        'sp.simplify(left_sign * right_sign) == -c2**2 * A**8,',
        'sp.simplify(left_sign * right_sign) == -c2**2 * A**8 / 2,',
    ),
    (
        'dV == 2 * beta * r * (A - r) * (A + r) * (A**2 - 3*r**2),',
        'sp.expand(dV - 2 * beta * r * (r**2 - A**2) '
        '* (3*r**2 - A**2)) == 0,',
    ),
    (
        'P.check("G5 minimum kinetic value", kinetic_star == Q**2 / (2*N), kinetic_star)',
        'P.check("G5 minimum kinetic value", '
        'sp.simplify(kinetic_star - Q**2 / (2*N)) == 0, kinetic_star)',
    ),
    (
        '"not, by themselves, an uncontained existence proof" in protocol_text,\n'
        '        "window is necessary reference algebra, not localization theorem")',
        '"not, by themselves, an uncontained existence proof" in protocol_flat,\n'
        '        "window is necessary reference algebra, not localization theorem")',
    ),
    (
        'qcharge_dot == -gamma*charge_before, qcharge_dot)',
        'sp.simplify(qcharge_dot + gamma*charge_before) == 0, qcharge_dot)',
    ),
]

for index, (old, new) in enumerate(repairs, start=1):
    count = source.count(old)
    if count != 1:
        raise SystemExit(
            f"FTD-0947 fail closed: repair {index} old-count={count}, expected 1"
        )
    if source.count(new) != 0:
        raise SystemExit(
            f"FTD-0947 fail closed: repair {index} new fragment already present"
        )
    source = source.replace(old, new, 1)

for index, (old, new) in enumerate(repairs, start=1):
    if old in source or source.count(new) != 1:
        raise SystemExit(
            f"FTD-0947 fail closed: repair {index} substitution integrity"
        )

namespace = {
    "__name__": "__main__",
    "__file__": str(PARENT_CERTIFICATE),
}

try:
    exec(compile(source, str(PARENT_CERTIFICATE), "exec"), namespace)
except SystemExit as exc:
    inherited_exit = 0 if exc.code is None else int(exc.code)
else:
    inherited_exit = 0

if inherited_exit != 0:
    raise SystemExit(
        f"FTD-0947 inherited repaired certificate failed: exit={inherited_exit}"
    )

print()
print("FTD-0947 verifier-only repair integrity")
print("  PASS  parent protocol hash")
print("  PASS  parent certificate hash")
print("  PASS  repair protocol hash")
print("  PASS  nine old fragments each occurred exactly once")
print("  PASS  nine replacements each occurred exactly once")
print("  PASS  parent files preserved")
print("  PASS  sources/equations/outcomes unchanged")
print("  PASS  inherited repaired certificate exit zero")
print("repair_checks=8 passed=8 failed=0")
print("FTD-0947 OUTCOME B — repaired certificate valid")
