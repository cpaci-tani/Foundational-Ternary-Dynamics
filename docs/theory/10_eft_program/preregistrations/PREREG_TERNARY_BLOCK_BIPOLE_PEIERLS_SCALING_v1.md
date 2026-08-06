# FTD-0621 — Ternary block-bipole Peierls scaling v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CONFIRMATORY RUN]`  
**Scope:** observer-only representability gate; no production adoption  
**Date:** 2026-07-27

## 1. Question

Can an exactly ternary, finite, neutral, integer-site configuration move its
electrostatic spectral weight toward the infrared as its physical width grows,
without fractional polarity, a fitted envelope, or a new primitive?

This is a necessary but insufficient precursor to an extended dynamical
carrier. A positive result establishes representational capacity only. It does
not establish binding, stability, mobility, a particle pole, electromagnetic
charge, or a physical matter ontology.

## 2. Locked native family

For width `w >= 1` and orientation `d in {x,y,z}`, define two adjacent equal
blocks. In coordinates whose `d` component is written first,

\[
s_{w,d}(n)=
\begin{cases}
+1,&0\le n_d<w,\quad 0\le n_j<w\ (j\ne d),\\
-1,&w\le n_d<2w,\quad 0\le n_j<w\ (j\ne d),\\
0,&\text{otherwise}.
\end{cases}
\]

The family therefore has exactly `w^3` positive sites, `w^3` negative sites,
`2w^3` occupied sites, zero net polarity, finite support, and no site whose
primitive state is other than `-1`, `0`, or `+1`. The three orientations and
their polarity mirrors form a cubic orbit. No coefficient is normalized by
the number of occupied sites.

This block bipole is selected as the smallest deterministic three-dimensional
integer family with all of the following simultaneously: exact neutrality,
connected occupied support, a width parameter in every spatial direction,
and an analytic Fourier transform. It is not asserted to be a physical bound
state.

## 3. Exact observer

Define

\[
A_w(k)=\sum_{r=0}^{w-1}e^{-ikr},\qquad
\widetilde s_{w,d}(k)=
 A_w(k_x)A_w(k_y)A_w(k_z)(1-e^{-iwk_d}).
\]

With the existing FTD-0541 quadratic coat,

\[
b_i(k_i)=\frac{3+\cos k_i}{4},\qquad
\lambda(k)=2\sum_i(1-\cos k_i),
\]

the locked finite-volume field energy is

\[
E_{w,d}=\frac{\beta}{2L^3}\sum_{k\ne0}
 \frac{|\widetilde s_{w,d}(k)|^2\prod_j b_j(k_j)^2}
 {\lambda(k)}.
\]

For translation along axis `i`, the exact compact-coat Peierls coefficient and
half-cell barrier are

\[
C_{w,d;i}=\frac{\beta}{2L^3}\sum_{k\ne0}
 \frac{|\widetilde s_{w,d}(k)|^2(1-\cos k_i)^2
       \prod_{j\ne i}b_j(k_j)^2}{\lambda(k)},
\qquad B_{w,d;i}=\frac{C_{w,d;i}}{16}.
\]

The dimensionless pinning index must independently satisfy

\[
\Pi_{w,d;i}=\frac{B_{w,d;i}}{E_{w,d}}
=\left\langle
 \left(\frac{1-\cos k_i}{3+\cos k_i}\right)^2
 \right\rangle_{E}.
\]

Every finite nonzero member must retain `B > 0`, consistently with FTD-0579.
The question is whether `B/E` tends toward zero with width.

## 4. Locked confirmatory matrix

- main volume: `L=257`;
- replication volume: `L=193`;
- held-out widths: `w={5,9,15,23,35}`;
- all three block orientations;
- all three translation axes;
- asymptotic fit window: `w={9,15,23,35}`;
- normalization: the unchanged FTD-0468 face-field work coefficient `beta`;
- no dynamics, relaxation, force amplification, smoothing, fractional
  occupancy, source renormalization, or post-run width selection.

The support must obey `2w < L` in every arm.

## 5. Locked algebraic and covariance gates

1. Counts and neutrality are exact integers.
2. The analytic structure factor agrees with direct summation at the fixed
   validation modes to relative residual `<=1e-11`.
3. The spectral identity for `Pi` closes to `<=1e-12`.
4. Cubic rotations permute `(d,i)` results to relative residual `<=1e-12`.
5. The two directions transverse to the block dipole agree to relative
   residual `<=1e-12`.
6. Every energy and finite-width barrier is strictly positive and finite.
7. Main/replication `Pi` values agree within 8 percent for `w<=23`. The
   `w=35` replication arm is recorded but excluded from this finite-volume
   gate because its full dipole length is a substantial fraction of `L=193`.

Failure of an algebraic gate yields
`TERNARY_BLOCK_BIPOLE_OBSERVER_INVALID` and no physical inference.

## 6. Locked scaling discriminator

Continuum dimensional scaling of a fixed-density three-dimensional bipole
predicts

\[
E=O(w^5),\qquad B=O(w^2),\qquad \Pi=O(w^{-3}).
\]

For both the parallel class `i=d` and transverse class `i!=d`, ordinary least
squares on the locked log-log fit window must give

- energy slope in `[4.65,5.35]`;
- absolute half-cell barrier slope in `[1.65,2.35]`;
- pinning-index slope in `[-3.35,-2.65]`;
- strict decrease of `Pi` at every successive registered width;
- main-volume `Pi(w=35) < 5e-5` in all nine orientation/translation arms.

The endpoint threshold is an engineering-resolution discriminator, not an
experimental claim and not evidence independent of the disclosed pilot in
Section 8. The slope and held-out covariance tests carry the structural
weight.

## 7. Verdicts and consequences

- `INTEGER_TERNARY_EXTENSION_SUPPRESSES_PEIERLS`: all algebraic,
  finite-volume, monotonicity, slope, and endpoint gates pass. This licenses
  construction of a connected deformable common-action carrier from an exact
  integer family. It does not license a matter or particle claim.
- `INTEGER_TERNARY_SUPPRESSION_NOT_ASYMPTOTICALLY_QUALIFIED`: algebra and
  monotonic suppression pass, but at least one slope, volume, or endpoint gate
  fails. A larger dynamical carrier is not yet licensed as an infrared repair.
- `INTEGER_TERNARY_EXTENSION_DOES_NOT_SUPPRESS_PINNING`: algebra passes but
  `Pi` is not strictly decreasing in both directional classes.
- `TERNARY_BLOCK_BIPOLE_OBSERVER_INVALID`: an exact observer/covariance gate
  fails.

If the first verdict fires, FTD-0622 must test the same integer architecture in
one connected action and measure Peierls index and translation-reaction defect
together. Independent copies do not qualify. If it does not fire, the program
must redesign the integer architecture as a new locked candidate or escalate
to a field/connection carrier; it may not replace this run post hoc with a
fractional favorable envelope.

## 8. Pilot disclosure

Before this document was locked, a local unrecorded NumPy pilot evaluated the
already-chosen analytic block-bipole formula for `w={1,2,4,8,16,32}` at
`L={129,257}`. It showed positive finite barriers and monotone relative
suppression. None of those widths belongs to the confirmatory matrix. The
pilot means this is not a blind discovery test. The held-out slopes, volume
stability, exact ternary construction, and covariance checks remain capable of
falsifying the proposed asymptotic interpretation.

## 9. Scope locks

- Observer-only; production tick, defaults, postulates, `RenderBridge`, and
  scenarios remain unchanged.
- No physical matter, charge, photon, Lorentz, unitarity, or pole claim may be
  promoted by this campaign.
- The prior finite-rigid obstruction remains exact at every finite `w`; this
  campaign tests suppression, not exact removal.
- A positive representability result does not show that the production engine
  can create, bind, preserve, or move the registered pattern.
