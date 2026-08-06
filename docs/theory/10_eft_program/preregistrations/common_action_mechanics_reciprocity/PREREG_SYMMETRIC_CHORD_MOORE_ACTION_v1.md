# Pre-registration — Symmetric chord Moore action (FTD-0580)

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Date:** 2026-07-26  
**Parents:** FTD-0478, FTD-0576, FTD-0577, FTD-0578, FTD-0579.  
**Production changes permitted:** none. Observer code, exact proof, test,
theorem, audit, and documentation reconciliation only.

## 1. Question

FTD-0578 found that the straight trilinear trajectory is time-exact but not
endpoint-energy-centered on edge and body diagonals. FTD-0579 proved that no
finite rigid extension cures that defect or Peierls pinning. This campaign
asks whether positivity and exact energy centering instead select a different
subcell coupling shape, whether its face current has a route-free symmetric
construction, and whether that construction removes self-pinning.

No production shape, movement rule, hidden route label, counterforce,
self-field subtraction, fitted coefficient, toggle, or scenario is permitted.

## 2. Positive energy-centered shape theorem

Let `d in {-1,0,+1}^3 \ {0}` and let `p_t(n)` be a nonnegative coupling
distribution for a unit-polarity hop from site `0` to site `d`. Require

```text
p_0=delta_0,             p_1=delta_d,
sum_n p_t(n)=1,          sum_n n p_t(n)=t d,
integral_0^1 p_t dt=(delta_0+delta_d)/2.                (1)
```

For every site other than `0,d`, nonnegativity and the last equality force
`p_t(n)=0` almost everywhere. The first moment then fixes the remaining two
weights. The unique continuous representative is

```text
p_t=(1-t) delta_0+t delta_d.                            (2)
```

The FTD-0577 Moore coat is applied after (2): `rho_t=B_M p_t`. Because the
finite Laurent ring is an integral domain, the coat does not weaken the
finite-support uniqueness statement.

This chord shape is a deterministic coupling sidecar. It is not the FTD-0478
trilinear straight-segment density and is not a new primitive state.

## 3. Democratic shortest-path face current

Let `A={i:d_i!=0}` and `D=|A|`. Average uniformly over all `D!` monotone
shortest face paths from `0` to `d`. Equivalently, for a subset
`S subset A\{i}`, the oriented edge from vertex `v_S` in direction `d_i`
has unsigned weight

```text
w(S,i)=|S|! (D-|S|-1)!/D!.                             (3)
```

At every intermediate subset vertex, total incoming and outgoing weight both
equal `|S|!(D-|S|)!/D!`; the start has unit outward flow and the endpoint has
unit inward flow. Therefore the raw face current `K_d` obeys

```text
d_f K_d=delta_0-delta_d.                                (4)
```

Equation (3) is invariant under permutations and sign changes of the active
axes. It removes an arbitrary x/y/z route choice within the explicitly
selected class of monotone shortest-path face currents. Curl additions and
non-shortest paths remain outside that class and are not claimed unique.

Apply the FTD-0577 bridge

```text
q_i=A_i product_(j!=i) B_j K_i.                         (5)
```

Then `D_c q=rho_0-rho_1` exactly.

## 4. Exact temporal action

With temporal hats `w_0=1-t`, `w_1=t`, (2) gives

```text
T_0=rho_0/3+rho_1/6,
T_1=rho_0/6+rho_1/3,
T=T_0+T_1=(rho_0+rho_1)/2,
Q_0=Q_1=q/2.                                            (6)
```

Thus

```text
D_c Q_0=rho_0-T,        D_c Q_1=T-rho_1.               (7)
```

For the FTD-0576 field-work coordinate `R=J-W/2`, the common interaction

```text
I_ch=G_C sum_(a=0,1) [<T_a,D R_a>+<Q_a,C R_a>]         (8)
```

is simultaneously the exact time integral for the chord history and the
endpoint-energy-centered source on every Moore direction. Its source and
probe terms are adjoints of the same selected functional.

## 5. Peierls discriminator

For a subcell coordinate `r`,

```text
rho_hat_r=B_M[(1-r)+r exp(-i k dot d)].                  (9)
```

The FTD-0575 Hodge response gives

```text
V_self(r)=V_self(0)+C_d r(1-r),                         (10)

C_d=(G_C^2/L^3) sum_k R_H(k) B_M(k)^2
                         [1-cos(k dot d)].              (11)
```

For every nonzero integer `d`, `C_d>0` on the infinite lattice: the integrand
is nonnegative and strictly positive on an open set. The chord repair can
close energy centering but must not be called a gapless mobile law if this
barrier remains positive.

## 6. Registered arms

Exact proof:

- positivity/average uniqueness of (2);
- the subset-flow identity (3)--(4) for axial, edge, and body classes;
- the temporal integrals (6)--(8);
- the quadratic law (9)--(11) and strict positivity.

Compiled observer:

- `L in {17,33}`, both polarities, all 26 signed Moore directions;
- `t in {0,1/8,...,1}`: 936 shape samples;
- 104 raw-face, coated-central, temporal-centering, and split-continuity arms;
- 104 Peierls coefficients and 936 direct potential samples;
- all 24 proper cubic rotations of one body-diagonal reference;
- frozen production hash/default/toggle/scenario checks.

Partition, first moment, wrong-sign weight, raw continuity, coated continuity,
temporal centering, split continuity, direct/spectral potential law, polarity,
and cubic covariance residuals must be `<=1e-12`. Every Peierls coefficient
and half-cell barrier must be strictly positive and exceed `1e-14`.

## 7. Outcome map

If all registered gates pass, record

```text
SYMMETRIC_CHORD_CLOSES_MOORE_CENTERING_PEIERLS_PINNING_REMAINS
```

This is a constructive partial result: positivity plus exact centering selects
the chord shape; democratic shortest-path routing supplies an exact local
face current; the common action closes the endpoint-energy ledger; and a
positive Peierls barrier still closes the unmodified chord as gapless mobile
matter. It licenses no production implementation.

## 8. Frozen production provenance

```text
phase_read.cpp                  D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8
phase_write.cpp                 2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4
field_operators.h               25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48
native_energy_contract.h        3DB8F2DC573E7F4A87E17409878915E7B5A52CE1673713998C544516E0175621
```

