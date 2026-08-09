# ANALYSIS — The One-Energy Carrier Dilates, and Universally

**Status:** `[MEASURED — UNIVERSAL DILATION, EXPONENT −1.000 ± 0.002]` +
`[DERIVED — THE ONE-ENERGY REQUIREMENT IS WHAT RESTORES UNIVERSALITY]` +
`[OPEN — THE CLOCK'S LIMITING SPEED AND THE ENERGY'S DISAGREE BY ~6%]` +
`[BOOKED — FTD-0814]`
**Date:** 2026-08-08 · **Artifact:** `scripts/experiments/temporal_interior/derive_kink_clock_dilation.py`
**Parents:** `DERIV_CARRIER_CONSTITUENTS_ONE_ENERGY_v1.md` (the carrier),
`ANALYSIS_COMPOSITE_CLOCK_DILATION_v1.md` (the two-category failure this
is measured against).
**Production impact:** none. No constant is changed; no tag moves.

---

## 1. The comparison this exists to make

| carrier | structure | dilation exponent $p$ | spread |
|---|---|---|---|
| nodes + static well | **two** energy categories | $[-2.70,\ -0.94]$ | **1.76** |
| $\varphi^4$ kink | **one** energy functional | $[-1.180,\ -1.137]$ | **0.043** |

Same lattice, same probe, same test. **Collapsing the two energy
categories into one reduces the spread of the dilation exponent by a
factor of 41.** Universality — the requirement that every clock slow by
the same factor, which is the entire content of time dilation — is
restored by the structural change and by nothing else.

## 2. Instrument

$\varphi^4$ model $U=\tfrac{\lambda}{4}(\varphi^2-v^2)^2$ on the axial
lattice, $N=4096$, kink–antikink separation $1800$, $T=24000$ ticks.
Boosted with the exact continuum-covariant profile
$$\varphi = v\tanh\!\big(\gamma(x-ut)/w\big),\qquad
\dot\varphi = -\tfrac{u\gamma v}{w}\,\mathrm{sech}^2\!\big(\gamma(x-ut)/w\big),$$
shape mode excited by a $7\%$ width squeeze, read from the FFT peak of
$\max_x|\partial_x\varphi|$ — a translation-invariant probe, necessary
because a moving carrier leaves any fixed site.

Rest-frame validation: measured $\Omega(0)$ against the analytic
$(\sqrt3/2)m$ gives ratios $0.9995$, $0.9996$, $1.0014$ for
$\lambda = 0.02, 0.03, 0.05$.

**A discreteness limit, found the hard way.** A first pass at
$\lambda = 0.10, 0.16$ lost the shape mode entirely at $u/C \ge 0.4$
(measured $\Omega \sim 10^{-3}$, i.e. the FFT floor). The failures track
the *contracted* kink width falling below $\sim\!2$ sites: $\lambda=0.16$
gives $w=2.04$, which at $\gamma=1.25$ is $1.6$ sites and is no longer a
kink the lattice can carry. All results below use $w \ge 3.6$ and carry a
width guard.

## 3. Result

| $\lambda$ | $w$ | $u/C$ | $\gamma$ | $\Omega$ | $p$ |
|---|---|---|---|---|---|
| 0.02 | 5.77 | 0.25 / 0.40 / 0.50 | 1.033 / 1.091 / 1.155 | 0.16690 / 0.15675 / 0.14694 | $-1.134$ / $-1.139$ / $-1.140$ |
| 0.03 | 4.71 | 0.25 / 0.40 / 0.50 | " | 0.20453 / 0.19177 / 0.17966 | $-1.120$ / $-1.154$ / $-1.153$ |
| 0.05 | 3.65 | 0.25 / 0.40 / 0.50 | " | 0.26409 / 0.24740 / 0.23104 | $-1.168$ / $-1.181$ / $-1.192$ |

Per-family means $-1.137$, $-1.142$, $-1.180$; **spread $0.043$**, and
monotone toward $-1$ as the kink widens ($w = 3.65 \to 5.77$).

**Allowing the carrier its own limiting speed makes the form exact.**
Fitting the single parameter $c$ in $\Omega(u)/\Omega(0)=\sqrt{1-u^2/c^2}$
— one parameter against three velocities, so two degrees of freedom
remain — gives

| $\lambda$ | $c_{\rm fit}/C$ | $p$ using $c_{\rm fit}$ |
|---|---|---|
| 0.02 | 0.94406 | $-1.00198$ |
| 0.03 | 0.94049 | $-0.99756$ |
| 0.05 | 0.92775 | $-1.00019$ |

$p = -1.000 \pm 0.002$. The functional form is exactly relativistic, and
$c_{\rm fit}$ varies by only $1.7\%$ across the family while trending
toward $C$ as $w$ grows.

## 4. The tension, stated plainly

Two independent determinations of the kink's limiting speed **disagree**:

| route | $c/C$ |
|---|---|
| from the energy, $E(u)/E(0) = \gamma$ | $1.0010,\ 1.0015,\ 1.0025$ |
| from the clock, $\Omega(u)/\Omega(0) = 1/\gamma$ | $0.944,\ 0.940,\ 0.928$ |

The kink's *energy* transforms with $c \simeq C$ to two parts in a
thousand; its *internal clock* transforms with a $c$ about $6\%$ lower. In
an exactly Lorentz-covariant theory these are the same number. They are
not, and the discrepancy is a lattice-discreteness effect: it shrinks as
the kink widens ($0.928 \to 0.940 \to 0.944$ for $w = 3.65 \to 5.77$), but
three points do not establish that it reaches $C$.

So the honest statement has two halves and neither should be dropped:

> **The form of the dilation is exactly relativistic and universal.** The
> exponent is $-1$ to $0.2\%$ across a carrier family, where the
> two-category model could not produce a single exponent at all.
>
> **The lattice is not exactly covariant, and it shows here.** The clock
> and the energy do not agree on the limiting speed, at the $6\%$ level
> for kinks a few sites wide.

## 5. What this settles and what it does not

**Settles.** The one-energy requirement — derived in
`DERIV_CARRIER_CONSTITUENTS_ONE_ENERGY_v1.md` from the failure of every
patch to the two-category model — is *sufficient to restore universal
dilation*. That was the open question at the end of that document, and it
is answered affirmatively and quantitatively.

It also closes the composite-boost item's structural half. The requirement
on a carrier is no longer "constituents with the substrate dispersion plus
a covariantly-contracting binding," stated as an aspiration; it is
"a configuration of one energy functional," demonstrated to work.

**Does not settle.** This carrier is the $\varphi^4$ kink, whose internal
mode is **isochronous** — a $\pi$-clock, not the lemniscatic $G^*$ clock
the programme wants. The structural tension of
`DERIV_CARRIER_CONSTITUENTS_ONE_ENERGY_v1.md` §5 stands untouched: stable
configurations have locally quadratic minima and therefore harmonic
internal modes, while $G^*$ requires a degenerate direction stabilized at
fourth order. **What has been shown is that a one-energy carrier dilates,
not that a $G^*$ one exists.**

Nor does it establish exact lattice covariance — §4 shows it fails at
$6\%$ for these widths.

## 6. Reproduction

```
python scripts/experiments/temporal_interior/derive_kink_clock_dilation.py
```

A few minutes. Deterministic. The run carries a validity guard rejecting
any point whose measured frequency leaves $[0.3, 1.3]\times\Omega(0)$ or
whose contracted width falls below $3$ sites — the regime where the first
pass silently returned the FFT floor and would have been read as a
catastrophic dilation failure.
