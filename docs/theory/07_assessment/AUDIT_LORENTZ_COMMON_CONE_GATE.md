# AUDIT — Common-cone gate after the BCC-time construction

**Registry:** FTD-0412  
**Status:** `[CORRECTION — Wilson real-time operator]` + `[SELECTED LEADING-CONE DIAGNOSTIC]` + `[SCOPED NO-GO — nearest-neighbour scalar-r Wilson quartic matching]` + `[OPEN — dynamical gauge/gravity poles and interacting common cone]`  
**Verdict:** `LEADING-CONE-ALIGNABLE; LIVE-COMMON-CONE-FAILS`  
**Verification:** `scripts/proofs/proof_lorentz_common_cone.py`; native `lorentz_common_cone`  
**Date:** 2026-07-22

> **FTD-0413 successor note.** The scalar-`r` no-go in this audit remains
> exact for its axial kinetic hypothesis `K_i=sin(q_i)`. FTD-0413 exits that
> hypothesis by adding a normalized face-diagonal kinetic weight. It constructs
> a selected SC+FCC-local free common cone through q4, while leaving q6 and the
> live/interacting multi-sector gates open.

## 0. Result

FTD-0411 supplies a selected flux-wave cone with `c^2=1/7`. That does not by
itself recover a physical light cone. Lorentz recovery requires every
propagating observable sector to share one limiting pole after a single
overall choice of time units.

The current engine does not meet that condition:

| sector | current object | leading speed or status |
|---|---|---|
| default free flux | production SC+FCC wave update | `c^2=1/3` |
| BCC-time flux prototype | selected period-two surrogate | `c^2=1/7` |
| manifested matter | imposed causal budget, not a measured pole | `C_SPEED^2=1/3` |
| standalone Wilson matter | inserted Branch-B spatial operator | raw normalization `c_s^2=1` |
| gauge links | background/relaxation variables | no dynamical gauge pole |
| native latency gravity | elliptic Poisson solve | no frequency-dependent gravity pole |
| CosmicEngine gravity waves | phenomenological radius update | imposed `C_SPEED^2=1/3` |

The Wilson sector also contained a prior operator-identification error. The
code evolved the spatial Wilson operator `D_W` directly and treated a norm
identity on special upper-only spinors as its energy spectrum. Generic
spinors do not obey that identity. FTD-0412 separates the retained spatial
`D_W` diagnostic from a corrected Hermitian real-time Wilson Hamiltonian
`H_W`.

That correction invalidates the physical interpretation of the historical
FTD-0126 orbit and `g-2` numbers. They were produced by the wrong real-time
operator and cannot stand as a Wilson-Dirac measurement. Their files and
numbers remain provenance for the retired implementation; the quantitative
campaign must be rerun before any replacement outcome is assigned. The
separate conceptual fact that a fixed classical magnetic background contains
no photon loop remains true, but it does not rescue the retired measurement.

A direct run of the corrected historical orbit fixture confirms the
distinction. Energy and norm are conserved (`2.25e-7` and `1.39e-7` relative
drift), but the packet spans the periodic box and fails the locked bounded-orbit
criterion. The benchmark is therefore disabled as a default CTest gate and
retained as a manual research instrument. Its criterion was not weakened.

For massless Wilson matter, selecting `c_s^2=1/7` makes the leading matter
slope equal to the BCC-time flux slope. This is a calibration, not a
derivation from P1–P5. It does not produce a full common pole: for every scalar
Wilson parameter `r`, the matter pole has a nonzero direction-dependent
quartic correction whereas the FTD-0411 flux pole is quartic-free.

Therefore LR-2 remains failed. The result achieved here is only a corrected
matter pole plus a precise boundary on the simplest attempted alignment.

---

## 1. Why the common cone is load-bearing

One sector can always be assigned unit speed by redefining the time unit. That
removes one normalization and no more. If two independent sectors have slopes
`c_A` and `c_B`, the dimensionless ratio `c_A/c_B` survives every common unit
change. A mismatch is physical preferred-frame data, not a convention.

Consequently, the numerical statement “the flux pole is cone-like” is
insufficient. Matter supplies emitters, detectors, rods, and clocks. Gauge and
gravity excitations must also be included if the framework claims a universal
causal metric. Until their poles exist and agree, `c` is a sector parameter.

---

## 2. Correction of the Wilson real-time operator

### 2.1 What the old test actually measured

With identity links, the implemented spatial operator has momentum form

\[
D_W(\mathbf q)=M(\mathbf q)I+i c_s\sum_i\gamma^i\sin q_i,
\]

where the module's spatial gamma matrices are anti-Hermitian and

\[
M(\mathbf q)=m+c_s r\sum_i(1-\cos q_i)
\]

for `a=1`. Thus `i gamma^i` is Hermitian and the actual eigenvalues are

\[
\lambda_\pm=M\pm c_s\sqrt{\sum_i\sin^2q_i}.
\]

The old smoke and full-Brillouin-zone tests initialized upper-only spinors.
For those fixtures the cross term has zero expectation, giving

\[
\frac{\|D_W\psi\|^2}{\|\psi\|^2}=M^2+c_s^2\sum_i\sin^2q_i.
\]

That equality is fixture-dependent; it is not the operator spectrum. For
`m=1/2`, `r=1`, `q=(pi/2,0,0)`, an eigenstate of `i gamma^1` has

\[
\|D_W\psi\|^2/\|\psi\|^2=(3/2+1)^2=25/4,
\]

while the retired oracle gives `13/4`. The prior energy-dispersion
interpretation is false.

### 2.2 Corrected real-time Hamiltonian

Real-time evolution now uses

\[
H_W(\mathbf q)
=c_s\sum_i\alpha^i\sin q_i
+\beta\left[m+c_s r\sum_i(1-\cos q_i)\right],
\]

with `alpha^i=gamma^0 gamma^i` and `beta=gamma^0`. The Clifford
anticommutators give the exact free spectrum

\[
\boxed{
E_W^2(\mathbf q)
=c_s^2\sum_i\sin^2q_i
+\left[m+c_s r\sum_i(1-\cos q_i)\right]^2}.
\]

The spatial `D_W` API remains for the existing Euclidean/gauge-covariance
diagnostics. `evolve_rk4_step` now calls `H_W`. The new `spatial_speed`
parameter defaults to `1`, so existing Branch-B normalization is preserved;
the `1/sqrt(7)` value is exercised only by the FTD-0412 diagnostic.

The corrected Hamiltonian is gauge covariant under the existing link
transformation and is Hermitian for unit-modulus links. Native tests verify
its free spectrum and gauge covariance. No RenderBridge production toggle
activates this standalone sector.

---

## 3. Leading-cone alignment

Set `m=0` and write

\[
S_2=\sum_iq_i^2,\qquad Q_4=\sum_iq_i^4.
\]

The corrected Wilson pole is

\[
E_W^2
=c_s^2\left[
\sum_i\sin^2q_i
+r^2\left(\sum_i(1-\cos q_i)\right)^2
\right].
\]

Its infrared expansion is

\[
\boxed{
E_W^2
=c_s^2S_2
+c_s^2\left(\frac{r^2}{4}S_2^2-\frac13Q_4\right)
+O(q^6)}.
\]

Selecting

\[
c_s^2=\frac17
\]

therefore aligns the leading Wilson slope with the selected FTD-0411
BCC-time flux slope. Nothing in the Wilson action or P1–P5 forces this value;
`spatial_speed=1/sqrt(7)` is a common-cone calibration candidate.

This is still useful: after correcting the operator, there is no algebraic
obstruction to equal leading slopes in the free massless matter and flux
diagnostics. The obstruction begins at the next order.

---

## 4. Scalar Wilson parameter cannot match the quartic-free flux pole

FTD-0411 gives

\[
\theta_{\rm flux}^2=\frac17S_2+O(q^6),
\]

with no quartic term. Matching this with nearest-neighbour Wilson matter would
require

\[
\frac{r^2}{4}S_2^2-\frac13Q_4=0
\]

for every direction.

On an axis, `q=(t,0,0)`, this condition gives

\[
r^2=\frac43.
\]

On a face diagonal, `q=(t,t,0)`, it gives

\[
r^2=\frac23.
\]

The requirements contradict each other. Hence:

> **Nearest-neighbour scalar-r no-go.** No scalar Wilson parameter `r`
> makes the corrected nearest-neighbour massless Wilson pole share the
> FTD-0411 flux pole through quartic order in all spatial directions.

This theorem is deliberately narrow. It does not exclude an improved fermion
derivative, direction-dependent higher-shell counterterms, a multi-field
matter transfer, or interacting renormalization. Each escape adds structure
that must be derived or explicitly selected and then retested over the full
Brillouin zone.

---

## 5. Gauge and gravity are absent from the common-cone comparison

The engine's SU(2)/SU(3) links undergo an imposed Wilson-staple relaxation and
are write-only with respect to the substrate. The Wilson `GaugeLinks` used by
the Branch-B matter module are fixed backgrounds. Neither object supplies a
linearized gauge-field equation with a temporal frequency `omega(q)`. A
gauge-sector limiting speed is therefore not merely unequal; it is undefined.

Native latency gravity is sourced through an 18-point Poisson solve. An
elliptic constraint has no propagating pole. CosmicEngine's gravitational-wave
radius increment `radius += C_SPEED*dt` inserts the existing speed and is not a
derivation of a substrate graviton cone.

Manifested matter has a related limitation. `causal_kinematics.h` imposes
`u^2/C_SPEED^2+latency^2<1`, but that is a kinematic budget. It is not the pole
of a linearized matter correlator. Retuning this budget from `1/3` to `1/7`
before a matter pole and a common unit map exist would replace one assumption
with another.

---

## 6. What changed and what remains open

Changed:

1. Wilson real-time RK4 evolution now uses a Hermitian Wilson Hamiltonian.
2. The smoke and full-BZ spectrum tests validate `H_W^2=E_W^2`, not the
   fixture-dependent norm of the spatial `D_W` operator.
3. Gauge covariance is tested for both retained `D_W` and corrected `H_W`.
4. `WilsonDiracParams::spatial_speed` makes the leading normalization explicit
   and preserves the legacy default `1`.
5. A default-independent native gate tests the selected `1/7` leading slope
   and the scalar-`r` quartic obstruction.
6. FTD-0126's historical orbit and `g-2` measurement is retracted as a
   physical Wilson-Dirac result because it evolved the wrong operator.
7. The corrected historical orbit fixture conserves energy and norm but fails
   localization; it is quarantined from the passing default test suite.

Not changed:

1. `C_SPEED=C_WAVE=1/sqrt(3)` remains the production default.
2. The FTD-0411 BCC-time flux prototype remains default off and CPU scoped.
3. Wilson matter remains standalone and Branch-B inserted.
4. Gauge and native gravity still lack propagating poles.
5. No interacting self-energy, operator-mixing, Ward, SME, or operational
   boost gate has passed.
6. No corrected-Hamiltonian replacement verdict for the Phase-II `g-2`
   campaign has been produced.

The next admissible LR-2 step is an improved matter stencil whose complete
quartic tensor matches the selected flux pole, followed by dynamical gauge and
hyperbolic gravity poles. Only after those objects exist can one common clock
normalization be tested rather than imposed sector by sector.

---

## 7. Reproduction

```powershell
python scripts/proofs/proof_lorentz_common_cone.py
ctest --test-dir engine/build -C Release -R lorentz_common_cone --output-on-failure
ctest --test-dir engine/build -C Release -R test_wilson_dirac --output-on-failure
```

Expected symbolic verdict:

```text
VERDICT  LEADING-CONE-ALIGNABLE; LIVE-COMMON-CONE-FAILS
```
