# FTD-0432 — Native Dressed Evaporation-Hazard Pre-Registration v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION RUN]`  
**Date locked:** 2026-07-23  
**Identifier:** `FTD-0432`

## 1. Question and scope

FTD-0431 is outcome D because the native wave/coupling/evaporation source is
not a single exponential. The descriptive source history slows and oscillates
after its generated field raises the local energy entering evaporation. This
campaign asks the narrower mechanism question:

> Does the exact production pre-RNG evaporation hazard quantitatively predict
> the non-exponential one-step changes of the coarse polarity source?

This is an estimator-validation campaign. It cannot establish conservation,
an infrared slow mode, `U(1)`, a particle pole, or a common cone. Momentum and
volume scaling are explicitly deferred until this conditional-hazard observer
passes.

## 2. Exact counterfactual observer

At the start of each registered step, call the existing read-only
`prepare_delta_j()` and reproduce the standard single-substrate, unit-tick
wave write without committing it:

\[
  \widetilde v_i=v_i+\Delta J_i,
  \qquad \widetilde J_i=J_i+\widetilde v_i.
\]

With damping, Langevin, dual substrate, alternate integrators, latency, and
movement off, these are exactly the fields seen by the production evaporation
loop on the next tick. For every occupied unlocked site define

\[
 E_i=|\widetilde J_i|^2+|\widetilde v_i|^2
   +\sum_{j\in N_6(i)}(|\widetilde J_j|^2+|\widetilde v_j|^2),
\]

\[
 p_i=K_{\rm EVAP\_RATE}\,d\tau_i
     \exp[-E_i/K_{\rm MANIFEST}^2].
\]

No random number is drawn by the observer. For the normalized Fourier source
`S_k=N^-1 sum_i s_i exp(-ik.x_i)`, record

\[
 L_k=N^{-1}\sum_i s_i p_i e^{-ik.x_i},\qquad
 E[S_k(t+1)|X_t]=S_k(t)-L_k,
\]

the projected conditional hazard

\[
 h_k=\Re[L_k S_k^*/|S_k|^2],
\]

the expected removal count `sum p_i`, and the Bernoulli variance proxy
`sum p_i(1-p_i)`. Locked sites contribute neither hazard nor variance.

The observer may allocate scratch arrays and write the existing `delta_j`
diagnostic buffer. It may not write voxel, lattice, RNG, toggle, backend, tick,
or integrator state. An observer-on/off trajectory comparison must give
identical state and RNG hashes.

## 3. Frozen arms and matrix

Use the same globally neutral dense square-wave source convention as FTD-0431
at `L=32`. The registered modes are

- low: `d=<100>, n=1`;
- middle: `d=<110>, n=2`;
- high: `d=<111>, n=3`.

For each mode run:

1. **isolated:** evaporation only, seeds `0,...,7`;
2. **coupled:** wave propagation, coupling, and evaporation, seeds `0,...,7`;
3. **locked control:** coupled toggles with every source site locked, seed `0`.

Each arm records 32 transitions, `t=0 -> 1` through `t=31 -> 32`.
Genesis, pair production, annihilation, weak transmutation, movement, both
Gauss mechanisms, damping, forces, Langevin noise, dual substrate, latency,
and alternate wave integrators are off. `dt=1` and the production constants
are unchanged.

Run the full matrix under Windows/MSVC CPU and WSL2 CUDA/GCC. CPU history must
identify every actual removal as evaporation; CUDA uses occupancy loss as the
accepted-event count. Both backends must report their requested actual kind.

## 4. Structural validity gates

Every admitted record must satisfy all of:

- exact registered arms, seeds, modes, and transition ticks;
- initially neutral full occupancy and `|S_k(0)|>=0.3`;
- monotone occupancy;
- CPU journal evaporation count equals actual removals with zero other events;
- isolated observer probabilities and `h_k` equal `0.1` within `10^-12`;
- isolated predicted fields have zero local energy within `10^-14`;
- locked expected and actual removals are zero and the source is unchanged
  within `10^-14`;
- every coupled mode has nonzero predicted local energy by transition 1 and a
  nonconstant hazard range of at least `0.02` over the 32 transitions;
- the observer-on/off CPU trajectory has identical voxel-state, field, tick,
  and RNG hashes for all 32 ticks.

Failure of any item is outcome D.

## 5. Ensemble conditional-expectation test

For each `(arm,mode,t)` with eight seeds, average the complex actual next
source and complex predicted next source across seeds before differencing.
For seed `r`, let `V_r=sum_i p_i(1-p_i)`. Define conservative ensemble scales

\[
 \sigma_S={\sqrt{\sum_r V_r}\over 8N},
 \qquad
 \sigma_N={\sqrt{\sum_r V_r}\over 8}.
\]

and standardized residuals

\[
 z_S={|\overline S_{t+1}-\overline{(S_t-L_t)}|
       \over\max(10^{-15},\sigma_S)},
 \qquad
 z_N={|\overline{\Delta N}-\overline{\sum_i p_i}|
       \over\max(10^{-15},\sigma_N)}.
\]

These use the Bernoulli variance as a declared diagnostic scale; they do not
claim independent lattice sites. The locked controls are excluded from this
standardization because their variance is exactly zero.

Outcome A requires, separately for source and occupancy residuals:

- maximum standardized residual `<= 6`;
- RMS standardized residual `<= 2.5`;
- Windows CPU and WSL2 CUDA ensemble expected/actual source values agree
  within `10^-10` absolute at every registered point.

The loose standardized gates absorb correlation and the fixed eight-seed
sample while still rejecting an incorrectly timed or incorrectly normalized
hazard observer.

## 6. Feedback discriminator and outcomes

For each coupled mode form the ensemble-mean `h_k(t)` from ensemble-mean
`L_k(t)` and `S_k(t)`. Outcome A additionally requires:

- `max_t h_k(t)-min_t h_k(t) >= 0.02`;
- `min_t h_k(t) <= 0.05`;
- the mean expected site probability is below `0.05` on at least one
  transition;
- all observer probabilities lie in `[0,0.1]`.

Locked outcomes are:

- **A — DRESSED HAZARD EXPLAINS NON-EXPONENTIALITY:** all structural,
  conditional-expectation, backend, and feedback gates pass. This validates
  the production hazard as the cause of FTD-0431 curvature but makes no
  conservation or infrared claim.
- **B — HAZARD VALID BUT FEEDBACK INSUFFICIENT:** the observer predicts actual
  changes, but one or more feedback-variation/suppression gates fail.
- **C — HAZARD MISMATCH:** structural execution passes, but the registered
  conditional-expectation residual gates fail. The timing, normalization, or
  assumed Bernoulli model is not an adequate account.
- **D — INVALID EXECUTION:** a structural, source-lock, observer-neutrality,
  output, or backend gate fails. No mechanism inference is recorded.

No outcome licenses retuning the production reaction rule. If outcome A
passes, the next separately locked campaign may test late-time and infrared
scaling of `h_k`; it may not reuse a finite-time plateau as evidence of exact
charge conservation.
