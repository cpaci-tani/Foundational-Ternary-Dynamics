# FTD-0797 — The de Rham Common-Cone Construction, Refuted; and a Separable No-Go

**Status:** `[REFUTATION — KILLED PRE-REGISTRATION]` +
`[EXACT — NOT ORIGINAL: Delta_p = M(k) I ON THE CUBIC DEC COMPLEX]` +
`[THEOREM — NEW, NEGATIVE: DEGREE-BLINDNESS EXCLUDES O(k^4) ISOTROPY]` +
`[ENGINE FACT — THE PRODUCTION STENCIL IS M18, NOT THE de RHAM LAPLACIAN]`
**Verdict:** `SPATIAL_DEGREE_BLINDNESS_DOES_NOT_TOUCH_THE_CONE`
**Parents:** `FTD-0411`, `FTD-0568`, `FTD-0412`, `FTD-0796`, PL-5, sidebranch §§22–24
**Production impact:** none — nothing was registered on the refuted claim

## 1. What was proposed, and why it fails

Proposed (in-session, by me): that the exact common cone demanded by §24 follows
from putting every sector on the cubic-lattice de Rham complex, because the
Hodge Laplacian is degree-blind there. Sent to a hostile referee before
registration and **refuted on three of three parts**.

**The one-line refutation.** Flat continuum `R^3` in Cartesian coordinates
*already* has `Delta_p = -grad^2 . I` on every form degree, identically and
trivially — and the inter-sector cone problem exists there anyway (phonons,
magnons and photons in one medium do not share a cone). **A property the
continuum already possesses cannot be what forces a common cone on a lattice.**

## 2. Part A survives — exactly true, not original, and inert

`Delta_p = M(k) . I` for `p = 0,1,2,3` on the cubic DEC complex, with
`M(k) = sum_i 2(1-cos k_i)`. Verified two independent ways: explicit integer
incidence matrices on a periodic lattice (`d.d = 0` exactly; deviation
`1.47e-14`), and exact symbolic residual zero. The convention-free certificate
is `spec(Delta_1) = spec(Delta_2) = 3 x spec(Delta_0)`.

**Not original.** The mechanism is `C^*(Z^3) = C^*(Z)^{tensor 3}`: the 1-D Hodge
Laplacian is already degree-blind, and Künneth gives 3-D degree-blindness for
free. This is the standard basis of the Kähler–Dirac / staggered construction.
Also verified: `(d + d*)^2 = Delta` with residual exactly `0.0`.

**Correction of an earlier error, confirmed.** A first version of the check
used a spurious conjugate in `d2`; the decisive diagnostic is that with it
`d2 . d1 != 0`, so the object is not a cochain complex at all. The correction
was right and the original was wrong.

## 3. Part B refuted — the construction never touches the cone

Write sector `s` as `S_s = int [ Z_s^t (d_t phi_s)^2 - Z_s^x phi_s* Delta phi_s ]`,
so the cone speed is

```text
C_s = Z_s^x / Z_s^t
```

Part A fixes the **spatial operator** `Delta` to be degree-blind. It constrains
neither `Z_s^t` nor `Z_s^x`. §24's exactly-marginal direction is precisely
`delta(Z_s^x / Z_s^t)` — **the one direction Part A does not touch.** Part A is
a purely spatial three-dimensional statement; *there is no time direction in the
complex at all*. §24's mechanism demands a single emergent `g_mu_nu` including
`g_00`, and `C_s` is the ratio `g^{ii}/g^{00}`. The proposal implements the half
that is irrelevant and omits the half that matters.

**No adequate protecting symmetry.** `d` and `d*` are not symmetries (not
invertible, `d^2 = 0`); the intertwiner `d Delta_p = Delta_{p+1} d` becomes a
dynamical statement only after the `Z`s are already equal — circular. The Hodge
star is a genuine `Z/2` isometry but protects only `C_0 = C_3` and `C_1 = C_2`,
**not** `C_0 = C_1` — two of the three needed equalities. The Kähler–Dirac taste
symmetry does mix degrees and is the only candidate in the vicinity, and **it is
the known counterexample**: taste symmetry is degenerate at tree level by the
complex's structure and is broken by interactions at `O(a^2)`, the dominant
systematic in staggered lattice QCD, requiring explicit improvement to suppress.
That is a worked, measured instance of exactly this proposal failing at loop
level.

**"Put everything on one complex" is a tuning, not a symmetry.**

## 4. Part C refuted on fact — the production operator is not the de Rham Laplacian

`engine/src/dag_engine.cpp:145-171` implements the **18-point SC+FCC Moore
stencil**: six face neighbours at `1/3`, twelve edge neighbours at `1/6`, centre
`-4`. Verified symbolically:

| symbol | `O(k^2)` | `O(k^4)` |
|---|---|---|
| `M18` (production) | `S2` | `-S2^2/12` — **isotropic** |
| `M6` (de Rham) | `S2` | `-Q4/12` — **cubic-anisotropic** |

with `M18 - M6 = -(1/6) sum_{i<j} k_i^2 k_j^2` at `O(k^4)`, and the ratio
`M18/M6` **not constant** — `1.000000` at `(0.3,0,0)`, `0.992556` at
`(0.3,0.3,0)`, `0.833333` at `(pi/2,pi/2,0)`.

All three registered sectors fail to be `C_s . M(k)`: production is `M18`; the
BCC-time flux pole is quartic-**free** while `C.M6` has quartic `-C.Q4/12 != 0`;
and Wilson matter — which *is* built from the complex, its Wilson term being
exactly `M6/2` — still fails, because its pole contains `M6` **squared**. That
last case is the sharpest: **being built from the complex is demonstrably not
sufficient.**

## 5. The prescription would destroy two registered results

- **PL-5.** The registered UV anisotropy exponent is `p = 4.0008 +/- 0.0006`.
  Fitting the phase-speed spread at 50 digits: `M18` gives `p = 4.0011`
  (reproducing PL-5); `M6` gives `p = 2.0005`. `M18`'s *isotropic* quartic is
  precisely why anisotropy is pushed to `k^4`. Rebuilding the flux sector on the
  complex falsifies PL-5 by roughly 3000 sigma.
- **`c^2 = 1/7`.** Solving FTD-0411's pole equation on `M18` leaves one condition
  and yields `c^2 = 1/7` exactly; on `M6` it leaves **two independent**
  conditions and has **no nonzero solution**. The integer 7 is a consequence of
  `M18`'s isotropic quartic.

## 6. The one new result, and it is negative

> **Separable no-go.** Degree-blindness on a tensor-product complex forces the
> symbol to be separable, `sum_i f(k_i)`. For **any** `f`, the coefficient of
> `k_i^2 k_j^2` in the quartic term is **identically zero** (verified
> symbolically). But `O(k^4)` isotropy requires the quartic to be proportional
> to `S2^2 = Q4 + 2 sum_{i<j} k_i^2 k_j^2`, which has a nonzero cross term.
> **Degree-blindness and `O(k^4)` isotropy are incompatible for every
> tensor-product lattice de Rham complex.**

The only escape, `f''''(0) = 0`, requires `f(k) = const . k^2` exactly, which is
impossible for a periodic symbol. So this is not repairable by a longer-range
1-D differential: Part A's virtue and FTD-0411/PL-5's virtue are bought by
**structurally incompatible stencils**, and the whole class is closed rather
than the instance.

This also closes the Kähler–Dirac operator as a candidate improved matter
stencil: `E^2 = M6(k)` has quartic `-Q4/12` against a quartic-free flux pole.

## 7. Ancillary findings

- **Attack on subspaces fails to bite.** Since `Delta_p = M(k) I` commutes with
  everything, Gauss projection and gauge fixing cannot change the shape.
- **But the stated hypothesis was too weak.** `K(xi) = d*d + xi . dd*` is built
  entirely from the complex's own `d` and `d*` and carries *two* dispersions —
  `M(k)` with multiplicity 2 and `xi . M(k)` with multiplicity 1 — unless
  `xi = 1`. So "built from `d` and `d*`" does **not** imply `omega^2 = C.M`. Under
  the honest hypothesis Part B is near-tautological.
- **Stability is not the cost.** Leapfrog needs `c^2 . max M <= 4`;
  `max M18 = 16/3` gives `c^2 <= 3/4`, `max M6 = 12` gives `c^2 <= 1/3` — so the
  production value sits *exactly on* the de Rham CFL boundary, marginally stable
  with zero margin. Worth recording, but not the binding objection.
- **A common anisotropic cone would close the inter-sector leak** at tree level
  (ratio 1 in every direction), but the surviving single cone is then
  anisotropic at `O(k^4)` rather than `O(k^6)` — the PL-5 regression above.

## 8. Record

Ninth refuted construction of the session, same failure mode as the other eight:
exact arithmetic attached to an interpretation never checked against the
framework's registered results. Two of those results — PL-5 and `c^2 = 1/7` —
would have been destroyed by the prescription, and both were reachable by
reading documents the proposal itself cited.

**Registered here:** Part A as a verified but `[NOT ORIGINAL]` and `[INERT]`
restatement of the Künneth/Kähler–Dirac structure, and the separable no-go as
the genuinely new content. **The residual open problem is unchanged**: §24's
demand for a cone exact by symmetry stands untouched, and the marginal direction
`Z^x/Z^t` remains unprotected.
