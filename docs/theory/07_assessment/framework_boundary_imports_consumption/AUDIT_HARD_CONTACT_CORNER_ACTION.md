# AUDIT — Hard-contact corner action

**Date:** 2026-07-25  
**Identifier:** `FTD-0516`  
**Status:** `[SELECTION — HARD-CONTACT MANIFOLD]` +
`[THEOREM — RELATIVISTIC CORNER CONDITIONS]` +
`[DERIVED — RESTRICTED HOUSEHOLDER IMPULSE]` +
`[CONSTRUCTIVE — EXACT FACE-BALANCE COMPOSITION]` +
`[CORRECTED BY FTD-0526 — IDENTICAL CONTACT IS PERMUTATION GAUGE]` +
`[OPEN — DISTINGUISHABLE CONTACT/FIELD ORIGIN]`  
**Verdict:**
`SELECTED_HARD_CONTACT_ACTION_DERIVES_RESTRICTED_IMPULSE_NO_FIELD_ORIGIN`  
**Pre-registration:**
[`PREREG_HARD_CONTACT_CORNER_ACTION_v1.md`](../../10_eft_program/preregistrations/framework_boundary_imports_consumption/PREREG_HARD_CONTACT_CORNER_ACTION_v1.md)  
**Run of record:** `engine/results/ftd_0516/windows_msvc_cpu.json`

## 1. The selected matter action

The existing dispersion

```text
H(p)=sqrt(m^2+c^2|p|^2)
```

has the free Legendre pair

```text
v=c^2 p/H,
L(v)=-m sqrt(1-|v|^2/c^2),
p=dL/dv.
```

For two carriers at an already detected boundary contact, select the unilateral
gap

```text
phi(x1,x2)=(x2-x1) dot n >= 0,
```

with `n` the existing chart normal from carrier 1 toward carrier 2. The
piecewise free action is varied with its corner constrained to `phi=0`.

This contact manifold is a model choice. It is not produced by face E/B,
polarity, Gauss law, or the aggregate current. The result below is conditional
on that selected matter geometry.

## 2. Exact corner derivation

Variation of the common collision position uses translation invariance and
gives equal-and-opposite impulses. Relative tangential variation is allowed
inside the contact surface and removes tangential impulse. Variation of the
collision time gives energy continuity. Equivalently, the corner/KKT equations
are

```text
p1+ - p1- = -lambda n,
p2+ - p2- = +lambda n,
H1+ + H2+ = H1- + H2-,
lambda >= 0,
lambda phi = 0.
```

The multiplier is an event solution, not retained state.

In the registered equal-mass, axial-relative, zero-COM class, let

```text
q=(p1- - p2-)/2,
q_n=q dot n > 0.
```

Substitution into energy continuity factorizes exactly:

```text
lambda(lambda-2q_n)=0.
```

The `lambda=0` branch leaves the relative gap rate incoming. The unilateral
outgoing condition selects the unique nontrivial branch

```text
lambda=2q_n,
q+=q-2(q dot n)n.
```

This is the FTD-0512 Householder reflection. FTD-0512's central impulse,
elasticity, and nonpenetration premises are therefore not independent within
this restricted class: they are the corner conditions and admissibility rule
of one selected hard-contact action.

## 3. Exact campaign

The frozen campaign used both polarities, all 26 nonzero Moore directions,
three translations, two speeds, and 144 explicit signed-cubic transforms.

```text
collision arms                         312
signed-cubic covariance arms           144
worst FTD-0512 match residual           0
worst corner-condition residual         2.2204460492503131e-16
worst KKT/branch residual               0
worst Legendre residual                 1.1102230246251565e-16
worst FTD-0514 face-balance residual    2.2204460492503131e-16
worst reversal residual                 6.6613381477509392e-16
worst translation residual              0
worst polarity residual                 0
worst cubic covariance residual         0
minimum multiplier                      0.39256107387425904
maximum incoming gap rate              -0.24999999999999994
minimum outgoing gap rate               0.24999999999999983
```

A positive-gap control returns exactly zero multiplier and impulse. A
penetrating initial state fails closed. Time reversal preserves the nonnegative
event multiplier and returns the reversed outgoing corner to the reversed
incoming corner.

## 4. What changed in the research map

The force-origin hierarchy is now:

1. FTD-0512: existing constituent phase space supports the restricted
   reflection, while aggregate face current cannot derive it;
2. FTD-0513/0514: rank-2 kinetic stress observes the counterflow and exact face
   transport supplies its local momentum balance;
3. FTD-0516: one selected hard-contact matter action generates the restricted
   impulse and composes with that exact balance.

This is a real compression of assumptions, but not native emergence. The hard
contact inequality remains selected. It neither follows from ternary occupancy
nor supplies a finite-range force before contact. It also does not couple the
matter action to face E/B, handle unequal masses, choose general 3D scattering,
or create a production collision transaction.

FTD-0525 proves that the surface is not an explicit raw production active set.
FTD-0526 then prevents overreading that result: in this identical-carrier
class, pass-through and the FTD-0516 momentum exchange are one permutation
quotient through phase space and exact face current. The corner impulse chooses
a labeled/chart representative but adds no aggregate physical content. It may
become physical for distinguishable carriers, unequal masses, or nontrivial
scattering; those sectors remain underived. Reciprocal electromagnetic motion
still requires a field action whose variation supplies force and recoil.

No production code, default, toggle, scenario, force, collision rule, field,
normalization, or tolerance changed.

- checks: `6/6 PASS`;
- test SHA256:
  `CC84694981AE8ABBD7A5B241300F6C26C165876CEBCD836D926A257F247B97EE`;
- header SHA256:
  `E7DAFE773DEA09F8490265710CE10125E8E6B0A49607570CB68BA6510754BECC`;
- implementation SHA256:
  `7F4CC1C1FF88727AED35B59157BAEAA527BE5C4260688219EAA9BA3087E14A7C`;
- locked preregistration SHA256:
  `9A25729DA28971BCA6E6A7A87C2EA8236E96ED93EF0AEFFAF5B49F5E86E28725`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
