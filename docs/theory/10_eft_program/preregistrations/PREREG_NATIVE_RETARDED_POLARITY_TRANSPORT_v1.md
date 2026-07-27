# PRE-REGISTRATION — Native retarded polarity transport v1

**Date locked:** 2026-07-23  
**Identifier:** `FTD-0430`  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`

## 1. Question and frozen claim boundary

FTD-0429 found a finite long-wavelength response in the frozen, reaction-free
native `wave_propagation + coupling` sector,

\[
 Z(k)=\frac{(\nabla\!\cdot J)_k}{s_k}
 \longrightarrow 3G_C .
\]

FTD-0430 asks whether the same response follows an actual source displacement
executed by the production `phase_movement`, with retarded support and the
unmodified native wave pole. This is the first moving-source test of the
coarse-scale charge interpretation. It is not a test of microscopic `U(1)`,
gauge redundancy, electromagnetism, a photon, a force law, or the empirical
speed of light.

The ontology, production tick, constants, phase order, 18-point stencil, and
all engine operators are frozen. The observer is read-only.

## 2. Engine sector and source history

The only enabled production terms are:

- `wave_propagation = true`;
- `coupling = true`;
- `movement = true`;
- `dual_substrate = false`;
- `strict_validation = true`.

Both `gauss_projection` and `matched_gauss_dynamics` are OFF. Damping,
genesis, evaporation, pair production, weak transmutation, forces, Coulomb,
emergent forces, Langevin noise, alternate integrators, and every Lorentz
prototype are OFF.

Each arm contains a sparse neutral pair at

\[
 A=(L/4,L/2,L/2),\qquad B=(5L/8,L/2,L/2).
\]

For orientation `+1`, `s(A)=+1` and `s(B)=-1`; orientation `-1` globally
reverses both signs. Flux and wave velocity start identically zero.

Two bridges evolve in parallel conceptually but are ticked sequentially:

1. **moving:** both particles have `v=(0.99 C_SPEED,0,0)` and
   `remainder=(1-v_x,0,0)`, causing exactly one production hop by `+x` at the
   end of tick 1;
2. **stationary:** the identical pair is locked at its original sites.

Immediately after tick 1 the moved particles are checked at `A+xhat` and
`B+xhat`, then locked with zero velocity and zero remainder. No further state
change is permitted. The difference between the moving and stationary arms
therefore isolates the response to a one-cell displacement without injecting
a field by hand.

CPU execution must record exactly two `Movement` journal events on tick 1 and
zero reaction events. The GPU path has no event journal and must reproduce the
exact before/after state positions instead.

## 3. Locked Fourier estimator

Nine modes are projected in one read-only volume pass:

- directions `(1,0,0)`, `(1,1,0)`, `(1,1,1)`;
- harmonics `n=1,2,3`;
- `k_a=2 pi n d_a/L`.

For each bridge,

\[
 s_k=L^{-3}\sum_x s(x)e^{-ik\cdot x},\qquad
 D_k=i\sum_a\sin(k_a)J_{a,k}.
\]

The displacement source and response are

\[
 \Delta s_k=s_{k,\mathrm{moving}}-s_{k,\mathrm{stationary}},\qquad
 \Delta D_k=D_{k,\mathrm{moving}}-D_{k,\mathrm{stationary}},
\]

and the measured ratio is `R_k(tau)=Delta D_k/Delta s_k`, with
`tau=tick-1`. All included modes must satisfy `|Delta s_k| >= 10^-8`.

The exact native symbols remain those locked in FTD-0429:

\[
 M(k)=4-\frac23(c_x+c_y+c_z)
       -\frac23(c_xc_y+c_xc_z+c_yc_z),
\]

\[
 \omega_k=\arccos\!\left(1-\frac{C_{\rm WAVE}^2M(k)}2\right),
 \qquad
 Z_{\rm exact}(k)=\frac{G_C}{C_{\rm WAVE}^2}
                  \frac{\sum_a\sin^2k_a}{M(k)}.
\]

The response is sampled at `tau=0`, `tau=1`, and at sixteen unique integer
ticks spaced as uniformly as rounding permits over two periods of the slowest
measured mode. Duplicate integer ticks are stored only once. Real and imaginary
parts are fit jointly to

\[
 R_k(\tau)=Z_k+B_k\cos(\omega_k\tau)+C_k\sin(\omega_k\tau).
\]

The fit is invalid unless:

- normalized complex residual is at most `10^-7`;
- `|Im Z_k| <= 10^-7 max(1,|Re Z_k|)`;
- `|Re Z_k-Z_exact(k)|/|Z_exact(k)| <= 10^-6`.

For the zero-initial-field step recurrence, the oscillatory amplitude obeys

\[
 \frac{\sqrt{|B_k|^2+|C_k|^2}}{|Z_k|}
 =\frac{1}{\cos(\omega_k/2)}.
\]

The relative error in this residue relation must be at most `10^-5`. Failure
of the residue gate means the static coefficient has not been shown to ride
the native on-shell pole even if the fitted intercept is nonzero.

## 4. Locked causal-support estimator

At every sampled tick, the observer also evaluates the sitewise central
divergence difference. Distance is the periodic Chebyshev distance to the four
sites whose ternary state changed: `A`, `A+xhat`, `B`, and `B+xhat`.

Because movement occurs after field evolution on tick 1, `tau=0` must satisfy

\[
 \|\Delta(\nabla\!\cdot J)\|_\infty\le10^{-11}.
\]

For `tau >= 1`, the source gradient has range one and the measured divergence
adds one cell. Each subsequent 18-point wave step expands support by at most
one Moore shell, so every value above `10^-13` must lie within

\[
 r_\infty\le\tau+1.
\]

The maximum magnitude outside that cone must be at most `10^-11`. The response
must also be nontrivial: the divergence difference at `tau=1` must exceed
`10^-10`, and the observed support radius must exceed two by the last sample.
This is a lattice dependency-cone test, not an identification with a measured
light cone.

## 5. Execution matrix and reproduction

- `L=32`, profile `full`: orientations `+1` and `-1`, Windows/MSVC CPU and
  WSL2 CUDA/GCC.
- `L=64`, profile `infrared`: orientation `+1`, WSL2 CUDA/GCC.

The Windows and WSL2 `L=32` results must agree mode by mode within `10^-5`
relative error. The two orientations must agree in their normalized response
within `10^-5`. Any missing or additional arm invalidates the record.

## 6. Locked infrared decision model

The primary positive-orientation WSL2 rows from `L=32,64` are fit to the same
predeclared pair used by FTD-0429, with
`q2=sum_a k_a^2` and `h4=sum_a k_a^4`:

\[
 M_0: Z=Z_0+Aq^2+Bh_4+Cq^4,
 \qquad
 M_\emptyset: Z=Aq^2+Bh_4+Cq^4.
\]

Advancement requires all of:

- `Delta BIC = BIC_empty-BIC_0 >= 10`;
- RMS residual of `M_0 <= 10^-4`;
- `|Z_0-3G_C|/(3G_C) <= 0.01`;
- `|Z_0-Z_0^(FTD-0429)|/Z_0^(FTD-0429) <= 0.002`.

No tolerance may be altered after inspecting campaign output. A failed or
ill-conditioned fit is a failed gate, not permission to change the model.

## 7. Locked outcomes

- **A — RETARDED NATIVE COARSE POLARITY RESPONSE:** all validity, movement,
  causal, pole, residue, backend, mirror, and infrared gates pass. This permits
  the restricted statement that native movement transports the source of the
  same coarse long-wavelength field response seen in FTD-0429.
- **B — STATIC-ONLY RESPONSE:** a finite intercept survives, but the moving
  source fails the causal, pole, residue, or FTD-0429 equality gate. The static
  susceptibility may not be promoted to transported emergent charge.
- **C — ZERO OR NONUNIVERSAL RESPONSE:** the intercept vanishes or differs
  across volume/direction/backend/orientation outside the locked tolerances.
  The FTD-0429 interpretation does not survive source transport.
- **D — INVALID EXECUTION:** source history, toggles, backend, mode content,
  source amplitude, source lock, or output integrity fails. No physical
  conclusion is recorded.

Even outcome A does not establish exact microscopic charge conservation,
reaction-sector conservation, gauge invariance, Lorentz invariance, photons,
Maxwell theory, or empirical signal speed.
