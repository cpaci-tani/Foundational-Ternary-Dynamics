# PRE-REGISTRATION — Native reaction-aware polarity slow mode v1

**Date locked:** 2026-07-23  
**Identifier:** `FTD-0431`  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`

## 1. Question and logical scope

FTD-0429/0430 establish a finite, retarded, transported polarity/flux response
only when production reactions are disabled. FTD-0431 asks whether that same
coarse source has a reaction-bearing mode whose decay rate vanishes as
`k -> 0`.

The first discriminator activates native evaporation because it is the lowest-
energy production reaction and directly acts on the undressed manifested
polarity used by FTD-0429/0430. A single allowed reaction with a nonzero
infrared decay intercept is sufficient to close conservation of this specific
coarse charge candidate across the full event set. It is not sufficient to
exclude every possible nonlocal, topological, or differently dressed charge
definition.

No constant formula, empirical target, Gauss projector, matched-field state,
force, counterterm, or modified reaction rule is admitted.

## 2. Frozen source and engine arms

Each primary arm starts from zero flux and zero wave velocity with a dense,
globally neutral ternary square mode

\[
s(x)=\begin{cases}
+1,&n\,d\!\cdot x\pmod L<L/2,\\
-1,&\text{otherwise},
\end{cases}
\]

for directions `d=(1,0,0),(1,1,0),(1,1,1)` and harmonics `n=1,2,3`.
All sites are unlocked. Eight deterministic seeds `0,...,7` are used.

Three arms are frozen:

1. **isolated evaporation:** only `evaporation=true`;
2. **coupled evaporation:** `wave_propagation=true`, `coupling=true`, and
   `evaporation=true`;
3. **locked control:** the coupled arm with every manifested voxel locked,
   seed `0` only.

In all arms `genesis=false`. Pair production, weak transmutation, movement,
annihilation, damping, both Gauss mechanisms, forces, Langevin noise, dual
substrate, alternate wave integrators, and Lorentz prototypes are OFF.

The isolated arm has an exact ensemble prediction. With zero local field
energy, every occupied site survives a tick with

\[
r_{\rm evap}=1-K_{\rm EVAP\_RATE}=0.9,
\qquad
\gamma_{\rm evap}=-\log r_{\rm evap}=0.105360515657826\ldots,
\]

independent of momentum. The coupled arm tests whether native field dressing
removes that finite source-decay intercept.

## 3. Locked observables

At settled ticks `t=0,...,16`, a read-only observer records

\[
S_k=L^{-3}\sum_xs(x)e^{-ik\cdot x},
\qquad
D_k=i\sum_a\sin(k_a)J_{a,k},
\]

along with occupancy, global signed state, and the number of sites removed
since the previous sample. CPU history instrumentation must contain only
`Evaporation` events and its count must equal the occupancy loss. CUDA has no
history journal; occupancy loss is the event count of record there because no
other state-changing term is enabled.

Every row is invalid unless:

- initial global signed state is exactly zero;
- `|S_k(0)| >= 0.3`;
- occupancy is monotone non-increasing;
- every unlocked arm loses at least one site by tick 1 and by tick 16;
- the locked control loses no sites and preserves its source mode below
  `10^-14` relative;
- the isolated arm has `|D_k(t)| <= 10^-14` at every tick;
- the coupled arm has a nonzero divergence response above `10^-8` by tick 2.

## 4. Source-decay estimator

For each `(L,d,n,seed,arm)`, define the phase-referenced amplitude

\[
A(t)=\Re\!\left[S_k(t)S_k(0)^*/|S_k(0)|^2\right].
\]

The primary early-time decay is the ordinary least-squares slope

\[
\log A(t)=c-\gamma_k t,
\qquad t=0,1,\ldots,6.
\]

The fit is invalid if any `A(t)<=0`. For each momentum, the campaign-level
value is obtained from the complex ensemble-mean `S_k(t)` before taking the
log; averaging per-seed magnitudes is prohibited. The normalized RMS residual
of each ensemble-mean time fit must be at most `0.02`.

The isolated calibration must satisfy

\[
|\gamma_{\rm isolated}-\gamma_{\rm evap}|/\gamma_{\rm evap}\le0.02
\]

for every `L=32` direction/harmonic point on both backends.

## 5. Coupled infrared decision fit

Use only WSL2-CUDA coupled-arm ensemble means from `L=32,64`. Define

\[
q^2=\sum_a k_a^2,
\qquad h_4=\frac{\sum_a k_a^4}{q^2}.
\]

Compare

\[
M_0:\gamma=\gamma_0+Aq^2+Bh_4+C/L^3,
\qquad
M_{\rm cons}:\gamma=Aq^2+Bh_4+C/L^3.
\]

The eight seed labels are shared across all momenta and volumes. Uncertainty in
`gamma_0` is the delete-one-seed jackknife standard error computed by repeating
the entire ensemble-mean time fit and infrared regression eight times. The
locked lower bound is `gamma_0-1.96 sigma_J`.

Outcome A requires all of:

- `BIC_cons-BIC_0 >= 10`;
- infrared-fit RMS `<= 0.01`;
- `gamma_0 > 0.02`;
- jackknife 95% lower bound `> 0.01`;
- Windows/MSVC CPU and WSL2-CUDA `L=32` gamma values agree within `0.01`
  absolute for every mode.

Outcome B requires the conserved model not to lose by `Delta BIC>=10` and the
95% interval for `gamma_0` to contain zero. Intermediate results are outcome C,
not permission to alter the fit.

## 6. Exact field-memory discriminator

For the coupled ensemble mean, the unchanged production wave/coupling map
must still satisfy

\[
D_{t+1}=(2-C_{\rm WAVE}^2M_{18}(k))D_t-D_{t-1}
       +G_C\sum_a\sin^2(k_a)S_t.
\]

The maximum residual divided by
`max(1,|D_{t+1}|,|D_t|,|D_{t-1}|,|F_kS_t|)` must be at most `10^-10` on CPU
and `10^-8` on CUDA.

The two homogeneous field roots remain unit-modulus source-free wave modes.
They do not count as a conserved charge mode because their `S_k` component is
zero. The reaction-bearing eigenvalue is the measured source survival factor
`exp(-gamma_k)`.

## 7. Execution matrix

- `L=32`, profile `full`: isolated and coupled arms for eight seeds plus the
  locked coupled control for seed `0`; Windows/MSVC CPU and WSL2 CUDA/GCC.
- `L=64`, profile `infrared`: coupled arms for eight seeds only; WSL2 CUDA/GCC.

Each arm contains all nine direction/harmonic modes and ticks `0,...,16`.
Files with missing/duplicate/additional rows or an unregistered backend,
volume, profile, seed, mode, arm, or tick are invalid.

## 8. Locked outcomes

- **A — FINITE REACTION DECAY:** the source-bearing mode has a positive,
  volume-stable infrared decay intercept. Conservation of the
  FTD-0429/0430 coarse polarity candidate is closed across native evaporation.
- **B — REACTION-AWARE SLOW MODE:** the decay intercept is statistically
  compatible with zero and the conserved model survives. A broader
  reaction-complete campaign is then licensed.
- **C — NONUNIVERSAL/UNRESOLVED:** activation and validity pass, but neither
  A nor B meets its locked statistical gates.
- **D — INVALID EXECUTION:** source, event, mode, recurrence, backend, output,
  or source-lock validity fails. No physical conclusion is recorded.

Even outcome A does not retract the reaction-free susceptibility or retarded
transport results. It states that their source is not conserved when the
native low-energy evaporation rule is admitted.
