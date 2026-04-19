"""
report.py — Shared banner/formatting helpers for exploration scripts.

Extracted from the duplicated definitions in:
  scripts/exploration/star_operator.py
  scripts/exploration/i_from_star.py
  (also present, in slightly divergent forms:
   coefficient_16_investigation.py, ternary_cube_tests.py,
   investigate_lemniscate_deep.py — see notes below)

Every one of those scripts redefined `header`, `subheader`, `fmt`,
`fmt_short`, and (in some cases) `continued_fraction`. The versions are
byte-equivalent in the first two files, so those can import from here
directly. The three other files have minor variations (different section
signatures / number arguments); we document them but leave them
unchanged unless the audit explicitly calls them out.

Nothing here alters scientific logic.
"""
from __future__ import annotations

from typing import Iterable

try:
    import mpmath
    from mpmath import mpf, floor
    _HAS_MPMATH = True
except ImportError:  # pragma: no cover
    mpmath = None  # type: ignore
    _HAS_MPMATH = False


SEP = "=" * 80
SUB = "-" * 60


def header(title: str) -> None:
    """Top-level banner (double-rule, padded title)."""
    print(f"\n{SEP}")
    print(f"  {title}")
    print(SEP)


def subheader(title: str) -> None:
    """Short dash-delimited subsection marker."""
    print(f"\n--- {title} ---")


def section(title: str) -> None:
    """Blank-line + double-rule section marker (used by coefficient_16_*)."""
    print(f"\n\n{SEP}")
    print(f"  {title}")
    print(SEP)


def fmt(x, digits: int = 40) -> str:
    """Format an mpmath number to `digits` significant figures."""
    if not _HAS_MPMATH:
        raise RuntimeError(
            "mpmath is required for scripts.common.report.fmt(). "
            "Install with: pip install mpmath"
        )
    return mpmath.nstr(x, digits)


def fmt_short(x, digits: int = 15) -> str:
    """Shorter version of fmt()."""
    if not _HAS_MPMATH:
        raise RuntimeError(
            "mpmath is required for scripts.common.report.fmt_short()."
        )
    return mpmath.nstr(x, digits)


def continued_fraction(x, n_terms: int = 20) -> list[int]:
    """
    Simple continued-fraction expansion of `x` (mpmath number) out to
    `n_terms` partial quotients. Stops early if the residual falls below
    10^-80 (matches star_operator.py convention).
    """
    if not _HAS_MPMATH:
        raise RuntimeError(
            "mpmath is required for scripts.common.report.continued_fraction()."
        )
    cfs: list[int] = []
    for _ in range(n_terms):
        a = floor(x)
        cfs.append(int(a))
        frac = x - a
        if abs(frac) < mpf(10) ** (-80):
            break
        x = 1 / frac
    return cfs
