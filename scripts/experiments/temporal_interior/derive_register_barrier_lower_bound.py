"""Machine-verifiable lower-bound certificate for the register barrier.

The register consists of three anchors at the vertices of an equilateral
triangle of side ``s`` and one mobile body.  Each anchor-body bond has law

    V(q) = -16 eps (q - 3/2)^2 (q - 3/4),  q < 3/2,
           0,                              q >= 3/2.

For ``s in [9/10, 13/10]`` the two mirror ground states have energy
``-3 eps``.  Every continuous path between them crosses the anchor plane,
so it is enough to prove that the energy in that plane is at least
``-2 eps``.  The proof has two parts:

* Analytically, ``V >= -eps`` everywhere and a bond can be negative only
  for ``3/4 < q < 3/2``.  Hence any point at which at most two bonds can
  be attractive already has energy at least ``-2 eps``.
* Closed boxes in ``(x, y, s)`` bound the squared distance ``q=d^2`` to
  every anchor.  Every primitive floating-point operation is widened by
  one binary64 ULP with ``numpy.nextafter``.  For the registered law these
  outward bounds show that no box permits all three bonds to be attractive;
  hence the analytic two-bond bound discharges every box.  No point samples
  are used.

Outside ``[-3/2, 3/2]^2`` the bond to the anchor at the origin has
``q=d^2 >= 9/4 > 3/2`` and is outside its attractive window, so the
analytic two-bond argument covers the rest of the infinite plane.  The
boxes partition the remaining rectangle and the full side-length interval
using exact rational endpoints.

Attainment is analytic rather than sampled.  A point at unit distance from
two anchors has third distance

    d3(s) = (sqrt(3)/2)s + sqrt(1 - s^2/4).

Its derivative is positive whenever ``s^2 < 3``.  This holds throughout
the declared interval, and at its left endpoint

    d3(9/10) = (9 sqrt(3) + sqrt(319))/20
             > (9*(17/10) + 17)/20 = 323/200 > 3/2.

Thus ``q_3=d_3^2>3/2``: the third bond is identically zero and the plane minimum ``-2 eps``
is attained for every allowed ``s``.  Combined with the topological plane
crossing and the explicit hinge upper bound, the register barrier is
exactly ``eps`` within this declared toy model.

Run with ``--json`` to emit only the machine-readable certificate.  A
human-readable audit followed by the same JSON object is the default.

Scope: the certificate is conditional on the declared compact-support
bond law, geometry, and the IEEE-754 assumptions checked at runtime.  It
does not establish a substrate result.  Compact support is load-bearing.
"""
from __future__ import annotations

import argparse
from fractions import Fraction
import hashlib
import json
from pathlib import Path
import platform
import sys
from typing import Any

import numpy as np


EPS = 1.0
R0 = 1.0
Q_SUP = 1.5
Q_REP = 0.75

# Exact rational partition.  The xy square is [-3/2, 3/2]^2 and the
# side-length interval is [9/10, 13/10].
XY_DEN = 250                 # xy cell width = 1/250 = 0.004
XY_MIN_NUM = -375            # -375/250 = -3/2
XY_MAX_NUM = 375             #  375/250 =  3/2
S_DEN = 50                   # s slab width = 1/50 = 0.02
S_MIN_NUM = 45               # 45/50 = 9/10
S_MAX_NUM = 65               # 65/50 = 13/10

NEG_INF = np.float64(-np.inf)
POS_INF = np.float64(np.inf)


def V(q):
    """Bond law, retained for diagnostics (not used as an interval bound)."""
    q = np.asarray(q, dtype=np.float64)
    return np.where(q < Q_SUP,
                    -16.0 * EPS * (q - 1.5) ** 2 * (q - 0.75),
                    0.0)


# ---------------------------------------------------------------------
# Directed outward rounding for nonnegative interval computations
# ---------------------------------------------------------------------
def rd(value):
    """One-ULP rounding toward -infinity."""
    return np.nextafter(np.asarray(value, dtype=np.float64), NEG_INF)


def ru(value):
    """One-ULP rounding toward +infinity."""
    return np.nextafter(np.asarray(value, dtype=np.float64), POS_INF)


def add_lo(a, b):
    return rd(np.asarray(a, dtype=np.float64) + np.asarray(b, dtype=np.float64))


def add_hi(a, b):
    return ru(np.asarray(a, dtype=np.float64) + np.asarray(b, dtype=np.float64))


def sub_lo(a, b):
    return rd(np.asarray(a, dtype=np.float64) - np.asarray(b, dtype=np.float64))


def sub_hi(a, b):
    return ru(np.asarray(a, dtype=np.float64) - np.asarray(b, dtype=np.float64))


def mul_nonnegative_lo(a, b):
    """Lower product bound when both exact operands are nonnegative."""
    product = rd(np.asarray(a, dtype=np.float64) *
                 np.asarray(b, dtype=np.float64))
    # nextafter(0, -inf) is negative; the exact product cannot be.
    return np.maximum(product, 0.0)


def mul_nonnegative_hi(a, b):
    """Upper product bound when both exact operands are nonnegative."""
    return ru(np.asarray(a, dtype=np.float64) *
              np.asarray(b, dtype=np.float64))


def sqrt_lo(a):
    root = rd(np.sqrt(np.maximum(np.asarray(a, dtype=np.float64), 0.0)))
    return np.maximum(root, 0.0)


def sqrt_hi(a):
    return ru(np.sqrt(np.maximum(np.asarray(a, dtype=np.float64), 0.0)))


def rational_lo(numerator, denominator):
    """Binary64 lower enclosure of an exact, small rational."""
    return rd(np.asarray(numerator, dtype=np.float64) /
              np.float64(denominator))


def rational_hi(numerator, denominator):
    """Binary64 upper enclosure of an exact, small rational."""
    return ru(np.asarray(numerator, dtype=np.float64) /
              np.float64(denominator))


def verify_arithmetic_model() -> dict[str, Any]:
    """Fail closed if the runtime is not the arithmetic model we certify."""
    info = np.finfo(np.float64)
    assert info.nmant == 52 and info.iexp == 11
    assert np.dtype(np.float64).itemsize == 8
    assert np.nextafter(np.float64(1.0), POS_INF) > 1.0
    assert np.nextafter(np.float64(1.0), NEG_INF) < 1.0
    assert np.sqrt(np.float64(4.0)) == 2.0
    assert np.float64(0.75) * 4.0 == 3.0
    assert np.float64(1.5) * 2.0 == 3.0
    return {
        "format": "IEEE-754 binary64",
        "rounding": "round-to-nearest primitives; one nextafter ULP outward",
        "mantissa_bits_including_hidden": 53,
        "exponent_bits": 11,
        "numpy_version": np.__version__,
        "checks_passed": True,
    }


# ---------------------------------------------------------------------
# Exact analytic components
# ---------------------------------------------------------------------
def verify_bond_law_exact() -> dict[str, Any]:
    """Verify the polynomial identities with exact rational arithmetic."""
    q0 = Fraction(3, 4)
    q1 = Fraction(1, 1)
    qsup = Fraction(3, 2)

    def v_fraction(q: Fraction) -> Fraction:
        if q >= qsup:
            return Fraction(0)
        return -16 * (q - qsup) ** 2 * (q - q0)

    assert v_fraction(q0) == 0
    assert v_fraction(q1) == -1
    assert v_fraction(qsup) == 0
    assert v_fraction(Fraction(1, 2)) == 4

    # V'(q) = -48(q-3/2)(q-1): decreasing up to q=1 and increasing
    # from q=1 to the compact-support endpoint.  Together with the sign
    # outside the attractive window, this proves V >= -1 globally.
    derivative_roots = (Fraction(1), Fraction(3, 2))
    curvature_at_one = Fraction(24)
    assert derivative_roots == (q1, qsup)
    assert curvature_at_one > 0

    return {
        "global_minimum": "V(1) = -eps",
        "negative_only_on": "3/4 < q < 3/2",
        "compact_support": "V(q) = 0 for q >= 3/2",
        "repulsive_checkpoint": "V(1/2) = 4 eps",
        "derivative_factorization": "V'(q) = -48 eps (q-3/2)(q-1)",
        "curvature_in_q_at_minimum": "d^2 V/dq^2 at q=1 is 24 eps",
        "radial_curvature_at_r_0": "d^2 V(r^2)/dr^2 at r=1 is 96 eps",
        "exact_rational_checks_passed": True,
    }


def verify_attainment_exact() -> dict[str, Any]:
    """Exact monotonic endpoint proof for the in-plane two-bond state."""
    s_hi = Fraction(13, 10)
    assert s_hi * s_hi < 3

    # sqrt(3) > 17/10 because (17/10)^2 < 3; sqrt(319) > 17.
    sqrt3_lower = Fraction(17, 10)
    sqrt319_lower = Fraction(17)
    assert sqrt3_lower * sqrt3_lower < 3
    assert sqrt319_lower * sqrt319_lower < 319

    d3_left_lower = (9 * sqrt3_lower + sqrt319_lower) / 20
    assert d3_left_lower == Fraction(323, 200)
    assert d3_left_lower > Fraction(3, 2)

    return {
        "formula": "d3(s) = (sqrt(3)/2)s + sqrt(1-s^2/4)",
        "monotonicity": "d3'(s) > 0 on [9/10,13/10] because s^2 < 3",
        "endpoint_exact_lower_bound": "d3(9/10) > 323/200 > 3/2",
        "consequence": "q3=d3^2>3/2, so the third bond is outside support; E_plane = -2 eps",
        "sampled_points_used": 0,
        "exact_rational_checks_passed": True,
    }


# ---------------------------------------------------------------------
# Outward-rounded box certificate
# ---------------------------------------------------------------------
def coordinate_separation_bounds(p_lo, p_hi, a_lo, a_hi):
    """Bounds on |p-a| for two closed one-dimensional intervals."""
    # The lower gap is zero when intervals overlap.  Outward inputs and
    # downward subtraction ensure it is never overestimated.
    gap = np.maximum(
        0.0,
        np.maximum(sub_lo(a_lo, p_hi), sub_lo(p_lo, a_hi)),
    )

    # The largest separation is attained at one of the two opposing ends.
    span = np.maximum(sub_hi(a_hi, p_lo), sub_hi(p_hi, a_lo))
    span = np.maximum(span, 0.0)
    return gap, span


def squared_distance_bounds(px_lo, px_hi, py_lo, py_hi,
                            ax_lo, ax_hi, ay_lo, ay_hi):
    """Outward bounds on q=d^2 between two rectangles."""
    gx, sx = coordinate_separation_bounds(px_lo, px_hi, ax_lo, ax_hi)
    gy, sy = coordinate_separation_bounds(py_lo, py_hi, ay_lo, ay_hi)

    d2_lo = add_lo(mul_nonnegative_lo(gx, gx),
                   mul_nonnegative_lo(gy, gy))
    d2_lo = np.maximum(d2_lo, 0.0)
    d2_hi = add_hi(mul_nonnegative_hi(sx, sx),
                   mul_nonnegative_hi(sy, sy))
    return d2_lo, d2_hi


def potential_at_point_lower(q):
    """Outward lower bound for V(q), with 3/4 <= q <= 3/2."""
    q = np.asarray(q, dtype=np.float64)
    left_factor = np.maximum(sub_hi(1.5, q), 0.0)
    right_factor = np.maximum(sub_hi(q, 0.75), 0.0)
    square = mul_nonnegative_hi(left_factor, left_factor)
    product = mul_nonnegative_hi(16.0, square)
    product = mul_nonnegative_hi(product, right_factor)
    return -product


def potential_interval_lower(a, b):
    """Outward lower bound on min(V([a,b])) from exact monotonicity.

    V decreases on [3/4,1], increases on [1,3/2], and is nonnegative
    outside that attractive window.  The selected endpoint is already an
    outward distance bound; ``potential_at_point_lower`` then rounds every
    nonnegative polynomial operation outward.
    """
    a = np.maximum(np.asarray(a, dtype=np.float64), 0.0)
    b = np.asarray(b, dtype=np.float64)
    out = np.zeros_like(a)

    straddles = (a <= 1.0) & (b >= 1.0)
    out[straddles] = -EPS

    left = (~straddles) & (b > Q_REP) & (b < 1.0)
    if np.any(left):
        out[left] = potential_at_point_lower(b[left])

    right = (~straddles) & (a > 1.0) & (a < Q_SUP)
    if np.any(right):
        out[right] = potential_at_point_lower(a[right])

    return out


def anchor_boxes(s_num_lo: int, s_num_hi: int):
    """Outward anchor-coordinate boxes for one exact rational s slab."""
    s_lo = rational_lo(s_num_lo, S_DEN)
    s_hi = rational_hi(s_num_hi, S_DEN)

    sqrt3_nearest = np.sqrt(np.float64(3.0))
    sqrt3_lo = rd(sqrt3_nearest)
    sqrt3_hi = ru(sqrt3_nearest)

    half_s_lo = mul_nonnegative_lo(s_lo, 0.5)
    half_s_hi = mul_nonnegative_hi(s_hi, 0.5)
    a3y_lo = mul_nonnegative_lo(
        mul_nonnegative_lo(s_lo, sqrt3_lo), 0.5)
    a3y_hi = mul_nonnegative_hi(
        mul_nonnegative_hi(s_hi, sqrt3_hi), 0.5)

    zero = np.float64(0.0)
    return (
        (zero, zero, zero, zero),
        (s_lo, s_hi, zero, zero),
        (half_s_lo, half_s_hi, a3y_lo, a3y_hi),
    )


def xy_cells():
    """Outward boxes for the exact rational xy partition."""
    nums = np.arange(XY_MIN_NUM, XY_MAX_NUM, dtype=np.int64)
    lo_1d = rational_lo(nums, XY_DEN)
    hi_1d = rational_hi(nums + 1, XY_DEN)

    px_lo, py_lo = np.meshgrid(lo_1d, lo_1d, indexing="ij")
    px_hi, py_hi = np.meshgrid(hi_1d, hi_1d, indexing="ij")
    return (px_lo.ravel(), px_hi.ravel(),
            py_lo.ravel(), py_hi.ravel())


def certify_s_slab(s_num_lo: int, s_num_hi: int,
                   cells: tuple[np.ndarray, ...]) -> dict[str, Any]:
    """Certify every xy cell for one closed exact rational s slab."""
    px_lo, px_hi, py_lo, py_hi = cells
    anchors = anchor_boxes(s_num_lo, s_num_hi)

    q_lows: list[np.ndarray] = []
    q_highs: list[np.ndarray] = []
    possible_count = np.zeros(px_lo.shape, dtype=np.uint8)

    for ax_lo, ax_hi, ay_lo, ay_hi in anchors:
        q_lo, q_hi = squared_distance_bounds(
            px_lo, px_hi, py_lo, py_hi,
            ax_lo, ax_hi, ay_lo, ay_hi,
        )
        q_lows.append(q_lo)
        q_highs.append(q_hi)
        possible_count += ((q_hi > Q_REP) & (q_lo < Q_SUP))

    candidate = possible_count == 3
    candidate_count = int(np.count_nonzero(candidate))
    exact_count = int(candidate.size - candidate_count)

    if candidate_count:
        e0 = potential_interval_lower(q_lows[0][candidate],
                                      q_highs[0][candidate])
        e1 = potential_interval_lower(q_lows[1][candidate],
                                      q_highs[1][candidate])
        e2 = potential_interval_lower(q_lows[2][candidate],
                                      q_highs[2][candidate])
        energy_lower = add_lo(add_lo(e0, e1), e2)
        interval_ok = energy_lower >= -2.0 * EPS
        interval_count = int(np.count_nonzero(interval_ok))
        uncertified = int(candidate_count - interval_count)
        worst = float(np.min(energy_lower))
    else:
        interval_count = 0
        uncertified = 0
        worst = None

    return {
        "s_interval": [f"{s_num_lo}/{S_DEN}", f"{s_num_hi}/{S_DEN}"],
        "cells": int(px_lo.size),
        "by_analytic_N_le_2": exact_count,
        "by_interval": interval_count,
        "uncertified": uncertified,
        "worst_energy_lower": worst,
        "worst_energy_lower_hex": None if worst is None else worst.hex(),
    }


def build_certificate(verbose: bool = True) -> dict[str, Any]:
    arithmetic = verify_arithmetic_model()
    bond = verify_bond_law_exact()
    attainment = verify_attainment_exact()
    cells = xy_cells()

    slab_results: list[dict[str, Any]] = []
    total_cells = 0
    total_exact = 0
    total_interval = 0
    total_uncertified = 0
    worst: float | None = None

    if verbose:
        print("Register barrier lower-bound certificate")
        print("  exact domain: x,y in [-3/2,3/2], s in [9/10,13/10]")
        print("  outside xy domain: origin bond is non-attractive, so N <= 2")
        print("  arithmetic: every binary64 primitive widened one ULP outward")
        print()
        print("  s slab          cells     analytic N<=2    interval   uncertified"
              "   worst E lower")
        print("  " + "-" * 88)

    for s_num_lo in range(S_MIN_NUM, S_MAX_NUM):
        result = certify_s_slab(s_num_lo, s_num_lo + 1, cells)
        slab_results.append(result)
        total_cells += result["cells"]
        total_exact += result["by_analytic_N_le_2"]
        total_interval += result["by_interval"]
        total_uncertified += result["uncertified"]
        slab_worst = result["worst_energy_lower"]
        if slab_worst is not None:
            worst = slab_worst if worst is None else min(worst, slab_worst)

        if verbose:
            a, b = result["s_interval"]
            print(f"  [{a:>5},{b:<5}]  {result['cells']:9d}"
                  f"   {result['by_analytic_N_le_2']:13d}"
                  f"   {result['by_interval']:9d}"
                  f"   {result['uncertified']:11d}"
                  f"   {result['worst_energy_lower']:+.9f}"
                  if result['worst_energy_lower'] is not None
                  else f"  [{a:>5},{b:<5}]  {result['cells']:9d}"
                  f"   {result['by_analytic_N_le_2']:13d}"
                  f"   {result['by_interval']:9d}"
                  f"   {result['uncertified']:11d}            n/a")

    assert total_uncertified == 0, (
        f"{total_uncertified} boxes remain uncertified")
    if worst is not None:
        assert worst >= -2.0 * EPS, (
            f"outward lower bound {worst} is below -2 eps")
        margin_lo: float | None = float(add_lo(worst, 2.0 * EPS))
        assert margin_lo >= 0.0
    else:
        assert total_interval == 0
        assert total_exact == total_cells
        margin_lo = None

    script_path = Path(__file__).resolve()
    script_sha256 = hashlib.sha256(script_path.read_bytes()).hexdigest()
    certificate: dict[str, Any] = {
        "schema": "ftd.register_barrier.interval-certificate.v1",
        "claim": "min_{z=0} E = -2 eps for every s in [9/10,13/10]",
        "status": "proved_with_stated_arithmetic_model",
        "scope": "declared compact-support bond law and three-anchor geometry",
        "arithmetic": arithmetic,
        "analytic_bond_law": bond,
        "analytic_attainment": attainment,
        "coverage": {
            "xy_domain": ["-3/2", "3/2", "-3/2", "3/2"],
            "s_domain": ["9/10", "13/10"],
            "xy_step": "1/250",
            "s_step": "1/50",
            "xy_cells_per_slab": int(cells[0].size),
            "s_slabs": S_MAX_NUM - S_MIN_NUM,
            "total_boxes": total_cells,
            "outside_domain_argument": (
                "outside [-3/2,3/2]^2 the origin bond has q>=9/4>3/2, "
                "so at most two bonds are attractive"
            ),
            "sampled_points_used": 0,
        },
        "result": {
            "boxes_by_analytic_N_le_2": total_exact,
            "boxes_by_outward_interval": total_interval,
            "uncertified_boxes": total_uncertified,
            "worst_outward_energy_lower": worst,
            "worst_outward_energy_lower_hex": (
                None if worst is None else worst.hex()),
            "margin_above_minus_2_lower": margin_lo,
            "margin_above_minus_2_lower_hex": (
                None if margin_lo is None else margin_lo.hex()),
            "plane_minimum": "-2 eps",
            "ground_energy": "-3 eps",
            "barrier_lower_bound": "eps",
        },
        "slabs": slab_results,
        "provenance": {
            "script": str(script_path),
            "script_sha256": script_sha256,
            "python": platform.python_version(),
            "implementation": platform.python_implementation(),
            "platform": platform.platform(),
        },
    }

    if verbose:
        print()
        print(f"  certified boxes: {total_cells:,}")
        print(f"  uncertified boxes: {total_uncertified}")
        if worst is None:
            print("  three-attractive candidate boxes: 0")
            print("  interval-energy fallback: not needed")
        else:
            print(f"  worst outward energy lower bound: {worst:+.12f}")
            print(f"  certified margin above -2 eps: {margin_lo:.12f}")
        print("  attainment: analytic monotonic endpoint proof; no samples")
        print("  conclusion: plane minimum = -2 eps; barrier >= eps")
        print()

    return certificate


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--json",
        action="store_true",
        help="emit only the machine-readable JSON certificate",
    )
    args = parser.parse_args(argv)
    certificate = build_certificate(verbose=not args.json)
    print(json.dumps(certificate, sort_keys=True,
                     separators=(",", ":") if args.json else None))
    return 0


if __name__ == "__main__":
    sys.exit(main())
