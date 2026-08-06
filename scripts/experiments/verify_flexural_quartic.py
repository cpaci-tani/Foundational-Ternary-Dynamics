"""C3 candidate: the transverse mode of a collinear trimer against the
registered compact law. Is the quadratic term zero BY GEOMETRY?"""
import sympy as sp

eps, y, u, r, q = sp.symbols("epsilon y u r q", positive=True)

# registered compact law, DERIV_MINIMAL_MANY_BODY_MATTER_NETWORK_v1 eq.3
V = -16*eps*(q - sp.Rational(3,2))**2 * (q - sp.Rational(3,4))

print("=== 1. THE BOND IS UNTENSIONED AT r0 = 1 ===")
Vr = V.subs(q, r**2)
print("  V(r0=1)   =", sp.simplify(Vr.subs(r,1)), "  (depth -eps)")
print("  V'(r0=1)  =", sp.simplify(sp.diff(Vr,r).subs(r,1)), " <- zero tension")
print("  V''(r0=1) =", sp.simplify(sp.diff(Vr,r,2).subs(r,1)), " (= 96 eps = k_bond)")
print("  support: V = 0 for q >= 3/2, so A-C at distance 2 (q=4) do NOT interact.")

print("\n=== 2. COLLINEAR TRIMER A--B--C, B DISPLACED TRANSVERSELY BY y ===")
# both bonds have length sqrt(r0^2 + y^2), so q = 1 + y^2 exactly
Vtot = sp.expand(2*V.subs(q, 1 + u))            # u = y^2, EXACT (no expansion)
print("  V_tot(u) = 2*V(q=1+u) =", sp.factor(Vtot))
Vtot_poly = sp.Poly(sp.expand(Vtot), u)
print("  expanded in u :", sp.expand(Vtot))
Vy = sp.expand(Vtot.subs(u, y**2))
print("  in y          :", Vy)

print("\n=== 3. THE DECISIVE CHECK: IS THE QUADRATIC TERM ZERO? ===")
ser = sp.series(Vy - Vy.subs(y,0), y, 0, 9).removeO()
print("  V(y) - V(0) =", sp.expand(ser))
c2 = sp.diff(Vy, y, 2).subs(y, 0)/2
c4 = sp.diff(Vy, y, 4).subs(y, 0)/24
c6 = sp.diff(Vy, y, 6).subs(y, 0)/720
print(f"  quadratic coefficient  = {sp.simplify(c2)}   <-- MUST BE ZERO")
print(f"  quartic   coefficient  = {sp.simplify(c4)}")
print(f"  sextic    coefficient  = {sp.simplify(c6)}")
assert sp.simplify(c2) == 0
print("\n  *** V''(0) = 0 EXACTLY, with NO tuning. C3's null-flatness is met. ***")
print("  The vanishing is forced by two registered facts only:")
print("    (i)  r0 = 1 is the bond MINIMUM  => zero tension, V'(r0) = 0")
print("    (ii) transverse geometry         => delta_l = O(y^2)")
print("  so V ~ (k/2)(delta_l)^2 = O(y^4). Nothing is selected to make it vanish.")

print("\n=== 4. THE WELL IN y ===")
# turning structure: dV/du = 0
dV = sp.simplify(sp.diff(Vtot, u))
print("  dV/du =", sp.factor(dV), " -> stationary at u = 0 and u = 1/2")
u_sep = sp.Rational(1,2)
print(f"  separatrix at u = 1/2  => y = 1/sqrt(2) = {float(sp.sqrt(u_sep)):.6f}")
print(f"  bond length there      = sqrt(1+1/2) = sqrt(3/2) = dissociation edge (exact)")
barrier = sp.simplify(Vtot.subs(u,u_sep) - Vtot.subs(u,0))
print(f"  barrier height         = {barrier}  (= 2 eps)")
print("\n  So: a QUARTIC-BOTTOMED well of depth 2 eps and half-width 1/sqrt(2),")
print("  whose separatrix is exactly the bond dissociation point.")
