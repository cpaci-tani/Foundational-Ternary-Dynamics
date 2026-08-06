# Theorem — Endogenous Reaction-Carrier Bound

**FTD ID:** FTD-0586  
**Status:** `[THEOREM — EXACT FINITE-VOLUME MODAL RESPONSE BOUND]` +
`[NUMERICAL FACT — FOUR REGISTERED SPECTRAL SUMS]` +
`[MEASURED — 96 SANITIZED LIVE ARMS]` +
`[CLOSED NEGATIVE — ENDOGENOUS AUTOCATALYSIS FOR N <= 3]` +
`[BOUNDARY SUPERSEDED BY FTD-0588/0589 — CLOSED THROUGH N=6]`  
**Date:** 2026-07-26  
**Verdict:**
`ENDOGENOUS_N_LE_3_AUTOCATALYSIS_CLOSED_BOUND_INCONCLUSIVE_AT_N_GE_4`

## 1. Scope

Consider the frozen single-substrate unit-tick sector with only native wave
propagation, state-gradient coupling, and genesis/evaporation enabled. Initial
flux, wave velocity, matter velocity, and subcell remainder are zero. The
Gauss projector, damping, forces, movement, pair production, weak
transmutation, clocks, and stochastic baths are absent.

There are `N` initial ternary sources. Each source either remains fixed or is
removed once by evaporation. The theorem decides whether these sources can
produce the **first** new genesis event through the causal coded source
`-G_C grad s`.

## 2. Exact modal response

For a Fourier mode of the production 18-point stencil,

\[
 M(k)=4-\frac23\sum_a\cos k_a
      -\frac23\sum_{a<b}\cos k_a\cos k_b,
 \qquad a(k)=C_{\rm WAVE}^2M(k).
\]

The field recurrence with one fixed source is

\[
 J_{n+1}-2J_n+J_{n-1}+aJ_n=f,
 \qquad f_k=-iG_C\sin(k)s_k.
\]

Let `cos(theta)=1-a/2`. Because the symbol is multi-affine in the three
cosines, its extrema occur at cube vertices. Direct evaluation gives

\[
 0\le M(k)\le\frac{16}{3},
 \qquad 0\le a(k)\le\frac{16}{9}<4.
\]

Thus every nonzero mode is stable. For zero initial field the normalized step
response is exactly

\[
 r_n=1-\cos(n\theta)+\tan(\theta/2)\sin(n\theta).
\]

The oscillatory part has amplitude

\[
 \sqrt{1+\tan^2(\theta/2)}=\sec(\theta/2)
 =\frac1{\sqrt{1-a/4}},
\]

so

\[
 |r_n|\le D(k):=1+\frac1{\sqrt{1-a(k)/4}}.
\]

## 3. Rectangular-pulse theorem

If a source evaporates at tick `T`, its history is one rectangular pulse. By
linearity its response is the difference between a step beginning at zero and
a delayed step beginning at `T`. Therefore every mode is bounded by `2D(k)`.

For one unit ternary source on a periodic `L^3` quotient, Fourier inversion
and the vector triangle inequality give the pointwise bound

\[
 \boxed{
 B_L=\frac{2G_C}{C_{\rm WAVE}^2L^3}
 \sum_{k\ne0}
 \left(1+\frac1{\sqrt{1-C_{\rm WAVE}^2M(k)/4}}\right)
 \frac{\sqrt{\sum_a\sin^2k_a}}{M(k)}.}
\]

Source position and polarity contribute only unit-modulus Fourier phases.
Consequently, for any arrangement and polarity choice of `N` such sources,

\[
 \boxed{|J(x,n)|\le N B_L}
\]

until the first genesis event.

This conditional phrase closes by induction. If `N B_L<K_GENESIS`, assume a
first genesis event occurs. Before that event every source history has exactly
the allowed on/off form, so the bound applies at the candidate site and makes
the threshold predicate false, a contradiction. Hence no first event exists.

## 4. Registered finite-volume evaluation

The spectral sums are numerical facts evaluated independently in C++ long
double and Python binary64 on fixed preregistered volumes:

| `L` | one-source step bound | one-source pulse bound `B_L` | `3B_L` | `K_GENESIS-3B_L` |
|---:|---:|---:|---:|---:|
| 9 | 0.19045523883737153 | 0.38091047767474306 | 1.1427314330242293 | 0.37365462612774847 |
| 17 | 0.19256408532504496 | 0.38512817065008992 | 1.1553845119502697 | 0.36100154720170807 |
| 33 | 0.19315613831405509 | 0.38631227662811018 | 1.1589368298843306 | 0.35744922926764722 |
| 65 | 0.19331414902334521 | 0.38662829804669041 | 1.1598848941400712 | 0.35650116501190654 |

Thus every registered volume satisfies

\[
 3B_L<K_{\rm GENESIS}=1.5163860591519780.
\]

Using the largest registered bound,

\[
 \left\lfloor\frac{K_{\rm GENESIS}-10^{-12}}{\max_LB_L}\right\rfloor=3.
\]

Four sources are the first count **not excluded by this inequality** because
`4 max(B_L)=1.5465131921867616>K_GENESIS`. This does not prove that four
sources reach threshold; phase cancellation, spatial geometry, evaporation,
and genesis drain remain unresolved.

## 5. Production conformance

The locked live observer ran 96 endogenous arms:

- `L={9,17}`;
- both polarities and all six axial orientations;
- one-site and three-site seeds;
- constant locked sources and unlocked evaporation pulses;
- 128 ticks per arm, 12,288 aggregate ticks.

Every endogenous arm recorded zero genesis events. Support always remained a
subset of the initial support. All 96 initial sites in the unlocked arm set
evaporated, exercising the delayed off-step. The maximum observed flux was
`0.080678735152695802`, the bound excess was zero, and velocity/remainder
remained bit-exact zero. An on/off history-journal pair preserved the selected
state hash and RNG hash exactly.

Four external local-field controls at magnitude `100` each produced one
accepted genesis event. The null endogenous result is therefore not a dead
genesis detector.

## 6. Consequence and boundary

FTD-0474 remains a valid observation of finite-support manifestation patterns
after large external flux injection. FTD-0586 proves that this observation
cannot be reinterpreted as self-ignition from one, two, or three sanitized
ternary sources in the causal prescribed-source sector. FTD-0587 later
classifies its qualified dispersal tail more sharply as a selected-Gauss-
stabilized evaporative remnant with zero post-cut genesis.

The result does not cover:

- legacy Gauss projection or dual-substrate genesis;
- damping, forces, moving sources, or inherited void memory;
- an externally incident wave packet;
- four or more initial sources;
- persistence after removal of an external drive;
- identity transport, a particle pole, common action, or reciprocal recoil.

FTD-0588 subsequently proves that the apparent four-source opening came from
sourcewise triangle counting: exact spatial Parseval closes all asynchronous
histories through `N=4` and all common histories through `N=5`. The surviving
causal boundary is common `N>=6` or the all-originals-removed residual tail of
an asynchronous five-source history. Numerically searching cluster shapes or
amplitudes until genesis occurs would not turn that remainder into a native
derivation.

FTD-0589 then cancels the constant step pieces within every finite pulse and
closes arbitrary one-time-removal histories through `N=6`. The first count not
excluded by the combined analytic bound is now `N=7`.

## 7. Verification

- preregistration SHA-256:
  `2AB91067BD68FC995BDF0318843E074ADF027ADE899F9F2DF1688C0D07F64251`;
- native CTest: PASS;
- independent symbolic/spectral/result proof: 72/72 PASS;
- external sensitivity controls: 4/4 accepted genesis;
- production/default/toggle/scenario changes: none.
