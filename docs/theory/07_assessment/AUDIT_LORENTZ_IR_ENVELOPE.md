# FTD-0414 — Selected Free-Sector Lorentz IR Envelope

**Date:** 2026-07-22  
**Status:** `[DERIVED — selected free clocks and q6 truncation]` + `[IMPLEMENTED DIAGNOSTIC]` + `[CONDITIONAL PHENOMENOLOGY — calibrated a]` + `[OPEN — interactions and radiative stability]`  
**Verdict:** `IR-BOUND-DEFINED; FREE-TREE ADEQUACY CONDITIONAL ON CALIBRATION`  
**Verifier:** `scripts/proofs/proof_lorentz_ir_envelope.py`  
**Native gate:** `lorentz_ir_envelope`

---

## 0. Outcome

Exact all-orders Lorentz symmetry is not imposed as the acceptance condition
for the discrete framework. The weaker condition is operational:

> over the experimentally accessed dimensionless momentum interval `q=ka`,
> every observable sector must share one cone to within the applicable bound.

For the selected free FTD-0411 live flux clock and FTD-0413 improved Wilson
matter clock, the first tree-level speed spread is

$$
\boxed{
\frac{\Delta v_{\max}}{c_s}
=\frac{11}{540}(ka)^4+O((ka)^6)}.
$$

The largest same-direction matter/flux gap is

$$
\boxed{
\max_{\hat n}\frac{|v_m-v_f|}{c_s}
=\frac{65}{3969}(ka)^4+O((ka)^6)}.
$$

Therefore a requested fractional speed tolerance `epsilon` gives the
leading-order requirement

$$
\boxed{ka<q_\epsilon
=\left(\frac{540\epsilon}{11}\right)^{1/4}}.
$$

This is a falsifier, not a whole-theory pass. This Lorentz construction does
not derive or measure the physical lattice spacing `a`. The framework does,
however, maintain two explicitly conditional calibrations: electron-primary
gives `a=ell_P` through the selected electron ladder, while legacy
Planck-primary imposes `a=ell_P`. Either makes a numerical free-tree estimate
possible, but neither supplies an interacting limiting-speed matrix or shows
that radiative corrections preserve the tree-level `q^4` suppression.

---

## 1. Sixth-order basis correction

The finite-momentum native test exposed a notation collision inherited from
FTD-0407/0411. One formula used `Q6` for a cubic discriminator, while adjacent
prose defined `Q6=sum(q_i^6)`. Taken literally, the old formula predicted the
axis coefficient `1/120`, contradicting the exact identity

$$
M_{18}(q,0,0)=2-2\cos q
=q^2-\frac{q^4}{12}+\frac{q^6}{360}+O(q^8).
$$

Use the unambiguous pure-power basis

$$
S_2=\sum_iq_i^2,\qquad Q_4=\sum_iq_i^4,\qquad Q_6=\sum_iq_i^6.
$$

The exact production SC+FCC symbol is then

$$
\boxed{
M_{18}=S_2-\frac{S_2^2}{12}
+\frac{S_2Q_4}{72}-\frac{Q_6}{90}+O(q^8)}.
$$

This correction changes none of the FTD-0407 through FTD-0413 quartic
results. It corrects their displayed sixth-order tensors. In particular, the
literal and live-surrogate BCC-time poles differ by the same isotropic
`-S2^3/2058` term as before.

---

## 2. Selected live poles

Let `n_i=q_i/q`, `A4=sum(n_i^4)`, and `A6=sum(n_i^6)`. After factoring the
selected `c_s^2=1/7`, write

$$
\frac{\omega_s^2}{c_s^2}
=q^2+B_s(\hat n)q^6+O(q^8).
$$

### 2.1 Improved Wilson matter with unit-step RK4

The semidiscrete FTD-0413 Hamiltonian contributes

$$
\frac{S_2^3}{36}+\frac{S_2Q_4}{36}-\frac{Q_6}{15}.
$$

The unit-step RK4 phase adds `-S2^3/2940` after `c_s^2` is factored. Hence

$$
\boxed{B_m=\frac{121}{4410}+\frac{A_4}{36}-\frac{A_6}{15}}.
$$

### 2.2 Live BCC-time flux surrogate

Substituting the corrected `M18` into the exact period-two phase series gives

$$
\boxed{B_f=-\frac{121}{17640}+\frac{A_4}{72}-\frac{A_6}{90}}.
$$

These formulas describe selected free clocks. They do not describe manifested
ternary matter, gauge modes, gravity, or interacting bound states.

---

## 3. Exact directional extrema

Set `x_i=n_i^2`. The direction sphere becomes the compact simplex
`x_i>=0`, `sum x_i=1`, with `A4=sum x_i^2` and `A6=sum x_i^3`. The extrema of
the symmetric cubic polynomials occur among their exact interior critical
points and edge critical points.

For matter,

| direction | `B_m` |
|---|---:|
| axis `(1,0,0)` | `-101/8820` |
| face `(1,1,0)/sqrt(2)` | `29/1176` |
| body `(1,1,1)/sqrt(3)` | `155/5292` |

The axis is the minimum and the body diagonal is the maximum. Since
`v/c_s=1+Bq^4/2+O(q^6)`, their difference is

$$
\frac12\left(\frac{155}{5292}+\frac{101}{8820}\right)
=\frac{11}{540}.
$$

The flux coefficients lie inside that interval. The same-direction gap is

$$
B_m-B_f=\frac{121}{3528}+\frac{A_4}{72}-\frac{A_6}{18}.
$$

Its largest absolute value occurs on the body diagonal and equals `130/3969`
in squared phase, hence `65/3969` in phase speed.

---

## 4. Why this is not production matter integration

The live RenderBridge matter field is ternary manifested occupancy with a
kinematic velocity. The FTD-0413 object is a four-component complex Wilson
spinor. No canonical map between those state spaces exists. Inserting the
spinor Hamiltonian into `phase_movement` would add a new ontology and would
not be a harmless runtime toggle.

FTD-0414 therefore adds an analytic/native diagnostic only. It leaves the
Wilson coefficient default off and makes no RenderBridge production change.

---

## 5. Empirical adequacy gate

The leading inverse envelope is implemented as
`lorentz_ir_q_limit(epsilon)`. A physical comparison must separately provide:

1. the highest relevant physical wavenumber `k_max`;
2. the applicable fractional speed/dispersion tolerance `epsilon`;
3. a physical or bounded lattice spacing `a`, with its calibration status;
4. an exact finite-`q` remainder check beyond the asymptotic term;
5. interacting, polarization, gauge, gravity, and radiative corrections.

The free tree-level proposal is compatible with a bound only if

$$
k_{\max}a<q_\epsilon.
$$

Under either existing `a=ell_P` calibration, the leading low-momentum map is
`q=E/E_P`. The selected all-sector free-tree envelope then becomes

$$
\left.\frac{\Delta v_{\max}}{c_s}\right|_{a=\ell_P}
=\frac{11}{540}\left(\frac{E}{E_P}\right)^4
+O\!\left((E/E_P)^6\right).
$$

For orientation, this leading term is `3.1e-62` at `13.6 TeV`, `9.2e-55`
at `1 PeV`, and `9.2e-35` at `10^20 eV`. These numbers are conditional
arithmetic, not independent predictions: physical photons have not been
identified with the selected flux surrogate, manifested matter has not been
identified with the Wilson spinor, and the finite-q/interacting corrections
are unbounded. They do show that, *if* the Planck-scale calibration and the
selected free clocks are accepted, direct tree-level q4 dispersion is not the
dominant Lorentz risk. The dominant risk is radiative generation of allowed
lower-dimensional preferred-frame operators.

Without adopting a calibration, “violations are Planck suppressed” is an
assumption rather than a dimensionless result. Even after adopting one,
tree-level `q^4` suppression does not answer the
Collins–Perez–Sudarsky–Urrutia percolation objection without a radiative
mixing calculation.

---

## 6. Status table

| Claim | Status |
|---|---|
| Correct pure-power sixth-order `M18` tensor | `[THEOREM — exact expansion]` |
| Selected free matter/flux speed spread `11(ka)^4/540` | `[DERIVED — selected clocks, leading order]` |
| Same-direction free common-cone gap `65(ka)^4/3969` | `[DERIVED — selected clocks, leading order]` |
| C++ diagnostic and exact finite-q native gate | `[IMPLEMENTED]` |
| Exact all-orders Lorentz symmetry | not required by this gate and not established |
| Planck-calibrated free-tree magnitude | `[CONDITIONAL PHENOMENOLOGY]` — tiny direct q4 term, conditional on selected clocks/carriers |
| Physical Lorentz compatibility | `[OPEN]` — requires finite-q data, carrier identification, interactions, and experimental inputs |
| Interacting/radiatively stable recovery | `[OPEN — HARD]` |

The practical Lorentz target is now empirical infrared adequacy, not exact
continuum symmetry. Failure of the inequality at any observed scale falsifies
the selected cone implementation.
