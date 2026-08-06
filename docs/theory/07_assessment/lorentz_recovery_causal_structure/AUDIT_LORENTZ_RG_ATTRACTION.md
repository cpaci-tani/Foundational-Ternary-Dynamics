# FTD-0416 — Optimistic Lorentz RG-Attraction Bound

**Date:** 2026-07-22  
**Status:** `[SELECTED IR SURROGATE]` + `[EXTERNAL ONE-LOOP RESULT]` + `[DERIVED — linearized mixing and integrated bound]` + `[OPEN — FTD lattice threshold]`  
**Verdict:** `COMMON-CONE-IR-ATTRACTIVE; ATTRACTION-ALONE-QUANTITATIVELY-INSUFFICIENT`  
**Verifier:** `scripts/proofs/proof_lorentz_rg_attraction.py`

---

## 0. Outcome

The best available perturbative mechanism is real but too weak to be the
whole Lorentz-recovery story. In the standard anisotropic-QED surrogate, the
one-loop velocity matrix has a common-speed zero mode and an IR-attractive
relative-speed mode. The attraction shuts off as the QED coupling decreases,
however, leaving only a power of the ratio of couplings rather than an
exponential erasure of UV Lorentz violation.

For one gauge field and `N_f` massless charged Dirac species,

$$
\frac{\delta_{\rm IR}}{\delta_{\rm UV}}
=\left(\frac{\alpha_{\rm IR}}{\alpha_{\rm UV}}\right)^{(N_f+2)/N_f}
$$

at one loop and to first order in the relative cone mismatch `delta`.
Taking the most optimistic perturbative values
`alpha_IR=1/137`, `alpha_UV<=1`, and integer `N_f>=1`, the strongest possible
suppression is the one-species value

$$
\frac{1}{137^3}=3.889\times10^{-7}.
$$

Thus an IR tolerance `epsilon` still requires the threshold mismatch to obey

$$
|\delta_{\rm UV}| < 137^3\epsilon.
$$

For illustration, `epsilon=10^-15` requires
`|delta_UV|<2.571353e-9`; `epsilon=10^-21` requires
`|delta_UV|<2.571353e-15`. These are tolerance translations, not adopted
experimental limits.

This is an optimistic external continuum result. It does not calculate the FTD
Brillouin-zone threshold and does not establish that FTD lies in this
surrogate's basin of attraction.

---

## 1. Frozen bridge surrogate

To prevent scheme drift, this audit freezes the narrowest interacting IR
surrogate that contains the FTD-0413 common-cone variables:

$$
\mathcal L_{\rm IR}=
\frac{Z_E}{2}\mathbf E^2-\frac{Z_B}{2}\mathbf B^2
+\sum_{a=1}^{N_f}\bar\psi_a
\left(Z_t\gamma^0D_0+Z_s\gamma^iD_i\right)\psi_a.
$$

Define

$$
c_A^2=Z_B/Z_E,\qquad v_\psi=Z_s/Z_t,\qquad
\delta=v_\psi/c_A-1.
$$

The interaction is the standard `U(1)` minimal vertex. Its coupling and matter
functional form retain the Branch-B statuses recorded in
`SPEC_WILSON_DIRAC_FTD.md`: imported Wilson/Dirac matter, imposed vertex
coupling, and selected connection map.

At tree level the selected FTD free symbols that motivate this surrogate are

$$
T_B(\theta)=\frac23(1-\cos^3\theta),\qquad
M_{18}(\mathbf q)=4-\frac23\sum_i\cos q_i
-\frac23\sum_{i<j}\cos q_i\cos q_j,
$$

with the selected pole `T_B=(1/7)M18`. The time kernel has the exact finite-
range decomposition

$$
T_B=\frac12(1-\cos\theta)+\frac16(1-\cos3\theta).
$$

That observation does not complete the interacting action. The declared map
`A_mu = P_T J_mu` uses a transverse projector containing the inverse spatial
lattice Laplacian. Its Fourier multiplier is direction-dependent at zero
momentum and is therefore spatially nonlocal. The bridge surrogate is a
gauge-fixed EFT construction, not a local derivation from the five FTD
postulates. A native action must either introduce independent link variables
with their own local dynamics or replace this projection by a proved local
mechanism.

---

## 2. External one-loop input

Roy, Juricic, and Herbut calculate the one-loop velocity flow for anisotropic
fermions coupled to a fluctuating `U(1)` field in three spatial dimensions:
[JHEP 04, 018 (2016)](https://arxiv.org/abs/1510.07650). Restricting their
equations to `N_f` charged Dirac species with no scalar/Yukawa sector and
linearizing at `v_psi=c_A=c` gives

$$
\begin{pmatrix}\beta_{v_\psi}\\ \beta_{c_A}\end{pmatrix}
=\alpha
\begin{pmatrix}
-8/3 & 8/3\\
4N_f/3 & -4N_f/3
\end{pmatrix}
\begin{pmatrix}\delta v_\psi\\ \delta c_A\end{pmatrix}.
$$

This is the complete marginal velocity-mixing matrix only for the frozen
continuum surrogate at linear order. It is not the complete FTD coefficient
matrix requested by FTD-0415.

The eigenvalues are

$$
\lambda_{\rm common}=0,\qquad
\lambda_{\rm rel}=-\frac43(N_f+2)\alpha.
$$

The zero eigenvector changes the common unit of speed. The negative relative
eigenvalue makes the equal-cone line IR attractive in the cited RG convention.

The same calculation gives

$$
\beta_\alpha=-\frac43N_f\alpha^2.
$$

Eliminating RG time yields

$$
\frac{d\ln|\delta|}{d\ln\alpha}=\frac{N_f+2}{N_f},
$$

which integrates to the bound in §0.

The cited paper absorbs a constant `1/(8pi^2)` into its displayed gauge
coupling before writing these beta functions. That constant cancels from the
endpoint ratio and does not change the exponent. The numerical `1/137^3`
statement uses standard physical fine-structure normalization for both
endpoints and the declared perturbative ceiling `alpha_UV<=1`; it is not a
scheme-independent claim about where perturbation theory must fail.

---

## 3. Why this does not close FTD-0415

The continuum logarithm knows only the marginal IR operators. The FTD danger
is a finite threshold correction generated while integrating the selected
q6/q8 Lorentz-breaking lattice operators over the full Brillouin zone. That
finite matching contribution is precisely where the Collins-type
radiative-percolation problem lives.

FTD-0413 sets the two selected bare free cones equal through q4, but equality
of the leading bare slopes does not force the threshold correction to vanish.
If full-lattice matching generates `delta_match`, the optimistic continuum
flow supplies only

$$
|\delta_{\rm IR}|=S_{\rm RG}|\delta_{\rm match}|,
\qquad
S_{\rm RG}\ge 1/137^3
$$

under the perturbative assumptions in §0. A generic loop factor is not enough:
for a `10^-15` tolerance the unknown threshold must already be below
`2.6e-9` even under the strongest one-species attraction.

The species count makes the exponent `(N_f+2)/N_f=1+2/N_f` smaller, not
larger. Additional charged species therefore weaken this particular
suppression factor for fixed endpoint couplings. Their effect on thresholds
must be calculated rather than presumed beneficial.

---

## 4. Exact next calculation

FTD-0417 freezes a deliberately minimal noncompact unit-plaquette connection
at selected `c_A²=1/7`. FTD-0418 then freezes its matching one-tick axial
Wilson regulator, derives the exact one- and two-photon vertices, and proves
both lattice Ward identities. FTD-0419 evaluates one complete `xi=1`
QED_L-like step scheme and obtains `delta_match/g²=-0.32696906(5)`. Under the
selected `g²=alpha` wiring, even this section's strongest `1/137³` attraction
leaves `9.28e-10`, about `9.28e5` above a declared `1e-15` tolerance.

The remaining LR-3 deliverable is therefore no longer a bare integration. It
is to freeze the required dimension-four anisotropy counterterm, obtain a
gauge-independent on-shell pole match, and run that tuned condition across
every charged-species threshold. The axial pair is exactly local and stable
but not radiatively self-matched.

Omitting the two-photon seagull terms or evaluating only the universal
logarithm is not a complete lattice calculation.

---

## 5. Status table

| Claim | Status |
|---|---|
| Frozen anisotropic-QED bridge action | `[SELECTED IR SURROGATE]` |
| Published one-loop beta functions | `[EXTERNAL ONE-LOOP RESULT]` |
| Linearized 2x2 velocity matrix and its eigenvalues | `[DERIVED — from external equations]` |
| Integrated suppression law | `[DERIVED — from external equations]` |
| `A=P_T J` is not a finite-support local map | `[THEOREM — Fourier-symbol obstruction]` |
| FTD-0419 step-scheme threshold `delta_match/g²` | `-0.32696906(5)` `[NUMERICAL FACT — scheme-specific]` |
| automatic one-loop cancellation in that scheme | `[CLOSED NEGATIVE]` |
| gauge-independent on-shell threshold | not calculated |
| Quantitative Lorentz recovery | `[OPEN — HARD]` |

The result supplies a viable direction—IR attraction—but removes the option
of treating “RG flow toward a common cone” as sufficient without a threshold
calculation. The attraction must be quantitatively combined with the lattice
matching coefficient.
