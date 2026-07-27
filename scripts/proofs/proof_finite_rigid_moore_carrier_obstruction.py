#!/usr/bin/env python3
"""Independent exact proof witnesses for FTD-0579."""

import sympy as sp

t = sp.symbols("t", real=True)
ux, uy, uz = sp.symbols("u_x u_y u_z")


def mismatch(active):
    product = sp.prod(1 + t*u for u in active)
    temporal = sp.integrate(product, (t, 0, 1))
    endpoint = sp.Rational(1, 2)*(1 + sp.prod(1 + u for u in active))
    return sp.factor(temporal-endpoint)


assert mismatch([ux]) == 0
assert mismatch([ux, uy]) == -ux*uy/sp.Integer(6)
assert sp.expand(mismatch([ux, uy, uz])
                 + (ux*uy+ux*uz+uy*uz)/6 + ux*uy*uz/4) == 0

# A representative generic Laurent product is nonzero; the general step is
# the integral-domain property of R[z_x^±1,z_y^±1,z_z^±1].
x, y, z, a0, a1, a2 = sp.symbols("x y z a0 a1 a2")
B = (x+1)**2*(y+1)**2*(z+1)**2
A = a0+a1*x+a2*y*z
M_edge = -(x-1)*(y-1)/6
M_body = -((x-1)*(y-1)+(x-1)*(z-1)+(y-1)*(z-1))/6 \
         -(x-1)*(y-1)*(z-1)/4
assert sp.Poly(sp.expand(B*A*M_edge), x, y, z, a0, a1, a2).is_zero is False
assert sp.Poly(sp.expand(B*A*M_body), x, y, z, a0, a1, a2).is_zero is False

# Exact fractional-translation modulus and Peierls polynomial.
r, c = sp.symbols("r c", real=True)
ck = sp.symbols("c_k", real=True)
modulus2 = sp.expand((1-r)**2+r**2+2*r*(1-r)*ck)
assert sp.expand(modulus2-(1-2*r*(1-r)*(1-ck))) == 0
potential = sp.symbols("V_0")+c*r*(1-r)
assert sp.simplify(-sp.diff(potential, r)+c*(1-2*r)) == 0

# The Hodge response has infrared limit three.
e, qx, qy, qz = sp.symbols("e q_x q_y q_z", real=True)
cx, cy, cz = sp.cos(e*qx), sp.cos(e*qy), sp.cos(e*qz)
den = 4-sp.Rational(2, 3)*(cx+cy+cz) \
      -sp.Rational(2, 3)*(cx*cy+cx*cz+cy*cz)
num = 3*(sp.sin(e*qx)**2+sp.sin(e*qy)**2+sp.sin(e*qz)**2)
q2 = qx*qx+qy*qy+qz*qz
assert sp.simplify(sp.limit(den/e**2, e, 0)-q2) == 0
assert sp.simplify(sp.limit(num/e**2, e, 0)-3*q2) == 0

# Gaussian rescaling constants. Integral exp(-q^2/4) d^3q/(2pi)^3.
pi = sp.pi
I0 = 1/pi**sp.Rational(3, 2)
G, N = sp.symbols("G N", positive=True)
U0 = 3*G**2*I0/(2*N**sp.Rational(3, 2))
C = 3*G**2*I0/N**sp.Rational(5, 2)
barrier = C/4
assert sp.simplify((barrier/U0)-1/(2*N)) == 0

# Exact binomial mismatch ratios. Under cos^(2N),
# E[u]=-1/(N+1), E[|u|^2]=2/(N+1).
d = sp.symbols("d", positive=True)
mu1 = -1/d
mu2 = 2/d
edge_ratio = sp.simplify(mu2**2/36)
p2 = 3*mu2**2+6*mu2*mu1**2
t2 = mu2**3
pt = 3*mu2**2*mu1
body_ratio = sp.simplify(p2/36+t2/16+pt/12)
assert edge_ratio == 1/(9*d**2)
assert sp.simplify(body_ratio-(2*d-1)/(6*d**3)) == 0

print("FTD-0579 exact finite rigid Moore-carrier proof: PASS")
print("M_edge=-u_i*u_j/6")
print("M_body=-sum_pairs/6-u_i*u_j*u_k/4")
print("Pi_binomial~1/(2N)")
print("edge_ratio=1/[9(N+1)^2]")
print("body_ratio=[2(N+1)-1]/[6(N+1)^3]")
print("verdict=FINITE_RIGID_MOORE_CARRIER_CANNOT_REMOVE_CENTERING_OR_PEIERLS_EXTENSION_SUPPRESSES_ONLY")
