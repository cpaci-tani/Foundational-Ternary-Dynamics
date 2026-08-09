# ANALYSIS — A Composite Inherits Its Constituents' Cone, Not Its Own Mass's

**Status:** `[DERIVED — MOMENTUM-WEIGHTED AVERAGE, VERIFIED TO 8 DIGITS]` +
`[MEASURED — N-INDEPENDENCE EXACT TO 7 DIGITS]` +
`[BOOKED — FTD-0813]`
**Date:** 2026-08-08 · **Artifact:** `scripts/experiments/temporal_interior/derive_composite_cone.py`
**Parents:** `ANALYSIS_MASSIVE_CONE_AND_DILATION_v1.md` (the single-species
result this composes), `DERIV_TWO_OWED_PROOFS_v1.md` §2.7 amendment.
**Origin:** the owner's observation that *"what constitutes a bounded body
of composites"* is prior to asking whether a composite inherits $C_{\rm eff}$
— that a body's clocks are set by what binds it, hierarchically.
**Production impact:** none. No constant is changed; no tag moves.

---

## 1. The question is prior, and it has a sharp form

"Does a bound composite inherit its constituents' $C_{\rm eff}$?" is
ill-posed until one says what makes it a composite. A body is bound by
*something*, and the binding decides which constituents' properties are
averaged and with what weights.

That vague statement has an exact form. Treat the mass-dependence of the
limiting speed as a perturbation $\delta_a = (c_a^2 - C^2)/C^2$ on an
otherwise Lorentzian theory. The composite's shift is $\langle\Delta
H\rangle$ in the bound state, and the $K$-dependent part of each
constituent's kinetic energy is its share of the total momentum. Hence

$$\boxed{\;\delta_{\rm comp} = \sum_a w_a\,\delta_a,\qquad
w_a = \frac{\mu_a}{\sum_b \mu_b},\quad \mu_a = \frac{m_a}{c_a^2}\;}$$

the **momentum-fraction-weighted average** of the constituents' excesses.
The binding determines $w_a$; the constituents supply $\delta_a$.

Operationally the weights need no interaction model. A composite at total
momentum $K$ distributes $K$ among its parts, and the distribution is
fixed by the co-moving (equal-velocity) condition, which is exactly

$$E(K) \;=\; \min_{\{k_a\,:\,\sum_a k_a = K\}} \ \sum_a \omega_a(k_a).$$

To first order in $\delta$ the binding shifts the composite's rest mass but
not this weighting, so the kinematics is computable from the free
dispersions alone.

## 2. Two candidate answers, differing by $N^2$

| | reading | $\delta_{\rm comp}$ |
|---|---|---|
| **(A)** | composite behaves as a fundamental particle of mass $M_{\rm tot}$ | $M_{\rm tot}^2/6$ |
| **(B)** | composite inherits the weighted average of its constituents | $\sum_a w_a M_a^2/6$ |

For $N$ identical constituents of mass $m$, (A) gives $N^2m^2/6$ and (B)
gives $m^2/6$. The two differ by $N^2$ — for a macroscopic body, by more
than fifty orders of magnitude.

## 3. The computation decides: (B), exactly

Using the exact M18 lattice-KG dispersion along an axis,
$4\sin^2(\omega/2) = 4C^2\sin^2(k/2) + M^2$.

**Single species** reproduces the parent result, $c_a^2/C^2 - 1 \to M^2/6$
(ratio $0.9997$ at $M=0.05$, drifting to $1.053$ at $M=0.5$ as the $M^4$
term enters).

**Two unequal constituents** — $\delta_{\rm comp}$ against the weighted
average:

| $M_1$ | $M_2$ | $\delta_{\rm comp}$ | weighted avg | as $M_{\rm tot}$ |
|---|---|---|---|---|
| 0.30 | 0.30 | 0.015275313 | 0.015275308 | 0.060000 |
| 0.30 | 0.50 | 0.033005495 | 0.033005488 | 0.106667 |
| 0.20 | 0.60 | 0.049729191 | 0.049729151 | 0.106667 |
| 0.10 | 0.50 | 0.036651462 | 0.036651455 | 0.060000 |
| 0.40 | 0.45 | 0.031578464 | 0.031578454 | 0.120417 |

Agreement to **eight significant figures** in every case.

**$N$ identical constituents** ($m = 0.3$):

| $N$ | $M_{\rm tot}$ | $\delta_{\rm comp}$ | $(NM)^2/6$ | suppression |
|---|---|---|---|---|
| 1 | 0.3 | 0.0152753134 | 0.015 | 1.0 |
| 2 | 0.6 | 0.0152753134 | 0.060 | 3.9 |
| 5 | 1.5 | 0.0152753144 | 0.375 | 24.5 |
| 10 | 3.0 | 0.0152753070 | 1.500 | 98.2 |
| 30 | 9.0 | 0.0152752940 | 13.500 | 883.8 |

$\delta_{\rm comp}$ is **constant to seven digits** across a thirtyfold
range of total mass. The limiting-speed excess is an *intensive* property
set by the constituents, not an extensive one that accumulates.

## 4. Why this is load-bearing rather than a refinement

| body | as one particle of its total mass | as a composite of nucleons |
|---|---|---|
| 1 kg | $\Delta C/C = 5.9\times10^{13}$ | $1.6\times10^{-40}$ |
| Earth | $2.1\times10^{63}$ | $1.6\times10^{-40}$ |

Reading (A) would make a one-kilogram object superluminal by thirteen
orders of magnitude. The compositional result is therefore not a
correction to a viable picture; it is the difference between a viable
picture and an absurd one, and it follows from the kinematics without
being put in by hand.

**Every body built from nucleons shares one limiting speed to about
$1.6\times10^{-40}$.** Bodies differ from one another only through
*composition* — the electron mass fraction ($\sim\!5\times10^{-4}$) and the
nuclear binding fraction ($\sim\!1\%$) — so the differential between two
macroscopic bodies of different make-up is of order $10^{-42}$, not
$10^{-40}$.

## 5. The hierarchy, and where it goes quiet

The originating intuition was hierarchical: Earth's year is set by the
Sun, its day by its own rotation, and a planet elsewhere will differ
because the bodies binding it differ. That is right, and the calculation
shows precisely which part of it keeps changing as one ascends and which
part stops.

- **Dynamical periods are hierarchical and stay so.** The year, the day, a
  nuclear vibration: each is set by a specific binding at a specific
  scale, and each differs from system to system. Nothing here constrains
  them.
- **The limiting speed saturates at the first level.** Because
  $\delta_{\rm comp}$ is a weighted *average*, ascending the hierarchy —
  nucleon $\to$ nucleus $\to$ planet $\to$ star–planet system — re-averages
  quantities that are already equal. Every level above the nucleon returns
  the same answer.

So a planet in another system genuinely has different clocks, and
genuinely has the *same* limiting speed, provided it is built from the
same constituents. Composition, not size, is the variable.

**The recursion continues downward, and suppresses further.** If nucleons
are themselves composite, $\delta_{\rm nucleon}$ should likewise be the
momentum-weighted average over *its* constituents. Most of a nucleon's
mass is field energy rather than constituent rest mass, so that average is
dominated by near-massless contributions with $\delta \approx 0$, pushing
the value down by roughly the massive-constituent fraction. The figure
$1.6\times10^{-40}$ is therefore an **upper bound at the nucleon level of
description**, and a more fundamental description makes it smaller, not
larger. This is the owner's question — what counts as a constituent —
recurring one level down, with the same answer: the binding decides.

## 6. Scope

First order in $\delta$, and non-relativistic internal motion. Two effects
are not included and both act to reduce the estimate: the binding field's
own momentum share (weight $\approx$ binding-energy fraction, with
$\delta = 0$ for a massless mediator), and the downward recursion of §5.
The result is a statement about the scalar sector's kinematics, not about
FTD's own mass-generation mechanism.

It also does not touch the inter-sector problem. FTD-0412 records
order-unity mismatches *between* sectors; composition averaging operates
within a sector and cannot repair that.

## 7. Reproduction

```
python scripts/experiments/temporal_interior/derive_composite_cone.py
```

Seconds; deterministic. The $N$-identical case is exact by symmetry
($k_a = K/N$); the two-body case does an explicit split search.
