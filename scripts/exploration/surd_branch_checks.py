"""Surd-branch checks for the FTD master quadratic  x^2 - 16 G*^2 x + 16 G*^3 = 0.

Every number is computed here (mpmath, 60 digits) or derived symbolically (sympy).
Sections:
  1. constants and Vieta identities
  2. algebra over Q(t), t = G*: irreducibility, the Galois swap, generic polynomial test
  3. transfer operators of finite deterministic maps (exact characteristic polynomial)
  4. forward and backward iteration of the Mobius map T(x) = 16G*^2 - 16G*^3/x
  5. the finite-horizon readout r_N = p_{N+1}/p_N and the exact identity D(r_N) = (x-/x+)^N
  6. which root forward dominance selects, for three readings of the operator
  7. numbers that some repository documents state differently
"""
import random
import mpmath as mp
import sympy as sp

mp.mp.dps = 60
S = lambda v, n=20: mp.nstr(v, n)

G = mp.gamma(mp.mpf(1) / 4) / mp.gamma(mp.mpf(3) / 4)
d = mp.sqrt(G * (4 * G - 1))
xp, xm = 8 * G**2 + 4 * G * d, 8 * G**2 - 4 * G * d
q = xm / xp

print("== 1. Constants (computed) ==")
print("G*        =", S(G, 25))
print("  G* - Gamma(1/4)^2/(sqrt(2) pi) =", S(G - mp.gamma(mp.mpf(1) / 4)**2 / (mp.sqrt(2) * mp.pi), 3))
print("delta     =", S(d, 25))
print("x+        =", S(xp, 25))
print("x-        =", S(xm, 25))
print("Vieta: x+ + x- - 16G*^2 =", S(xp + xm - 16 * G**2, 3), "| x+ x- - 16G*^3 =", S(xp * xm - 16 * G**3, 3),
      "| 1/x+ + 1/x- - 1/G* =", S(1 / xp + 1 / xm - 1 / G, 3))
print("q = x-/x+ =", S(q, 15), "| 1/q = x+/x- =", S(1 / q, 15))

print("\n== 2. Algebra over Q(t), t = G* ==")
t, x, D = sp.symbols('t x D')
P = x**2 - 16 * t**2 * x + 16 * t**3
print("discriminant in x          :", sp.factor(sp.discriminant(P, x)))
print("factor_list of P over Q    :", sp.factor_list(P))
print("square-free part t(4t-1)   :", sp.sqf_list(t * (4 * t - 1)))
print("P as a polynomial in t     :", sp.Poly(P, t).all_coeffs(), "(degree 3, leading coefficient 16)")
mod = D**2 - t * (4 * t - 1)
red = lambda e: sp.expand(sp.rem(sp.expand(e), mod, D))
rp, rm = 8 * t**2 + 4 * t * D, 8 * t**2 - 4 * t * D
print("P(x+), P(x-) mod D^2 - t(4t-1):", red(P.subs(x, rp)), ",", red(P.subs(x, rm)))
cs = sp.symbols('c0:5')
Qg = sum(c * x**k for k, c in enumerate(cs))
Ep, Em = sp.Poly(red(Qg.subs(x, rp)), D), sp.Poly(red(Qg.subs(x, rm)), D)
A, B = Ep.coeff_monomial(1), Ep.coeff_monomial(D)
print("generic quartic Q, write Q(x+) = A + B D; then Q(x-) - (A - B D) =", sp.simplify(Em.as_expr() - (A - B * D)))

print("\n== 3. Transfer operators of finite deterministic maps ==")
random.seed(20261006)
for n in (12, 24):
    phi = [random.randrange(n) for _ in range(n)]
    Am = sp.zeros(n, n)
    for i, j in enumerate(phi):
        Am[i, j] = 1
    cp = sp.expand(Am.charpoly(x).as_expr())
    on_cycle = set()
    for i in range(n):
        z = i
        for _ in range(n):
            z = phi[z]
        on_cycle.add(z)
    lens, seen = [], set()
    for z in on_cycle:
        if z in seen:
            continue
        c, w = [z], phi[z]
        while w != z:
            c.append(w)
            w = phi[w]
        seen.update(c)
        lens.append(len(c))
    pred = x**(n - sum(lens)) * sp.prod([x**L - 1 for L in lens])
    ok = True
    for f, e in sp.factor_list(cp)[1]:
        if sp.expand(f - x) == 0:
            continue
        ok = ok and any(sp.rem(x**m - 1, f, x) == 0 for m in range(1, n + 1))
    print(f"n = {n}: cycle lengths {sorted(lens)}, transient states {n - sum(lens)}; "
          f"charpoly - x^transient * prod(x^L - 1) = {sp.expand(cp - pred)}; "
          f"every irreducible factor is x or divides some x^m - 1: {ok}")

print("\n== 4. Iteration of T(x) = 16G*^2 - 16G*^3/x and of its inverse ==")
T = lambda z: 16 * G**2 - 16 * G**3 / z
Ti = lambda y: 16 * G**3 / (16 * G**2 - y)
print("T'(x+) = 16G*^3/x+^2 =", S(16 * G**3 / xp**2, 15), "; minus q:", S(16 * G**3 / xp**2 - q, 3))
print("T'(x-) = 16G*^3/x-^2 =", S(16 * G**3 / xm**2, 15), "; minus 1/q:", S(16 * G**3 / xm**2 - 1 / q, 3))
for z0 in (mp.mpf(1), mp.mpf(10), mp.mpf(70), mp.mpf(1000), mp.mpf(-5), xm * (1 + mp.mpf(10)**-40)):
    z = w = z0
    for _ in range(80):
        z = T(z)
    for _ in range(80):
        w = Ti(w)
    print(f"  start {S(z0, 10):>16}: forward 80 steps |z - x+| = {S(abs(z - xp), 3):>8} ; backward 80 steps |w - x-| = {S(abs(w - xm), 3)}")

print("\n== 5. Finite-horizon readout r_N = p_{N+1}/p_N, p_N = x+^N + x-^N ==")
p = [sp.Integer(2), 16 * t**2]
for N in range(2, 5):
    p.append(sp.expand(16 * t**2 * p[-1] - 16 * t**3 * p[-2]))
for N in range(5):
    print(f"  p_{N} = {sp.factor(p[N])}")
print("  p_-1 = p_1/(16 t^3) =", sp.simplify(p[1] / (16 * t**3)), " (this is 1/x+ + 1/x- = 1/G*)")
a, b = sp.symbols('a b', positive=True)
chk = []
for N in (-3, -2, -1, 1, 2, 3, 4, 5):
    r = (a**(N + 1) + b**(N + 1)) / (a**N + b**N)
    chk.append(sp.simplify((a - r) / (r - b) - (b / a)**N))
print("  symbolic check of (x+ - r_N)/(r_N - x-) = (x-/x+)^N for N in -3..5 (0 means proved for that N):", chk)
print("  N    r_N                      nearer   D(r_N)/q^N - 1")
for N in range(-4, 7):
    r = (xp**(N + 1) + xm**(N + 1)) / (xp**N + xm**N)
    Dr = (xp - r) / (r - xm)
    gap = abs(r - xp) - abs(r - xm)
    tag = 'midpoint' if abs(gap) < mp.mpf(10)**-40 else ('x+' if gap < 0 else 'x-')
    print(f"  {N:>2}   {S(r, 20):>24}   {tag:>8}   {S(Dr / q**N - 1, 3)}")
print("  r_-1 - 2G* =", S((xp**0 + xm**0) / (1 / xp + 1 / xm) - 2 * G, 3), " ; r_0 - 8G*^2 =", S((xp + xm) / 2 - 8 * G**2, 3))

print("\n== 6. Which root does dominance select? Three readings, two directions ==")
Ma = mp.matrix([[16 * G**2, -16 * G**3], [1, 0]])          # eigenvalues x+, x- : candidates for 1/alpha (multiplier)
Mb = mp.matrix([[1 / G, -1 / (16 * G**3)], [1, 0]])        # eigenvalues 1/x+, 1/x- : candidates for alpha (per-tick factor)
Ea, Ei = mp.expm(-Ma), mp.expm(Ma)                         # exp(-H), exp(+H) with spec(H) = {x+, x-} (decay rate)


def growth(M, steps=400):
    v = mp.matrix([1, mp.mpf('0.3')])
    g = 0
    for _ in range(steps):
        w = M * v
        g = mp.norm(w) / mp.norm(v)
        v = w / mp.norm(w)
    return g


rows = [
    ("(a) 1/alpha is a one-tick multiplier", growth(Ma), 1 / growth(mp.inverse(Ma))),
    ("(b) alpha is a one-tick factor", 1 / growth(Mb), growth(mp.inverse(Mb))),
    ("(c) 1/alpha is a decay rate, M = exp(-H)", -mp.log(growth(Ea)), mp.log(growth(Ei))),
    ("(d) alpha is a damping, J <- (1 - alpha) J", 1 / (1 - growth(mp.eye(2) - Mb)), 1 / (1 - 1 / growth(mp.inverse(mp.eye(2) - Mb)))),
    ("(e) alpha is a decay rate, M = exp(-alpha)", 1 / (-mp.log(growth(mp.expm(-Mb)))), 1 / mp.log(growth(mp.expm(Mb)))),
]
for name, fw, bw in rows:
    nm = lambda v: 'x+' if abs(v - xp) < 1e-30 else ('x-' if abs(v - xm) < 1e-30 else '??')
    print(f"  {name:<42} forward -> 1/alpha = {S(fw, 12):>14} ({nm(fw)})   backward -> {S(bw, 12):>14} ({nm(bw)})")

print("\n== 7. Numbers some repository documents state differently ==")
print("x+/x- =", S(xp / xm, 10), "; x+/x- - 1 =", S(xp / xm - 1, 10), "; 55.71/1.2572 =", S(mp.mpf('55.71') / mp.mpf('1.2572'), 6))
print("8G*^2(1 +/- sqrt(1 - 1/G*))     =", S(8 * G**2 * (1 + mp.sqrt(1 - 1 / G)), 10), ",", S(8 * G**2 * (1 - mp.sqrt(1 - 1 / G)), 10))
print("8G*^2(1 +/- sqrt(1 - 1/(4G*)))  =", S(8 * G**2 * (1 + mp.sqrt(1 - 1 / (4 * G))), 15), ",", S(8 * G**2 * (1 - mp.sqrt(1 - 1 / (4 * G))), 15))
print("harmonic mean of the roots =", S(2 / (1 / xp + 1 / xm), 12), "; 2G* =", S(2 * G, 12), "; G* =", S(G, 12))
print("x- - G* =", S(xm - G, 10), "(positive, so x- > G*)")
