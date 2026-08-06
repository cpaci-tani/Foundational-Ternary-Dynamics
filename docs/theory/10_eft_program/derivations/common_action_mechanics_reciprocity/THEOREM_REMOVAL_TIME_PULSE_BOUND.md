# Theorem — Removal-Time Pulse Bound

**FTD ID:** FTD-0589  
**Status:** `[THEOREM — EXACT RECTANGULAR-PULSE CANCELLATION]` +
`[THEOREM — ARBITRARY ONE-TIME-REMOVAL BOUND]` +
`[NUMERICAL FACT — FOUR REGISTERED SPECTRAL SUMS]` +
`[MEASURED — 96 SANITIZED CONFORMANCE ARMS]` +
`[CLOSED NEGATIVE — ENDOGENOUS AUTOCATALYSIS FOR N <= 6]` +
`[BOUNDARY SUPERSEDED BY FTD-0590 — N=7 CLOSED]`  
**Date:** 2026-07-26  
**Verdict:**
`ARBITRARY_REMOVAL_N_LE_6_CLOSED_NEXT_COUNT_7_UNRESOLVED`

## 1. Scope

Use the frozen FTD-0586/0588 sector: one periodic finite-volume field
quotient, production 18-point wave propagation, native state-gradient
coupling, and genesis/evaporation. Initial flux, wave velocity, matter
velocity, and remainder vanish. Gauss projection, damping, forces, movement,
pair production, reactions, clocks, and baths are absent.

There are `N` distinct stationary ternary sources of arbitrary position and
polarity. Every source begins present at tick zero and may be removed once at
an arbitrary integer tick. The theorem applies until a first descendant
genesis event.

## 2. Exact pulse cancellation

For a nonzero production mode, write

\[
 a(k)=C_{\rm WAVE}^2M(k),\qquad
 \cos\theta(k)=1-\frac{a(k)}2.
\]

FTD-0586 derived the exact normalized step response

\[
 r_n=1-\cos(n\theta)+\tan(\theta/2)\sin(n\theta).
\]

If one source is removed after `T` ticks, its response at `n>=T` is

\[
 p_{n,T}=r_n-r_{n-T}.
\]

The constant `1` cancels before taking absolute values. Sum-to-product gives

\[
\begin{aligned}
p_{n,T}
 &=2\sin(T\theta/2)
 \left[\sin((n-T/2)\theta)
 +\tan(\theta/2)\cos((n-T/2)\theta)\right]\\
 &=\boxed{2\sec(\theta/2)\sin(T\theta/2)
 \sin\!\left((n-(T-1)/2)\theta\right)}.
\end{aligned}
\]

Therefore, uniformly in removal and observation time,

\[
 \boxed{|p_{n,T}(k)|\le2\sec(\theta(k)/2)
 =\frac{2}{\sqrt{1-C_{\rm WAVE}^2M(k)/4}}.}
\]

The FTD-0586 envelope `2(1+sec(theta/2))` was valid but retained two
constant step terms that cancel identically in every finite pulse.

## 3. One-pulse and hybrid history bounds

Fourier inversion and the vector triangle inequality give the exact-duration-
uniform one-source pulse bound

\[
 \boxed{
 P_L=\frac{2G_C}{C_{\rm WAVE}^2L^3}
 \sum_{k\ne0}
 \frac{\sqrt{\sum_a\sin^2k_a}}
 {M(k)\sqrt{1-C_{\rm WAVE}^2M(k)/4}}.}
\]

At any candidate first-event tick, let `r` originals have disappeared. The
`N-r` originals still present share one common step history, so FTD-0588
bounds them jointly by `C_L sqrt(N-r)`. Each removed original is one exact
finite pulse bounded by `P_L`. Hence

\[
 \boxed{|J(x,n)|\le H_L(N,r)
 :=C_L\sqrt{N-r}+rP_L,\qquad0\le r\le N.}
\]

This is independent of source geometry, polarity, removal schedule,
observation site, and observation tick. The all-off residual tail is the
special case `H_L(N,N)=NP_L`.

Putting `y=sqrt(N-r)` also gives the continuous relaxation

\[
\begin{aligned}
H_L(N,r)
 &=NP_L+C_Ly-P_Ly^2\\
 &\le NP_L+\frac{C_L^2}{4P_L},
\end{aligned}
\]

because

\[
 P_Ly^2-C_Ly+\frac{C_L^2}{4P_L}
 =\frac{(2P_Ly-C_L)^2}{4P_L}\ge0.
\]

The registered decision uses the sharper finite maximum

\[
 H_L^{\max}(N)=\max_{r=0,\ldots,N}H_L(N,r).
\]

## 4. Registered finite-volume result

Independent C++ long-double and Python binary64 evaluations agree within
`5e-15`:

| `L` | `P_L` | `C_L` | `H_L^max(6)` | maximizing `r` | margin | `H_L^max(7)` | maximizing `r` | margin |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 9 | 0.20776823543433087 | 0.30397065730643763 | 1.3428118344780919 | 5 | 0.17357422467388584 | 1.5505800699124228 | 6 | -0.03419401076044504 |
| 17 | 0.20987747517787672 | 0.30909222934825376 | 1.3584796052376373 | 5 | 0.15790645391434044 | 1.5683570804155140 | 6 | -0.05197102126353626 |
| 33 | 0.21046957467654109 | 0.31182601851585356 | 1.3641738918985591 | 5 | 0.15221216725341868 | 1.5746434665751003 | 6 | -0.05825740742312258 |
| 65 | 0.21062758886981753 | 0.31324294475520342 | 1.3663808891042910 | 5 | 0.15000517004768676 | 1.5770084779741087 | 6 | -0.06062241882213093 |

The threshold is `K_GENESIS=1.5163860591519780`. Thus every registered
volume closes every arbitrary one-time-removal history through `N=6`.
`N=7` is the first count not excluded by this inequality. The negative margin
for seven is not a genesis prediction.

## 5. First-event induction

Assume `N<=6` and suppose a first descendant genesis event exists. Immediately
before its event predicate, no descendant exists, so every field source is
still one of the original stationary sites with exactly one permitted on/off
history. For some `r`, the field therefore obeys

\[
 |J|\le H_L(N,r)\le H_L^{\max}(N)<K_{\rm GENESIS}.
\]

The strict inequality makes the genesis predicate false, contradicting the
assumed first event. Hence no first descendant can occur. This closes the
FTD-0588 five-source all-off residual tail and also closes arbitrary six-source
histories on the registered quotients.

## 6. Exact removal-time Gram diagnostic

For a fixed prescribed history define

\[
 K_L(d,n,T)=\frac{G_C}{C_{\rm WAVE}^2L^3}
 \sum_{k\ne0}\frac{-i\sin k}{M(k)}p_{n,T}(k)e^{ik\cdot d}.
\]

Then

\[
 \left|\sum_jq_jK_j\right|^2=q^T\mathcal Gq,
 \qquad \mathcal G_{ij}=K_i\cdot K_j.
\]

The observer checked this equality on 48 fixed histories. Its maximum residual
was `5.4210108624275222e-20`. Integer translations were exact, and all 24
proper cubic rotations closed with maximum covariance residual
`2.831408037109452e-17`. This verifies the operator bookkeeping; the theorem
does not optimize this Gram matrix over schedules.

## 7. Production conformance

The locked campaign executed 96 arms and 12,288 ticks:

- 64 prescribed permanent, synchronous, staggered, and paired histories;
- 32 native-unlocked histories;
- `L={9,17}`, `N={5,6}`, both polarities, and two independently selected
  moment-isotropic shapes;
- 176 native evaporation events;
- all four `(L,N)` cells exercised complete removal;
- zero genesis and zero bound contradictions;
- maximum observed flux `0.11074116846428322`;
- velocity and remainder bit-exact zero;
- history observer state/RNG neutrality exact.

The live null result is conformance evidence. Universal closure comes from the
inequality and first-event induction, not from the 96 histories.

## 8. Consequence and boundary

The apparent five-source residual opening in FTD-0588 came entirely from
bounding a rectangular pulse as two unrelated steps. Exact temporal
cancellation moves the causal state-gradient boundary to arbitrary histories
with at least seven sources.

This still supplies no reciprocal field-to-matter force, worldline identity,
momentum exchange, stable localized matter, particle pole, Lorentz recovery,
or scenario qualification. FTD-0590 subsequently evaluates the preregistered
cubic-orbit coherence relaxation and closes arbitrary histories through
`N=7`. FTD-0591 then closes the separately locked `N=8` evaluation. The next
causal-source count begins at `N=9`; the reciprocal nonlinear carrier/action
problem is unchanged.

## 9. Verification

- preregistration SHA-256:
  `F438DBB1950E009641B1332D57B23B2EDFC23CD522A4E23C17E5FCC967AF5A33`;
- focused native CTest: PASS;
- independent symbolic/spectral/result proof: 120/120 PASS;
- CSV arms: 96/96 valid;
- production/default/toggle/scenario changes: none.
