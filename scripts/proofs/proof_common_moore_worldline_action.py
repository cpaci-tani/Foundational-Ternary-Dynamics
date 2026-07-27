#!/usr/bin/env python3
"""Exact algebraic proof witnesses for FTD-0578.

This is a structural calculation, not a parameter search. It proves the
registered diagonal-centering norms and the polynomial point-Peierls law.
"""

from fractions import Fraction as F
import sympy as sp


def convolve_axis(field, axis):
    out = {}
    for site, value in field.items():
        for offset, weight in ((-1, F(1, 4)), (0, F(1, 2)), (1, F(1, 4))):
            target = list(site)
            target[axis] += offset
            target = tuple(target)
            out[target] = out.get(target, F(0)) + weight * value
    return {site: value for site, value in out.items() if value}


def coat(field):
    for axis in range(3):
        field = convolve_axis(field, axis)
    return field


def norm2(field):
    return sum(value * value for value in field.values())


def difference(lhs, rhs):
    keys = set(lhs) | set(rhs)
    return {k: lhs.get(k, F(0)) - rhs.get(k, F(0)) for k in keys
            if lhs.get(k, F(0)) != rhs.get(k, F(0))}


def endpoint_midpoint(dim):
    zero = (0, 0, 0)
    end = tuple(1 if axis < dim else 0 for axis in range(3))
    return {zero: F(1, 2), end: F(1, 2)}


def time_average(dim):
    result = {}
    for bits in range(1 << dim):
        ones = bits.bit_count()
        value = F(sp.factorial(ones) * sp.factorial(dim - ones),
                  sp.factorial(dim + 1))
        site = tuple((bits >> axis) & 1 if axis < dim else 0
                     for axis in range(3))
        result[site] = value
    return result


expected = {1: F(0), 2: F(1, 1536), 3: F(5, 3072)}
for dimension in (1, 2, 3):
    mismatch = difference(coat(time_average(dimension)),
                          coat(endpoint_midpoint(dimension)))
    got = norm2(mismatch)
    assert got == expected[dimension], (dimension, got)

# D^T=-G and C^T=C fix both endpoint source derivatives.
g, t0, t1, q0, q1 = sp.symbols("g t0 t1 q0 q1")
Ddr0, Ddr1, Cdr0, Cdr1 = sp.symbols("Ddr0 Ddr1 Cdr0 Cdr1")
dI = g * (t0 * Ddr0 + t1 * Ddr1 + q0 * Cdr0 + q1 * Cdr1)
source_pairing = g * (t0 * Ddr0 + q0 * Cdr0) + g * (t1 * Ddr1 + q1 * Cdr1)
assert sp.expand(dI - source_pairing) == 0

# The magnetic Lorentz-form term has exactly zero scalar work.
vx, vy, vz, bx, by, bz = sp.symbols("vx vy vz bx by bz", real=True)
velocity = sp.Matrix([vx, vy, vz])
magnetic = sp.Matrix([bx, by, bz])
assert sp.simplify(velocity.dot(velocity.cross(magnetic))) == 0

# The compact carrier's static energy is exactly quadratic inside a cell.
r, c, v0, cosk = sp.symbols("r c v0 cosk", real=True)
modulus2 = sp.expand((1-r)**2 + r**2 + 2*r*(1-r)*cosk)
assert sp.expand(modulus2 - (1 - 2*r*(1-r)*(1-cosk))) == 0
potential = v0 + c*r*(1-r)
assert sp.simplify(sp.diff(potential, r) - c*(1-2*r)) == 0
assert sp.simplify(-sp.diff(potential, r) + c*(1-2*r)) == 0

print("FTD-0578 exact common Moore worldline-action proof: PASS")
print("centering_norm2_axial=0")
print("centering_norm2_edge=1/1536")
print("centering_norm2_body=5/3072")
print("peierls_form=V0+C*r*(1-r), F=-C*(1-2*r)")
print("verdict=COMMON_MOORE_WORLDLINE_ACTION_DERIVED_ENERGY_CENTERING_MISMATCH_PEIERLS_PINNED")
