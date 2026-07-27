# FTD-0433 — Native Dressed-Hazard Infrared Scaling Pre-Registration v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION RUN]`  
**Date locked:** 2026-07-23  
**Identifier:** `FTD-0433`

## 1. Question and licensed scope

FTD-0432 validates the exact production pre-RNG evaporation hazard as the
mechanism behind FTD-0431's non-exponential polarity history. It does not show
whether the dressed hazard decreases toward the infrared because its three
representative modes changed direction and harmonic together.

FTD-0433 asks:

> For one fixed axial fundamental source family, does the dressed production
> hazard at the first native field antinode decrease systematically as
> momentum is lowered?

This is a finite-volume scaling discriminator. Even outcome A is evidence for
infrared suppression at a pole-defined phase, not proof of a zero asymptotic
decay rate, exact charge conservation, `U(1)`, a hydrodynamic pole, or
long-time survival.

## 2. Frozen source family and native phase

At every volume initialize the same full-occupancy, globally neutral axial
square source

\[
 s(x,y,z)=\begin{cases}+1,&0\le x<L/2,\\-1,&L/2\le x<L,
 \end{cases}
\]

which is the fixed Fourier family `d=<100>, n=1`, with

\[
 k_L=2\pi/L.
\]

Use the exact production 18-point symbol and pole

\[
 M_{18}(k)=4-\frac23(c_x+c_y+c_z)
 -\frac23(c_xc_y+c_xc_z+c_yc_z),
\]

\[
 \omega_L=\arccos\!\left(1-\frac12 C_{\rm WAVE}^2M_{18}(k_L,0,0)\right).
\]

The FTD-0432 observer at transition `t` predicts the field after wave write
`m=t+1`. Freeze the first-antinode transition as

\[
 t_L^*=\operatorname{round}(\pi/\omega_L)-1.
\]

Record its phase error `|(t_L^*+1)omega_L-pi|`; it must be at most
`omega_L/2 + 10^-14`. No measured hazard is used to select a tick.

## 3. Execution matrix

Primary WSL2 CUDA/GCC volumes are

\[
 L\in\{12,16,20,24,32,40,48\}.
\]

At every volume run the coupled arm with seeds `0,...,7`, recording every
transition `t=0,...,t_L^*`. Run an independent Windows/MSVC CPU reproduction at
`L=32` with the same eight seeds and transitions.

The only active production terms are `wave_propagation`, `coupling`, and
`evaporation`. Genesis, pair production, annihilation, weak transmutation,
movement, both Gauss mechanisms, damping, forces, Langevin noise, dual
substrate, latency, and alternate wave integrators are off. `dt=1` and all
production constants remain unchanged.

The exact FTD-0432 hazard observer is reused without modification. CPU history
must identify every accepted removal as evaporation; CUDA uses occupancy loss.

## 4. Structural and observer-validity gates

Every admitted file must satisfy all of:

- exact registered backend, volume, source, eight seeds, and transition set;
- full initial occupancy, zero global signed state, and `|S_k(0)|>=0.3`;
- monotone occupancy and positive source projection through `t_L^*+1`;
- exact registered `omega_L`, `t_L^*`, and phase-error bound;
- all site probabilities in `[0,0.1]`;
- finite nonnegative expected removals and Bernoulli variance;
- CPU history evaporation count equals occupancy loss, with zero other events;
- Windows CPU and WSL2 CUDA `L=32` source, expected loss, hazard, local-energy,
  and actual-removal fields agree within `10^-10` absolute.

For each WSL `(L,t)`, ensemble-average the eight seeds and repeat the FTD-0432
conditional-expectation standardized residuals

\[
 z_S={|\overline S_{t+1}-\overline{(S_t-L_t)}|
       \over\max(10^{-15},\sqrt{\sum_rV_r}/(8L^3))},
\]

\[
 z_N={|\overline{\Delta N}-\overline{\sum_i p_i}|
       \over\max(10^{-15},\sqrt{\sum_rV_r}/8}.
\]

Across the complete primary matrix, each family must have maximum `<=6` and
RMS `<=2.5`. Failure of a structural or observer-validity gate is outcome D.

## 5. Locked first-antinode estimators

For each volume, at `t_L^*`, form ensemble-mean source `S_L`, expected loss
`Q_L`, and projected hazard

\[
 h_L^*=\Re(Q_LS_L^*)/|S_L|^2.
\]

Also record the post-transition survival amplitude

\[
 A_L^*=\Re[S_L(t_L^*+1)S_L(0)^*]/|S_L(0)|^2.
\]

Uncertainties in `h_L^*` are delete-one-seed jackknife standard errors. The
largest-volume upper bound is `h_48^*+1.96 sigma_J`.

For adjacent volumes define the local effective exponent

\[
 p_{L_1,L_2}=\frac{\log(h_{L_2}^*/h_{L_1}^*)}
 {\log(k_{L_2}/k_{L_1})}.
\]

No polynomial intercept fit is authorized in v1. Seven finite volumes do not
justify converting one chosen functional form into an asymptotic theorem.

## 6. Locked outcomes

- **A — FIRST-ANTINODE IR SUPPRESSION:** all structural/observer gates pass;
  `h_L^*>0` and decreases strictly with increasing `L`; `h_48^*/h_12^*<=0.25`;
  the 95% upper bound on `h_48^*` is below `0.01`; both
  `p_32,40` and `p_40,48` exceed `0.25`; and `A_48^*>0.1`.
  This licenses finite-volume evidence for dressed-hazard suppression only.
- **B — FINITE HAZARD AT THE TESTED SCALE:** all validity gates pass and either
  the 95% lower bound on `h_48^*` exceeds `0.01`, or
  `h_48^*/h_12^*>=0.75` with both last effective exponents having absolute
  value below `0.25`. This is a finite tested-scale result, not an asymptotic
  no-go.
- **C — UNRESOLVED SCALING:** execution and observer validation pass, but
  neither A nor B meets every locked gate. Nonmonotonic or crossover behavior
  is reported rather than fitted away.
- **D — INVALID EXECUTION:** any structural, observer, backend, source-lock,
  or output gate fails. No scaling inference is recorded.

No outcome permits changing the production reaction rule. Outcome A would
license a separately preregistered larger-volume/late-time campaign; it would
not license calling polarity exactly conserved. Genesis, annihilation, and
weak transmutation remain separate future reaction classes.
