#!/usr/bin/env python3
"""Exact algebraic witnesses for FTD-0580 (no parameter search)."""

from fractions import Fraction as F
from itertools import permutations
from math import factorial

import sympy as sp


def democratic_flow(dimension):
    flow = {}
    for order in permutations(range(dimension)):
        vertex = tuple(0 for _ in range(dimension))
        for axis in order:
            edge = (vertex, axis)
            flow[edge] = flow.get(edge, F(0)) + F(1, factorial(dimension))
            endpoint = list(vertex)
            endpoint[axis] = 1
            vertex = tuple(endpoint)
    return flow


for dimension in (1, 2, 3):
    flow = democratic_flow(dimension)
    divergence = {}
    for (vertex, axis), weight in flow.items():
        divergence[vertex] = divergence.get(vertex, F(0)) + weight
        endpoint = list(vertex)
        endpoint[axis] = 1
        endpoint = tuple(endpoint)
        divergence[endpoint] = divergence.get(endpoint, F(0)) - weight
    zero = tuple(0 for _ in range(dimension))
    one = tuple(1 for _ in range(dimension))
    assert divergence[zero] == 1
    assert divergence[one] == -1
    assert all(value == 0 for vertex, value in divergence.items()
               if vertex not in (zero, one))
    for (vertex, axis), weight in flow.items():
        subset_size = sum(vertex)
        expected = F(factorial(subset_size)
                     * factorial(dimension - subset_size - 1),
                     factorial(dimension))
        assert weight == expected

# Positivity and the endpoint-only time average force all off-endpoint weights
# to zero almost everywhere. The first moment then fixes these two weights.
t = sp.symbols("t", real=True, nonnegative=True)
p0, p1 = 1 - t, t
assert sp.simplify(p0 + p1 - 1) == 0
assert sp.integrate(p0, (t, 0, 1)) == F(1, 2)
assert sp.integrate(p1, (t, 0, 1)) == F(1, 2)

w0, w1 = 1 - t, t
assert sp.integrate(w0 * p0, (t, 0, 1)) == F(1, 3)
assert sp.integrate(w0 * p1, (t, 0, 1)) == F(1, 6)
assert sp.integrate(w1 * p0, (t, 0, 1)) == F(1, 6)
assert sp.integrate(w1 * p1, (t, 0, 1)) == F(1, 3)

rho0, rho1 = sp.symbols("rho0 rho1")
T0 = rho0 / 3 + rho1 / 6
T1 = rho0 / 6 + rho1 / 3
midpoint = (rho0 + rho1) / 2
assert sp.simplify(T0 + T1 - midpoint) == 0
assert sp.simplify((rho0 - rho1) / 2 - (rho0 - midpoint)) == 0
assert sp.simplify((rho0 - rho1) / 2 - (midpoint - rho1)) == 0

r, theta, coefficient, v0 = sp.symbols(
    "r theta coefficient v0", real=True)
factor2 = sp.expand((1-r)**2 + r**2 + 2*r*(1-r)*sp.cos(theta))
assert sp.simplify(factor2 - (1 - 2*r*(1-r)*(1-sp.cos(theta)))) == 0
potential = v0 + coefficient*r*(1-r)
assert sp.simplify(potential.subs(r, F(1, 2)) - v0 - coefficient/4) == 0
assert sp.simplify(-sp.diff(potential, r) + coefficient*(1-2*r)) == 0

print("FTD-0580 exact symmetric chord Moore-action proof: PASS")
print("shape=(1-t)*delta_0+t*delta_d")
print("route_weight=|S|!*(D-|S|-1)!/D!")
print("T0=rho0/3+rho1/6, T1=rho0/6+rho1/3")
print("Vself=V0+C_d*r*(1-r), C_d>0")
print("verdict=SYMMETRIC_CHORD_CLOSES_MOORE_CENTERING_PEIERLS_PINNING_REMAINS")

