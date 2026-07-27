# FTD-0419 — Full-Brillouin-zone one-loop matching

**Date:** 2026-07-22  
**Status:** `[NUMERICAL FACT — frozen QED_L-like step scheme]` + `[THEOREM — action-level Ward identities; finite-grid longitudinal cancellation]` + `[SCOPED CLOSED NEGATIVE — automatic one-loop common-cone cancellation]` + `[OPEN — gauge-independent on-shell matching and ternary bridge]`  
**Verdict:** `BARE-COMMON-CONE-NOT-ONE-LOOP-FIXED; COUNTERTERM-REQUIRED-IN-FROZEN-SCHEME; PHYSICAL-ON-SHELL-VERDICT-OPEN`  
**Verifier:** `scripts/proofs/proof_lorentz_full_bz_matching.py`  
**CUDA quadrature:** `scripts/proofs/cuda/lorentz_full_bz_matching.cu`  
**Data:** `scripts/proofs/_lorentz_full_bz_matching.csv`

---

## 0. Outcome

The FTD-0417/0418 bare leading common cone is not a one-loop fixed point in
the frozen `xi=1`, one-flavour, QED_L-like finite-volume step scheme. Factoring
out the symbolic link coupling `g^2`, the deterministic full-zone calculation
gives

$$
\delta Z_s-\delta Z_t=-0.31225681\ldots,
$$

$$
\delta Z_B-\delta Z_E=+0.02942448\ldots,
$$

and therefore

$$
\boxed{
\frac{\delta_{\rm match}}{g^2}
=(\delta Z_s-\delta Z_t)
-\frac12(\delta Z_B-\delta Z_E)
=-0.32696906(5).}
$$

The parenthetical uncertainty is a conservative high-`N` extrapolation
systematic, not a Monte Carlo standard error. Every integral is a deterministic
uniform Brillouin-zone sum.

Under the already priced **selected** vertex wiring `g^2=alpha_FTD`, this
scheme-specific correction is

$$
\delta_{\rm match}=-2.38601\times10^{-3}.
$$

FTD-0416's strongest optimistic one-species running, `1/137^3`, would reduce
that only to `9.28e-10`, still approximately `9.28e5` times a declared
`1e-15` infrared tolerance. Equivalently, a counterterm of approximately
`+2.386e-3` is required at the matching scale and must be fixed to roughly one
part in `9e5` under that tolerance and RG surrogate.

This is a negative result for **automatic cancellation in the frozen
scheme**, not yet a gauge-independent experimental exclusion. The extracted
`Z` coefficients are off-shell 1PI matching coefficients in a declared gauge
and infrared scheme; they are not an on-shell observable. A physical SME
verdict requires pole matching or another gauge-independent renormalization
condition.

---

## 1. Frozen one-loop objects

Use the FTD-0418 propagators and vertices with `g^2` factored out. The fermion
propagator is

$$
S(p)=\frac{W(p)-i\sum_\mu\gamma_\mu K_\mu(p)}
{W(p)^2+\sum_\mu K_\mu(p)^2},
$$

$$
W(p)=m+\sum_\mu r_\mu(1-\cos p_\mu),
\qquad
K_\mu(p)=\nu_\mu\sin p_\mu,
$$

where `r_0=nu_0=1` and `r_i=nu_i=c=1/sqrt(7)`. In the FTD-0418 anisotropic
Feynman gauge,

$$
D_{00}(k)=\frac{c^2}{\widehat k_0^2+c^2\widehat{\mathbf k}^{,2}},
\qquad
D_{ij}(k)=\frac{\delta_{ij}}
{\widehat k_0^2+c^2\widehat{\mathbf k}^{,2}}.
$$

### 1.1 Fermion self-energy

Expanding the averaged fermion inverse, rather than adding diagrams by hand,
fixes the relative sign:

$$
\Sigma(p)=\frac12\int_{\rm BZ}
V_{\mu\mu}^{(2)}(p,p;k,-k)D_{\mu\mu}(k)
-\int_{\rm BZ}
V_\mu^{(1)}(p,p-k)S(p-k)V_\mu^{(1)}(p-k,p)D_{\mu\mu}(k).
$$

The first term is the Wilson seagull/tadpole; the second is exchange. For
`Sigma(p)=i gamma_mu p_mu nu_mu delta Z_mu+...`, analytic differentiation of
the integrand at `p=0` gives `delta Z_t` and `delta Z_s`. No finite external-
momentum difference is used for this part.

The seagull is load-bearing. At `N=40`, where the total matter difference is
already within `1.7e-5` of its step-scheme limit,

| contribution | `delta Z_s-delta Z_t` |
|---|---:|
| compulsory two-photon seagull | `-0.23746920` |
| one-photon exchange | `-0.07477118` |
| total | `-0.31224038` |

Deleting the contact term would therefore not be a harmless simplification;
it would change the answer by most of the threshold and violate the frozen
action contract.

### 1.2 Photon polarization

Expanding `-Tr log D_W[A]` gives the bubble and contact terms

$$
\Pi_{\mu\nu}(k)=
\int_{\rm BZ}\operatorname{tr}
\left[S(p)V_\mu^{(1)}S(p+k)V_\nu^{(1)}\right]
-\delta_{\mu\nu}\int_{\rm BZ}
\operatorname{tr}\left[S(p)V_{\mu\mu}^{(2)}\right].
$$

For a transverse spatial component `i`, define

$$
\delta Z_E(k_0)=\frac{\Pi_{ii}(k_0,\mathbf0)}{\widehat k_0^2},
\qquad
\delta Z_B(k_j)=\frac{\Pi_{ii}(0,k_j)}{c^2\widehat k_j^2},
\quad i\ne j.
$$

The contact term cancels the zero-momentum bubble as required by gauge
invariance. At every grid size used for the step scheme, the longitudinal
polarizations at a nonzero grid momentum cancel to between `1e-15` and
`1e-17`, without a fitted subtraction.

---

## 2. The frozen QED_L-like step scheme

The coefficient in §0 is scheme-specific. Its complete definition is:

1. `xi=1` local anisotropic Feynman gauge from FTD-0418;
2. an `N^4` coordinate torus;
3. periodic photon loop momenta, with the single global zero mode removed;
4. massless fermions antiperiodic in all four directions;
5. first nonzero bosonic external momentum `q=2pi/N`;
6. `N -> infinity` at fixed `qL=2pi`;
7. one charged Dirac species;
8. no physical value assigned to `g` during integration.

Item 6 makes this a finite-volume step-renormalization scheme, not the same
object as an infinite-volume derivative taken before the infrared limit. This
distinction matters. Changing the spin structure or introducing a positive
mass changes the finite constant while leaving the Ward identities intact.
The coefficient must therefore never be quoted without its scheme.

### 2.1 Deterministic convergence

| `N` | `delta Z_s-delta Z_t` | `delta Z_B-delta Z_E` | `delta_match/g^2` | max longitudinal residual |
|---:|---:|---:|---:|---:|
| 64 | `-0.3122501112` | `0.0291659154` | `-0.3268330689` | `7.9e-15` |
| 96 | `-0.3122537931` | `0.0293006634` | `-0.3269041248` | `4.9e-16` |
| 128 | `-0.3122551068` | `0.0293512863` | `-0.3269307499` | `4.6e-17` |
| 192 | `-0.3122560529` | `0.0293897294` | `-0.3269509176` | `1.0e-16` |
| 256 | `-0.3122563855` | `0.0294040478` | `-0.3269584094` | `2.4e-17` |
| 320 | `-0.3122565397` | `0.0294109649` | `-0.3269620222` | `2.7e-17` |

The asymptotic fit

$$
f(N)=f_\infty+\frac{a\log N+b}{N^2}+\frac{d}{N^4}
$$

gives `-0.32696906...`. Moving the lower fit cut from `N=64` to `N=96`
changes the extrapolated value by less than `5e-8`. This fit form controls the
massless finite-volume logarithm; it is not a fit to any physical constant.

The CUDA run of record evaluates as many as `320^4=10,485,760,000` momentum
points per row on the RTX 5090 through WSL2. The independent NumPy
implementation reproduces every `N=8` component to better than `5e-13`, and
an explicit `4x4` gamma-matrix multiplication independently checks the reduced
Clifford trace.

---

## 3. Infrared-scheme diagnostic

The first attempted extraction coupled `q=2pi/N` to the infinite-volume limit
without naming the resulting finite-volume scheme. Repeating it with a
different fermion spin structure produced a different photon constant. That
exposed a noncommuting order of limits before a universal number was claimed.

As a second diagnostic, introduce a positive scalar fermion mass, choose
`q=m/8`, and increase `N`. The following are finite-regulator witnesses, not
an extrapolated massless coefficient:

| `m` | largest `N` | `delta_match/g^2` |
|---:|---:|---:|
| 0.400 | 96 | `-0.2557933` |
| 0.200 | 128 | `-0.2609753` |
| 0.100 | 256 | `-0.2626993` |
| 0.050 | 320 | `-0.2619305` |
| 0.025 | 384 | `-0.2606124` |

The values are not identical to the step scheme, confirming finite-part scheme
dependence. They nevertheless preserve the sign and order of magnitude. This
supports only the scoped conclusion that the selected bare cone does not show
automatic cancellation in either tested prescription. It does not define a
scheme-independent massless limit.

---

## 4. Species and coupling dependence

For `N_f` identical charged Dirac species, the fermion self-energy correction
for one external species is unchanged at this order, while the photon bubble
is multiplied by `N_f`. In the frozen step scheme,

$$
\boxed{
\frac{\delta_{\rm match}(N_f)}{g^2}
=-0.31225681\ldots
-0.01471224\ldots N_f.}
$$

Every positive integer species count therefore makes the negative mismatch
larger; species multiplicity does not cancel it. Additional scalar, compact-
link, or non-Abelian fields would define a different action and require their
own complete matching calculation.

The integration itself leaves `g` symbolic. Substituting `g^2=alpha_FTD` is
the already declared Branch-B vertex calibration, not a derivation of the
electromagnetic coupling from the loop.

---

## 5. What the negative result closes

FTD-0419 closes the following narrow claim:

> The bare FTD-0417/0418 local gauge-matter action, with its tree coefficients
> set to the same `c^2=1/7`, automatically retains that common cone at one loop
> without a counterterm.

That claim is false in the frozen step scheme. Both the matter and gauge parts
were calculated from the same action; the Ward contacts were retained; the
longitudinal residual vanishes; and the relative coefficient is not zero.

The available repair is explicit anisotropy tuning. One may add an allowed
dimension-four counterterm and impose a common-cone renormalization condition
at one scale. That is a calibration, not emergence or radiative protection.
Its running and retuning across thresholds must then be tracked.

---

## 6. What remains open

1. **Gauge-independent pole matching.** The present `Z` combination is a
   declared `xi=1` off-shell matching scheme and is not an on-shell observable.
2. **Gauge-parameter check.** A second `xi` value has not been evaluated.
3. **Physical species and thresholds.** The one-flavour massless step scheme
   is not the Standard Model spectrum.
4. **Counterterm trajectory.** No tuned bare anisotropy or multi-threshold RG
   trajectory has been calculated.
5. **Ternary current.** The Wilson current is not a derived current of the
   `(s,J)` history.
6. **Real-time unitarity.** The Euclidean regulator does not repair the
   many-to-one fundamental tick.
7. **SME comparison.** No gauge-independent coefficient has yet been mapped to
   an experimental preferred-frame bound.

---

## 7. Status table

| Claim | Status |
|---|---|
| one-loop integrands from the FTD-0418 action | `[DERIVED]` |
| reduced Clifford trace and seagull signs | `[THEOREM — finite algebra]` |
| finite-grid longitudinal Ward cancellation | `[NUMERICAL FACT — machine precision]` |
| QED_L-like step coefficient `-0.32696906(5) g^2` | `[NUMERICAL FACT — scheme-specific]` |
| exact zero without a counterterm in that scheme | `[CLOSED NEGATIVE]` |
| selected `g^2=alpha_FTD` translation | `[CONDITIONAL ARITHMETIC — imposed vertex wiring]` |
| scheme-independent on-shell relative speed | `[OPEN — HARD]` |
| physical Lorentz/SME adequacy | `[OPEN — HARD]` |

The result advances LR-3 from “integral undefined” to “one complete declared
scheme evaluated, automatic cancellation absent.” It does not promote the
local-link branch to a physical Lorentz-recovery result.
