# Theorem — Collective Source-History Bound

**FTD ID:** FTD-0588  
**Status:** `[THEOREM — EXACT FINITE-VOLUME COLLECTIVE-SOURCE BOUNDS]` +
`[NUMERICAL FACT — FOUR REGISTERED SPECTRAL SUMS]` +
`[MEASURED — 128 SANITIZED CONFORMANCE ARMS]` +
`[CLOSED NEGATIVE — COMMON N <= 5; ASYNCHRONOUS N <= 4]` +
`[BOUNDARY SUPERSEDED BY FTD-0589 — ARBITRARY N <= 6 CLOSED]`  
**Date:** 2026-07-26  
**Verdict:**
`COMMON_N_LE_5_ASYNC_N_LE_4_CLOSED_N5_RESIDUAL_TAIL_UNRESOLVED`

## 1. Scope

Use the frozen single-substrate, unit-tick, periodic-quotient sector containing
only the production 18-point wave operator, native state-gradient coupling,
and genesis/evaporation. Initial flux, wave velocity, matter velocity, and
remainder vanish. Gauss projection, damping, forces, movement, reactions,
clocks, and baths are absent.

There are `N` distinct stationary ternary sites with arbitrary positions and
polarity signs. The theorem bounds the field before the first descendant
genesis event. A common history means that every source has the same temporal
indicator: all remain present, or all are removed at one shared tick. An
asynchronous history lets every source be removed once at its own tick.

## 2. A stencil inequality forced by the production operator

Put

\[
 M(k)=4-\frac23\sum_a c_a-\frac23\sum_{a<b}c_ac_b,
 \qquad c_a=\cos k_a,
\]

and let the centered-gradient symbol have squared magnitude

\[
 g(k)^2=\sum_a\sin^2 k_a.
\]

Write `p=c_x+c_y+c_z` and `q=c_x^2+c_y^2+c_z^2`. Direct expansion gives

\[
 \boxed{
 3\bigl(M-g^2\bigr)
 =4\left(q-\frac{p^2}{3}\right)+\frac{(p-3)^2}{3}.}
\]

Both terms are nonnegative. Indeed,

\[
 3q-p^2=(c_x-c_y)^2+(c_x-c_z)^2+(c_y-c_z)^2.
\]

Equality requires equal cosines and `p=3`, hence `c_x=c_y=c_z=1`.
Therefore

\[
 \boxed{g(k)^2<M(k)}
\]

for every nonzero mode on a finite periodic quotient. This inequality is a
property of the matched production gradient and 18-point stencil; it is not a
continuum assumption.

## 3. Spatial orthogonality replaces linear source counting

For source positions `x_j` and signs `q_j in {-1,+1}`, define

\[
 S(k)=\sum_{j=1}^{N}q_j e^{-ik\cdot x_j}.
\]

Character orthogonality on the finite translation group gives

\[
 \begin{aligned}
 \sum_k|S(k)|^2
 &=\sum_{i,j}q_iq_j\sum_k e^{-ik\cdot(x_i-x_j)}\\
 &=L^3\sum_jq_j^2
 =L^3N.
 \end{aligned}
\]

This exact Parseval identity is why distinct synchronized sources carry
`sqrt(N)`, not `N`, in the geometry-independent pointwise bound.

## 4. Common-history theorem

FTD-0586 derived the zero-field modal step response `r_n(k)` and its envelope

\[
 |r_n(k)|\le D(k)
 =1+\frac1{\sqrt{1-C_{\rm WAVE}^2M(k)/4}}.
\]

For a common step, Fourier inversion gives

\[
 J(x,n)=\frac{G_C}{C_{\rm WAVE}^2L^3}
 \sum_{k\ne0}r_n(k)\frac{-i\sin k}{M(k)}S(k)e^{ik\cdot x}.
\]

Cauchy--Schwarz, the stencil inequality, and Parseval yield

\[
 \begin{aligned}
 |J(x,n)|
 &\le\frac{G_C}{C_{\rm WAVE}^2L^3}
 \left(\sum_{k\ne0}\frac{D(k)^2}{M(k)}\right)^{1/2}
 \left(\sum_{k\ne0}\frac{g(k)^2}{M(k)}|S(k)|^2\right)^{1/2}\\
 &\le C_L\sqrt N,
 \end{aligned}
\]

where

\[
 \boxed{
 C_L=\frac{G_C}{C_{\rm WAVE}^2}
 \sqrt{\frac1{L^3}\sum_{k\ne0}\frac{D(k)^2}{M(k)}}.}
\]

A synchronized rectangular pulse is the difference of two common steps, so

\[
 \boxed{|J(x,n)|\le2C_L\sqrt N.}
\]

The bound holds for every distinct source arrangement and every sign choice;
the tetrahedral live fixture is not an assumption of the theorem.

## 5. Asynchronous-removal theorem

Let source `j` be removed at `T_j`, if it is removed at all. Before first
genesis, linearity decomposes the field as

\[
 J=J_{\rm all-on}-\sum_{j:T_j\le n}J_{{\rm step},j}(n-T_j).
\]

If `r` sources have been removed, the common initial term obeys
`C_L sqrt(N)` and every delayed one-source step obeys the FTD-0586 triangle
bound `B_L^step`. Hence

\[
 \boxed{|J(x,n)|\le C_L\sqrt N+rB_L^{\rm step}.}
\]

For `N=4`, `r<=4`, which produces a uniform all-history bound. For `N=5`,
`r<=4` while at least one original source remains. A first-event induction
closes each subcritical case: if a first descendant event existed, its field
would still be governed by the corresponding prescribed histories and would
lie strictly below the genesis threshold, contradicting the event predicate.

The theorem does not close the `N=5, r=5` residual field after every original
source has disappeared.

## 6. Registered finite-volume values

The spectral sums were evaluated independently in C++ long double and Python
binary64:

| `L` | `C_L` | common pulse `N=5` | margin | asynchronous `N=4` | margin | `N=5`, one original remains | margin | all-off `N=5` envelope |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 9 | 0.30397065730643763 | 1.3593981058049753 | 0.15698795334700244 | 1.3697622699623615 | 0.14662378918961627 | 1.4415200082519739 | 0.07486605090000387 | 1.6319752470893452 |
| 17 | 0.30909222934825376 | 1.3823024722793018 | 0.13408358687267596 | 1.3884407999966872 | 0.12794525915529054 | 1.4614075774398307 | 0.05497848171214703 | 1.6539716627648757 |
| 33 | 0.31182601851585356 | 1.3945283491091134 | 0.12185771004286439 | 1.3962765902879275 | 0.12010946886405027 | 1.4698887278107771 | 0.04649733134120071 | 1.6630448661248320 |
| 65 | 0.31324294475520342 | 1.4008650358896921 | 0.11552102326228564 | 1.3997424856037877 | 0.11664357354819010 | 1.4736891140382269 | 0.04269694511375088 | 1.6670032630615721 |

Here `K_GENESIS=1.5163860591519780`. Thus:

- every common permanent/synchronous-pulse history is closed through `N=5`;
- every asynchronous one-time-removal history is closed through `N=4`;
- `N=5` is closed until the last original source disappears;
- the all-off `N=5` envelope is inconclusive;
- common `N=6` is not uniformly excluded because its bound is
  `1.5276292677750012` at `L=33` and `1.5345707603541368` at `L=65`.

## 7. Production conformance

The preregistered observer executed 128 arms and 16,384 ticks:

- 64 locked-step/synchronized-pulse arms at `N={4,5}`;
- 64 native unlocked arms at `N={4,5}`;
- both polarities, both tetrahedral chiralities, registered translations,
  `L={9,17}`, and four fixed unlocked seeds.

Results:

- common-history genesis: `0`;
- asynchronous four-source genesis: `0`;
- unlocked five-source genesis: `0`;
- analytic contradictions/bound excess: `0`;
- all 64 unlocked arms removed every original source;
- 288/288 initial unlocked sources evaporated exactly once;
- maximum observed closed-scope flux: `0.071895466243716816`;
- velocity and remainder: bit-exact zero;
- observer state/RNG neutrality: exact.

The five-source residual tail was therefore exercised in these fixed
moment-isotropic fixtures and was negative. That is a measurement, not a
universal theorem over all removal schedules. FTD-0589 subsequently supplies
that universal closure by cancelling the constant step terms inside each
finite pulse.

## 8. Consequence and boundary

FTD-0586's `N>=4` open boundary was an artifact of sourcewise triangle
counting. At the time of this theorem, exact spatial orthogonality moved the
universal boundary to:

- `N>=6` for common histories;
- `N>=5` for arbitrary one-time removal histories;
- specifically, for `N=5`, only the all-originals-removed residual tail.

FTD-0589 supersedes that residual boundary: the exact rectangular-pulse
identity closes arbitrary one-time-removal histories through `N=6`. The first
count not excluded by the combined bound is now `N=7`.

Nothing in this theorem supplies field-to-matter recoil, momentum exchange,
mobile identity, a pole, or a self-maintaining reaction front. The causal
state-gradient source remains a prescribed dressing mechanism whose source
capacity is now more sharply bounded.

## 9. Verification

- preregistration SHA-256:
  `06DE9E8B896272044D847FF5BEC53A342928E3B210B61AC4D3AD605D9D36692E`;
- native CTest: PASS;
- independent symbolic/spectral/result proof: 127/127 PASS;
- CSV arms: 128/128 valid;
- production/default/toggle/scenario changes: none.
