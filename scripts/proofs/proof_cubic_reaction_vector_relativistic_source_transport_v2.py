#!/usr/bin/env python3
"""FTD-0890 three-expression repair wrapper for the FTD-0889 certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_CUBIC_REACTION_VECTOR_RELATIVISTIC_SOURCE_TRANSPORT_v1.md"
)
PARENT_CERTIFICATE = ROOT / (
    "scripts/proofs/proof_cubic_reaction_vector_relativistic_source_transport.py"
)
REPAIR_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_CUBIC_REACTION_VECTOR_SOURCE_TRANSPORT_CERTIFICATE_REPAIR_v2.md"
)

PARENT_PROTOCOL_HASH = "A92F0BFB95993971AB80661B39296E948BA68E52ADED6D4A3DAF92804DB37F66"
PARENT_CERTIFICATE_HASH = "D8A8D80E1E6E497C08E7011ED7731E27C2B0B221EB894D3E9C8A61C89CF1EA0F"
REPAIR_PROTOCOL_HASH = "F4D8416C0AD1196070EFAEFF0DDEE4A2BA626252309142E9E44568EC15E7CF82"

REPAIRS = (
    (
        '''check("reaction norm maps exactly to relativistic kinetic energy",
      sp.simplify(kinetic_from_chart - rho**2 / 2) == 0)''',
        '''check("reaction norm maps exactly to relativistic kinetic energy",
      sp.simplify(E0**2 + c**2 * alpha**2 * rho**2
                  - (E0 + rho**2 / 2)**2) == 0
      and (E0 + rho**2 / 2).is_positive is True)''',
    ),
    (
        '''check("compatibility interval maps uniquely into eta in zero to pi over two",
      sp.diff(sp.asin(sp.sqrt(sp.symbols("z", positive=True))),
              sp.symbols("z", positive=True)).is_positive is True)''',
        '''t_interval = sp.symbols("t_interval", positive=True)
z_interval = t_interval / (1 + t_interval)
eta_interval_derivative = sp.simplify(
    sp.diff(sp.asin(sp.sqrt(z_interval)), t_interval))
check("compatibility interval maps uniquely into eta in zero to pi over two",
      eta_interval_derivative == 1 / (
          2 * sp.sqrt(t_interval) * (1 + t_interval))
      and eta_interval_derivative.is_positive is True)''',
    ),
    (
        '''check("mass scale and common-action coupling remain open",
      "does not determine `E0`, `c`," in PROTOCOL.read_text(encoding="utf-8")
      and "full common-action coupling" in PROTOCOL.read_text(encoding="utf-8"))''',
        '''protocol_flat = " ".join(PROTOCOL.read_text(encoding="utf-8").split())
check("mass scale and common-action coupling remain open",
      "does not determine `E0`, `c`," in protocol_flat
      and "full common-action coupling" in protocol_flat)''',
    ),
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def fail(message: str) -> None:
    raise SystemExit(f"FTD-0890 REPAIR INVALID: {message}")


if sha256(PARENT_PROTOCOL) != PARENT_PROTOCOL_HASH:
    fail("parent protocol hash mismatch")
if sha256(PARENT_CERTIFICATE) != PARENT_CERTIFICATE_HASH:
    fail("parent certificate hash mismatch")
if sha256(REPAIR_PROTOCOL) != REPAIR_PROTOCOL_HASH:
    fail("repair protocol hash mismatch")

source = PARENT_CERTIFICATE.read_text(encoding="utf-8")
for index, (old, new) in enumerate(REPAIRS, start=1):
    if source.count(old) != 1:
        fail(f"R{index} expected one old anchor, found {source.count(old)}")
    if source.count(new) != 0:
        fail(f"R{index} replacement already present in frozen parent")
    source = source.replace(old, new, 1)
    if source.count(old) != 0 or source.count(new) != 1:
        fail(f"R{index} post-substitution uniqueness check failed")

print("FTD-0890 repair preflight: parent/protocol hashes and three unique anchors PASS")
namespace = {
    "__name__": "__main__",
    "__file__": str(PARENT_CERTIFICATE),
}
exec(compile(source, str(PARENT_CERTIFICATE), "exec"), namespace)
