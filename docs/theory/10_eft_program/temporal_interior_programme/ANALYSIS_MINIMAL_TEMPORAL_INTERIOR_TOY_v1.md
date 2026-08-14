# ANALYSIS — The Minimal Temporal Interior: a Complete Computable Toy Model

**Status:** `[MODEL — ILLUSTRATIVE, MECHANISM-LEVEL]` +
`[EXACT — THREE INTERNAL IDENTITIES VERIFIED TO MACHINE PRECISION]` +
`[BOOKED — FTD-0811]`
**Date:** 2026-08-08 · **Artifact:** `scripts/experiments/temporal_interior/toy_minimal_temporal_interior.py`
**Figure:** `dissemination/papers/semantic_ontology/figures/fig9_toymodel.pdf`
**Parents:** `FOUND_SELECTION_POTENTIALITY_ACTUALIZATION_CHAIN_v1.md` (v1.3),
`SCOPE_TEMPORAL_INTERIOR_PROGRAM_v2.md`,
`ANALYSIS_CONE_SPEED_CHARACTERIZED_v1.md`,
`ANALYSIS_GEOMETRIC_BIT_REGISTER_SCREEN_v1.md`,
`PREREG_BORN_REGIME_MAP_ENGINE_v1.md`.
**Production impact:** none. No constant is changed; no tag moves; the toy
is a demonstrator, not an instrument of record.

---

## 1. Purpose

The temporal-interior programme has produced its results one wall at a
time: a cone speed characterized, a clock carrier constructed, a register
screened, a weighting regime measured. Each was established on its own
instrument, at its own scope, and the connective tissue between them has
so far been prose.

This document specifies a single *computable* system in which all five
pieces coexist and interact — small enough to run in seconds, exact enough
that three of its internal identities are verifiable to machine precision,
and honest enough that its one statistical estimate carries a bootstrap
interval rather than a point value. Its purpose is to make the
architecture's claim of *coherence* checkable rather than asserted: that
succession, duration, retention and actualization are not four separate
postulates but four consequences of a substrate, a carrier and one energy
scale.

The toy establishes no physics. Every quantitative claim of record remains
with its own instrument, at its own scope. What the toy adds is the
demonstration that those claims **compose**.

## 2. The five pieces

| # | piece | content | status of its content |
|---|---|---|---|
| 1 | substrate | causal polytopes; the light cone contained in all of them | exact geometry |
| 2 | clock | quartic oscillator $\ddot Q = -(4\lambda/m)Q^3$ | exact; closed-form period |
| 3 | register | double well $V(R)=\varepsilon(R^2-1)^2$ | exact; barrier $=\varepsilon$ |
| 4 | gate | threshold crossing of a field against $K$ | model; standard level-crossing |
| 5 | noise | two Ornstein–Uhlenbeck channels of correlation time $\tau$ | model; statistical estimate |

Pieces 1–3 are exact statements with independently known answers, so the
code can be checked against mathematics rather than against itself. Pieces
4–5 are the genuinely statistical part, and are reported with uncertainty.

### 2.1 Substrate — the contained cone (panel a)

In one tick influence reaches a finite neighbour set; its convex hull is
the causal polytope, and the reachable region after $n$ ticks is the
$n$-fold dilation. An isotropic cone of speed $c$ remains inside that
region for all time exactly when $c$ does not exceed the polytope's
inradius. The three candidate neighbourhoods give

$$\text{octahedron } \tfrac{1}{\sqrt3}, \qquad
\text{cuboctahedron } 1, \qquad \text{cube } 1 ,$$

so $C = 1/\sqrt3$ is the strictest containment bound over every candidate
— the cone fits under *all* readings of causal reach, with no commitment
required about which neighbourhood is fundamental. The panel draws this in
the plane: the dashed square is the cubic reach, the diamond the
octahedral reach, and the shaded disc the light cone, visibly interior to
both. Full argument and the accompanying tension: `ANALYSIS_CONE_SPEED_CHARACTERIZED_v1.md`.

### 2.2 Clock — the exact quartic oscillator (panels b, c)

The minimum viable carrier's mirror-even mode reduces, at quartic order,
to

$$m\ddot Q = -4\lambda Q^3 .$$

Energy conservation gives $\tfrac{m}{2}\dot Q^2 + \lambda Q^4 = \lambda A^4$,
so the quarter period is

$$\frac{T}{4} = \frac{1}{A}\sqrt{\frac{m}{2\lambda}}
\int_0^1\!\frac{du}{\sqrt{1-u^4}}
= \frac{1}{A}\sqrt{\frac{m}{2\lambda}}\cdot\frac{\sqrt\pi\,G^*}{4},$$

using $\int_0^1(1-u^4)^{-1/2}du = \tfrac14 B(\tfrac14,\tfrac12)
= \tfrac14\sqrt\pi\,\Gamma(1/4)/\Gamma(3/4)$. Hence the **period law**

$$\boxed{\;T\cdot A = \sqrt{\pi}\;G^*\;\sqrt{\tfrac{m}{2\lambda}}\;}
\qquad G^* = \Gamma(1/4)/\Gamma(3/4) = 2.958675119189\ldots$$

Two things follow, and both are *visible* rather than fitted.

First, the law is inverse, not isochronous: **a bigger swing is a faster
clock**. Panel (b) plots $Q(t)$ unscaled for $A = 0.30, 0.20, 0.12$ and
the three periods are $17.5, 26.2, 43.7$ — the amplitudes differ by
$2.5\times$ and the periods by the reciprocal factor. (Plotting the
*scaled* coordinate $Q/A$ against $t/T$ would collapse all three curves
onto one, since the waveform is amplitude-invariant by construction; that
is why the panel deliberately shows the unscaled trajectories.)

Second, the constant is $G^*$ and nothing is tuned to make it so. Panel
(c) integrates the equation of motion at six amplitudes spanning $6\times$
with `solve_ivp` at `rtol=1e-12`, detects the far turning point by event
detection, and reports

- fitted log–log slope $= -1.000000000$ (exactly $-1$),
- $\max_A |G^*_{\rm rec}/G^* - 1| = 1.61\times10^{-13}$,

where $G^*_{\rm rec} = T A \sqrt{2\lambda/m}/\sqrt\pi$ inverts the law with
no free parameter. The recovery is at integrator tolerance, which is the
correct outcome for an identity.

> **Reading guard.** $G^*$ here is the period constant of a quartic
> oscillator — a theorem about $\int_0^1(1-u^4)^{-1/2}du$. It is not, and
> must not be presented as, a substrate derivation of any physical
> constant. Its significance is that the clock's rate is set by a
> *lemniscatic* rather than a *harmonic* constant: a quadratic well would
> give $\pi$ and isochrony; the quartic well gives $G^*$ and amplitude
> dependence. That difference is the programme's content.

### 2.3 Register — one barrier, and it is $\varepsilon$ (panels d, e)

The bistable coordinate sits in

$$V(R) = \varepsilon\,(R^2-1)^2, \qquad V(\pm1)=0,\quad V(0)=\varepsilon,$$

so the barrier separating the two stable states equals the well depth
**exactly**, with no geometric prefactor. Panel (d) draws it. The
substrate-side counterpart of this identity — that the geometric bit's
saddle-path barrier is $\varepsilon$ to within $1.6$–$2.1\%$ across
$s\in[0.90,1.30]$, against a $\sim30\varepsilon$ through-core path — is the
result of `ANALYSIS_GEOMETRIC_BIT_REGISTER_SCREEN_v1.md`; the toy
reproduces the clean limit of that screen.

Retention then follows by Kramers/Arrhenius,

$$\tau_{\rm flip}\,\nu_0 \sim e^{\varepsilon/T},$$

plotted in panel (e). The exponential is the whole point: at
$T = 0.25\,\varepsilon$ a bit survives $\sim55$ attempt times, at
$T = 0.06\,\varepsilon$ it survives $\sim1.7\times10^7$. Memory is not a
separate mechanism bolted onto the substrate; it is the same well, read at
a different temperature.

### 2.4 Gate and noise — the weighting regime (panel f)

The gate is a threshold: a level crossing of a field $F$ upward through
$K = 0.5054620197$ registers one actual event. The field carries two
standing modes of wavenumbers $k_1, k_2$ with lattice-dispersion
frequencies

$$\Omega(k) = 2\arcsin\!\big(C\sin(k/2)\big),$$

and their amplitudes are set to **equal action**, $A_2 = A_1\sqrt{\Omega_1/\Omega_2}$,
so that $A_1^2\Omega_1 = A_2^2\Omega_2$. This is the discriminating
construction: the two modes then differ in amplitude but not in
occupation, so the two candidate weightings make opposite predictions for
the ratio of second-harmonic coefficients in the excess crossing rate —
$R = \Omega_1/\Omega_2 \equiv R_{\rm amp}$ under amplitude weighting,
$R = 1$ under occupation (Born) weighting. The Born-fraction

$$\mathrm{BF} = \frac{R - R_{\rm amp}}{1 - R_{\rm amp}}$$

is therefore $0$ for pure amplitude weighting and $1$ for pure Born
weighting, by construction.

Superposed on the modes are two Ornstein–Uhlenbeck noise channels of
correlation time $\tau$; the crossing count is differenced against a
signal-free control run to isolate the excess, and the second-harmonic
coefficients are extracted by least squares against
$\{1,\cos 2k_1x,\cos 2k_2x\}$.

Five cells at $L=1536$, $T=6000$ ticks, $10$ seeds give

| $\bar\Omega\tau$ | BF | $1\sigma$ (bootstrap, 800 resamples) |
|---|---|---|
| 0.32 | 0.028 | $-0.026/+0.028$ |
| 1.28 | 0.052 | $-0.027/+0.027$ |
| 5.07 | 0.100 | $-0.023/+0.021$ |
| 15.22 | 0.400 | $-0.034/+0.039$ |
| 58.77 | 0.774 | $-0.041/+0.047$ |

against the locked-run descriptive reference $0.860\,x^2/(16.6^2+x^2)$,
which predicts $0.0003, 0.005, 0.073, 0.393, 0.796$ at the same abscissae.
The two upper cells agree closely; the lower cells sit high by about one
standard deviation, which is what a deliberately small instrument should
look like when it is reported honestly. The intervals are shown, not
suppressed.

The structural claim the panel carries is the one that matters: **the
weighting of actual events is not a postulate but a regime**. When the
mode is slow against the noise correlation time the gate weights by
amplitude; when the mode outruns the noise bandwidth it weights by
occupation, which is the Born rule's content. The crossover is a ratio of
timescales, and the toy reproduces it from a threshold and two noise
channels alone.

## 3. The one-constant claim

The architecture's economy claim is that a single energy scale prices
three apparently unrelated things. In the toy this is literal — `EPS = 1.0`
is set once and enters:

1. **binding**, as the depth of the well that holds the carrier together;
2. **clock rate**, through $\lambda$, since the period law carries
   $\sqrt{m/2\lambda}$ and $\lambda$ is the quartic stiffness of that same
   well;
3. **retention**, as the Arrhenius barrier $\varepsilon$ in $e^{\varepsilon/T}$.

There is no separate memory constant, no separate clock constant, and no
separate binding constant. Raising $\varepsilon$ makes the carrier stiffer,
the clock faster and the bit longer-lived, together and in fixed
proportion. That is a falsifiable structural commitment: any measurement
that moved one of the three without the others would break it.

## 4. What the toy does and does not settle

It settles, as internal mathematics: the period law and its $G^*$
constant; the exactness of barrier $=\varepsilon$; the containment ordering
of the three polytopes. Each is verified in the artifact's own output.

It exhibits, as mechanism: that a threshold plus two noise channels
produces a weighting that interpolates between amplitude and occupation
as a function of $\bar\Omega\tau$ — the same qualitative behaviour, on an
independent instrument, as the locked v2 preregistration.

It does not establish that the substrate realizes any of this. The clock
carrier remains a constructed minimum-viable object, not a native one; the
native-carrier question is `[OPEN]` at its screened scope. The Born-regime
result remains mechanism-level with an `[IMPOSED]` ensemble, and the engine
campaign's own cells lie at $\bar\Omega\tau \in [0.19, 0.86]$ — below the
crossover, which is why the engine map returned a null. Nothing in the toy
alters those verdicts.

## 5. Reproduction

```
python scripts/experiments/temporal_interior/toy_minimal_temporal_interior.py
```

Runtime is under a minute on the reference host. All randomness is
seeded (`default_rng(1000 + 31·seed)`, phases from `default_rng(7)`,
bootstrap from `default_rng(99)`), so the printed `[verify]` block is
reproducible bit-for-bit. The script writes
`figures/fig9_toymodel.pdf` for the paper and a PNG alongside itself for
inspection.

Expected verification output:

```
[verify] clock: slope = -1.000000000  (exact -1)
[verify] clock: max |G*_rec/G*-1| = 1.609e-13
[verify] gate: Om*tau=   0.32  BF= 0.028 (-0.026/+0.028)
[verify] gate: Om*tau=   1.28  BF= 0.052 (-0.027/+0.027)
[verify] gate: Om*tau=   5.07  BF= 0.100 (-0.023/+0.021)
[verify] gate: Om*tau=  15.22  BF= 0.400 (-0.034/+0.039)
[verify] gate: Om*tau=  58.77  BF= 0.774 (-0.041/+0.047)
```
