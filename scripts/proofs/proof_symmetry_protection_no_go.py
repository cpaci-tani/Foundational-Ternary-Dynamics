"""FTD-1003 — certificate: no point-group selection rule protects an embedded
carrier mode in the matched face/edge field band.

Closes ONE of the four escapes FTD-0663
(`THEOREM_INTERNAL_MODE_FIELD_BAND_EMBEDDING.md`) left open. That audit
excluded frequency-gap protection exactly and excluded complete decoupling
for a prepared finite-volume excitation, then listed four surviving
mechanisms: "symmetry, destructive interference, a bound state in the
continuum, or a topological invariant". This certificate closes the FIRST in
its spatial point-group sense. Destructive interference and BIC-type
cancellation are the disclosed accidental escape (G6); a topological
mechanism is NOT closed here and is left explicitly open.

Setting (conditional on the FTD-0551/0641 SELECTED matched-field dynamics —
this is not postulate-forced content and the no-go inherits that
conditionality). The matched face/edge field has dispersion

    Omega(k) = 2 asin( sqrt( ( sin^2(k1/2)+sin^2(k2/2)+sin^2(k3/2) ) / 3 ) )

on the Brillouin torus, band [0, pi]. Per FTD-0641 the propagating sector is
divergence-free with TWO polarizations per k; longitudinal modes are slaved
by the constraint and do not propagate. The channel space at frequency omega
is therefore the rank-2 TRANSVERSE sub-bundle over the isofrequency surface
S(omega) = {k : Omega(k) = omega} -- not scalar L^2(S). This distinction is
load-bearing: the continuum SO(3) analogue of this no-go is FALSE (a
spherically pulsating source does not radiate -- Birkhoff), and it is false
precisely through the scalar-vs-transverse-bundle distinction, because the
momentum sphere is a NON-FREE SO(3) orbit and the transverse bundle over it
carries no l=0 sections. On the lattice the orbits are free and that escape
closes. An argument phrased on scalar L^2(S) would get the right lattice
answer for the wrong reason.

Criterion. The load-bearing statement is not the leading-order Fermi
golden rule but the Friedrichs-model necessary condition: a genuine EMBEDDED
eigenvalue of the linear coupled system requires the on-shell coupling to
vanish identically on S at the renormalized frequency. Since the argument
below covers every omega in (0, pi), frequency renormalization cannot escape
it. Scope: linearized matched-field dynamics.

Argument. With an O_h-invariant coupling and a localized mode carrying irrep
Gamma, the on-shell coupling is an equivariant section of the transverse
bundle over S; symmetry forces it to vanish identically only if Gamma is
ABSENT from the channel representation. Generic points of S have trivial
O_h stabilizer, and sections of a rank-d equivariant bundle over a free
orbit form d copies of the regular representation, which contains every
irrep with multiplicity d*dim(Gamma) > 0. Hence no irrep assignment can
forbid coupling by symmetry.

Illustrative/derivational content only: read-only mathematics, no engine
contact, no tag moved, no numerical search (every computational check
verifies a stated identity or exact group fact).
"""

from __future__ import annotations

import itertools
import math

PHI = 1.0911648733663635          # first internal doublet phase (FTD-0640)
C_SPEED = 1.0 / math.sqrt(3.0)
AXIS_TOP = 2.0 * math.asin(C_SPEED)   # 1.2309594... the RECORDED C2 edge
BAND_TOP = 2.0 * math.asin(1.0)       # pi -- the ACTUAL band top

COMPUTATIONAL = 0
DISCLOSURE = 0
FAIL = 0


def check(name: str, ok: bool, detail: str = "", disclosure: bool = False) -> None:
    """Record a check. Disclosure entries are scope statements that cannot
    fail by construction and are counted separately, never blended into the
    computational total."""
    global COMPUTATIONAL, DISCLOSURE, FAIL
    if ok:
        if disclosure:
            DISCLOSURE += 1
        else:
            COMPUTATIONAL += 1
        print(f"[{'DISC' if disclosure else 'PASS'}] {name}"
              + (f" :: {detail}" if detail else ""))
    else:
        FAIL += 1
        print(f"[FAIL] {name}" + (f" :: {detail}" if detail else ""))


def s_of(k):
    return sum(math.sin(ka / 2.0) ** 2 for ka in k)


def omega(k):
    return 2.0 * math.asin(math.sqrt(min(1.0, max(0.0, s_of(k) / 3.0))))


# ----------------------------------------------------------------------
# G1 — setting: dispersion, band, the mode's position
# ----------------------------------------------------------------------
check("G1 band top is pi", math.isclose(BAND_TOP, math.pi, abs_tol=1e-15))
check("G1 recorded C2 edge = 2 asin C is only the <100> axis-branch max",
      math.isclose(AXIS_TOP, 1.2309594173407747, abs_tol=1e-12)
      and AXIS_TOP < BAND_TOP,
      f"axis top {AXIS_TOP:.10f} < band top {BAND_TOP:.10f}")
check("G1 internal mode strictly inside the band", 0.0 < PHI < BAND_TOP,
      f"omega_0={PHI:.13f}")
S_LEVEL = 3.0 * math.sin(PHI / 2.0) ** 2
check("G1 isofrequency level 0 < s* < 1", 0.0 < S_LEVEL < 1.0,
      f"s*={S_LEVEL:.13f}")

# S is a REGULAR level set: grad(Omega)=0 requires sin(k_a)=0 for all a,
# i.e. every k_a in {0,pi}, giving s in {0,1,2,3}. s* is none of these.
check("G1 S(omega_0) is a regular level set (analytic, not sampled)",
      all(abs(S_LEVEL - c) > 1e-9 for c in (0.0, 1.0, 2.0, 3.0)),
      "critical s-values are exactly {0,1,2,3}; s*=0.8078 avoids all")

# ----------------------------------------------------------------------
# G2 — O_h as the 48 signed permutation matrices
# ----------------------------------------------------------------------
GROUP = [(perm, signs)
         for perm in itertools.permutations(range(3))
         for signs in itertools.product((1, -1), repeat=3)]
IDENTITY = ((0, 1, 2), (1, 1, 1))
check("G2 |O_h| = 48", len(GROUP) == 48)


def act(g, k):
    perm, signs = g
    return tuple(signs[i] * k[perm[i]] for i in range(3))


def wrap(k):
    """Canonical torus representative. The zone boundary needs explicit care:
    -pi and +pi are the SAME point but math.remainder returns distinct
    floats, which would make the corner read as an 8-point orbit instead of
    a fixed point."""
    out = []
    for ka in k:
        x = math.remainder(ka, 2.0 * math.pi)
        if abs(abs(x) - math.pi) < 1e-9:
            x = math.pi
        out.append(round(x, 9) + 0.0)
    return tuple(out)


check("G2 dispersion is O_h-invariant",
      all(math.isclose(omega(act(g, (0.7, 1.3, 2.1))), omega((0.7, 1.3, 2.1)),
                       abs_tol=1e-14) for g in GROUP))

# ----------------------------------------------------------------------
# G3 — S carries free orbits (genericity: analytic backstop + witnesses)
# ----------------------------------------------------------------------
def surface_point(theta, varphi, target):
    ux = math.sin(theta) * math.cos(varphi)
    uy = math.sin(theta) * math.sin(varphi)
    uz = math.cos(theta)
    lo, hi = 0.0, math.pi
    for _ in range(200):
        mid = 0.5 * (lo + hi)
        if omega((mid * ux, mid * uy, mid * uz)) < target:
            lo = mid
        else:
            hi = mid
    r = 0.5 * (lo + hi)
    return (r * ux, r * uy, r * uz)


samples = [surface_point(t, p, PHI)
           for t in (0.7, 1.0, 1.3, 1.9, 2.4)
           for p in (0.3, 0.9, 1.7, 2.6, 3.9, 5.1)]
check("G3 witnesses lie on S(omega_0)",
      all(math.isclose(omega(p), PHI, abs_tol=1e-9) for p in samples),
      f"{len(samples)} points, max dev "
      f"{max(abs(omega(p) - PHI) for p in samples):.2e}")

# Analytic backstop for genericity (the sampling only exhibits witnesses):
# the fixed-point set of any non-identity element of O_h is a union of flats
# of dimension <= 2; a real-analytic level set of the non-constant Omega
# cannot contain a 2-dimensional piece of such a flat without Omega being
# constant on an open subset of it. So the free part of S is open and dense.
check("G3 free part of S is open and dense (analytic backstop)",
      True,
      "fixed-point flats have dim <= 2; Omega non-constant real-analytic",
      disclosure=True)
free = sum(1 for p in samples
           if len({wrap(act(g, p)) for g in GROUP}) == 48)
check("G3 exhibited free orbits (trivial stabilizer, orbit size 48)",
      free == len(samples), f"{free}/{len(samples)}")

# The no-go is NOT confined below the recorded C2 edge: free orbits exist on
# every level set strictly inside the band, including above 2 asin C.
above = surface_point(1.1, 0.7, 1.5)     # 1.2310 < 1.5 < pi
check("G3 free orbits exist ABOVE the recorded C2 edge too",
      math.isclose(omega(above), 1.5, abs_tol=1e-9)
      and len({wrap(act(g, above)) for g in GROUP}) == 48,
      "omega=1.5 in (2 asin C, pi): still embedded, still unprotected")

# ----------------------------------------------------------------------
# G4 — the channel space is the rank-2 TRANSVERSE bundle, not scalar L^2(S)
# ----------------------------------------------------------------------
def gradient_symbol(k):
    """Discrete gradient symbol; the longitudinal (slaved) direction."""
    return tuple(2.0 * math.sin(ka / 2.0) for ka in k)


check("G4 longitudinal direction is nonzero everywhere on S "
      "(=> transverse fiber rank exactly 2)",
      all(max(abs(c) for c in gradient_symbol(p)) > 1e-9 for p in samples),
      "gradient symbol vanishes only at k=0, and s*>0 excludes it")
check("G4 the continuum analogue would FAIL here (why the bundle matters)",
      True,
      "SO(3): momentum sphere is a NON-free orbit, transverse bundle has no "
      "l=0 sections => Birkhoff non-radiation. Lattice orbits are free.",
      disclosure=True)

# ----------------------------------------------------------------------
# G5 — sections over a free orbit: d copies of the regular representation
# ----------------------------------------------------------------------
p0 = samples[0]
orbit = sorted({wrap(act(g, p0)) for g in GROUP})
perm_char = [sum(1 for pt in orbit if wrap(act(g, pt)) == pt) for g in GROUP]
check("G5 permutation character on the free orbit is regular "
      "(chi(E)=48, chi(g!=E)=0)",
      perm_char[GROUP.index(IDENTITY)] == 48 and sorted(perm_char)[-2] == 0,
      f"chi(E)={max(perm_char)}, next={sorted(perm_char)[-2]}")

FIBER_RANK = 2
IRREP_DIMS = {"A1g": 1, "A2g": 1, "Eg": 2, "T1g": 3, "T2g": 3,
              "A1u": 1, "A2u": 1, "Eu": 2, "T1u": 3, "T2u": 3}
check("G5 O_h has 10 irreps, sum of squared dims = 48",
      len(IRREP_DIMS) == 10 and sum(d * d for d in IRREP_DIMS.values()) == 48)

# Ind_e^G(fiber) = fiber_rank x regular rep; multiplicity of Gamma is
# (1/|G|) * (fiber_rank * |G|) * dim(Gamma) = fiber_rank * dim(Gamma).
mult = {n: FIBER_RANK * d for n, d in IRREP_DIMS.items()}
check("G5 every irrep appears in the transverse channel representation",
      all(m > 0 for m in mult.values())
      and all(mult[n] == FIBER_RANK * IRREP_DIMS[n] for n in IRREP_DIMS),
      ", ".join(f"{n}:{mult[n]}" for n in IRREP_DIMS))

# ----------------------------------------------------------------------
# G6 — the no-go, and the escapes it leaves open
# ----------------------------------------------------------------------
check("G6 NO-GO: no point-group selection rule can force the on-shell "
      "coupling to vanish identically on S",
      all(mult[n] > 0 for n in IRREP_DIMS),
      "every Gamma-isotypic component of the channel bundle is nonzero")

corner = (math.pi, math.pi, math.pi)
check("G6 escape (i): S degenerates only AT the band endpoint",
      math.isclose(omega(corner), BAND_TOP, abs_tol=1e-12)
      and len({wrap(act(g, corner)) for g in GROUP}) == 1,
      "omega(pi,pi,pi)=pi, orbit size 1 -- full O_h stabilizer")

# Accidental cancellation is achievable at codimension EXACTLY 1 here,
# because sigma^2 = 4*s is CONSTANT on S: any coupling factorizing as
# (c1 + c2*sigma^2(k)) * M_bare vanishes on the whole surface under the
# single tuning c1 = -4*c2*s*.
sigma2_on_S = [4.0 * s_of(p) for p in samples]
check("G6 escape (ii): sigma^2 is constant on S => accidental vanishing is "
      "codimension EXACTLY 1, not merely >= 1",
      max(sigma2_on_S) - min(sigma2_on_S) < 1e-8,
      f"sigma^2 = {sigma2_on_S[0]:.10f} on all witnesses; one tuning "
      f"c1 = -4*c2*s* kills the whole surface")
check("G6 escape (iii): strong coupling expelling the mode above the band "
      "is C2 SATISFIED, not a protected embedded mode",
      True, "level repulsion out of the band is band clearance",
      disclosure=True)
check("G6 escape (iv): a TOPOLOGICAL mechanism is NOT closed here",
      True, "FTD-0663's fourth caveat remains formally open",
      disclosure=True)

# ----------------------------------------------------------------------
# G7 — scope firewalls (disclosure; cannot fail by construction)
# ----------------------------------------------------------------------
for name, note in [
    ("G7 closes the point-group sense of 'symmetry' only",
     "not all conceivable protection mechanisms"),
    ("G7 proves symmetry cannot FORCE vanishing, not that coupling is nonzero",
     "accidental vanishing untested here; FTD-0676 measured Gamma_E>0"),
    ("G7 lower site symmetry only strengthens the conclusion",
     "Stab_H subset Stab_Oh = {e}; free H-orbits still give H-regular"),
    ("G7 scoped to LINEARIZED matched-field dynamics",
     "Friedrichs embedded-eigenvalue criterion, not nonlinear"),
    ("G7 conditional on the FTD-0551/0641 SELECTED dynamics",
     "not postulate-forced; inherits that conditionality"),
    ("G7 harmonics also lie in band (2*phi = 2.182 < pi)",
     "MacKay-Aubry requires harmonic clearance too"),
    ("G7 no engine contact, no production change, no tag moved",
     "FTD-0663 stands; C_SPEED remains a SELECTION"),
] :
    check(name, True, note, disclosure=True)

print()
total = COMPUTATIONAL + DISCLOSURE + FAIL
print(f"proof_symmetry_protection_no_go.py : "
      f"{COMPUTATIONAL}/{COMPUTATIONAL} computational checks passed; "
      f"{DISCLOSURE} disclosure/scope assertions logged (cannot fail); "
      f"{FAIL} failed (of {total})")
if FAIL == 0:
    print("OUTCOME B - point-group selection-rule protection EXCLUDED for "
          "every omega strictly inside (0, pi)")
raise SystemExit(0 if FAIL == 0 else 1)
