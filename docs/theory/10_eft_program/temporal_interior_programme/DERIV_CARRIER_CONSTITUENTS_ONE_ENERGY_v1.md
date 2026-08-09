# DERIVATION — The Carrier Constituents, from "Energy Has To Be One"

**Status:** `[DERIVED — ONE-ENERGY REQUIREMENT, FROM THE FAILURE OF EVERY PATCH]` +
`[MEASURED — A WORKING ONE-ENERGY CARRIER EXISTS AND IS ISOCHRONOUS]` +
`[STRUCTURAL — STABILITY AND NON-ISOCHRONY ARE IN TENSION]` +
`[BOOKED — FTD-0814]`
**Date:** 2026-08-08 · **Artifacts:** `scripts/experiments/temporal_interior/derive_composite_clock_dilation.py`,
this document's inline runs.
**Parents:** `ANALYSIS_COMPOSITE_CLOCK_DILATION_v1.md` (the failure this
starts from), `ANALYSIS_MINIMAL_TEMPORAL_INTERIOR_TOY_v1.md` (the MVC and
its quartic law).
**Origin:** the owner's constraint — *break it down semantically and derive
the carrier constituents; energy has to be one.*
**Production impact:** none. No constant is changed; no tag moves.

---

## 1. The semantic diagnosis: a type error, not a parameter error

The failed carrier carried **two energy categories with different
transformation laws**:

| category | object | behaviour under boost |
|---|---|---|
| constituent kinetic | $\omega(q)$, substrate-native | $\mu_K = \mu_0\gamma^3$, material-independent |
| binding | $V(r)$, an added potential | static; does not transform at all |

A potential is a *second ontological category* laid on top of the
substrate, and the substrate has no rule by which to transform it. In the
project's own vocabulary this is a **type** failure: a content-level
parameter cannot repair a mis-set type. That is why the dilation exponent
came out non-universal ($p \in [-2.70,-0.94]$) rather than merely wrong —
no choice of $G$, $R$ or $M$ was ever going to fix it.

## 2. The one-energy requirement is derived, not assumed

The obvious repair is to give the second category a transformation rule:
let the binding region Lorentz-contract, $R \to R/\gamma$,
self-consistently. **This was tested and it fails.**

| $M$, well $(G,R)$ | $p$, static well | $p$, contracting well |
|---|---|---|
| 0.40, (0.10, 6) | $-0.367$ | $-0.104$ |
| 0.40, (0.30, 3) | $-0.141$ | $-0.012$ |
| 0.25, (0.06, 7) | $+0.550$ | $+0.138$ |
| 0.60, (0.20, 5) | $-0.858$ | $-0.271$ |
| 0.80, (0.40, 4) | $-1.097$ | $-0.367$ |

Contraction moves $p$ toward **zero**, not toward $-1$, and the direction
depends on the well's shape: for a $\mathrm{sech}^2$ well the deep-limit
gap scales as $\mu^{-1/2}$, not the $\mu^{-1}$ that a particle-in-a-box
argument would give. The "rescale $\mu$ and $L$ separately" decomposition
is a *non-relativistic* decomposition and does not reproduce covariance.

> **Conclusion.** There is no assignment of transformation rules to a
> separately-specified potential that reproduces covariance. Covariance is
> a property of the whole functional, not a rule attachable to a term.
> Therefore the carrier's energy must be **one functional of the substrate
> field, with no separately-specified interaction**. This is forced by the
> failure of the patch, not adopted for elegance.

## 3. What that forces about the constituents

With energy a single functional
$$E[\varphi] = \sum_x\Big[\tfrac12\dot\varphi^2
 + \tfrac{C^2}{2}\,\varphi(-L_{18})\varphi + U(\varphi)\Big],$$
the "constituents" are not parts. There are no nodes and no struts; there
is a field configuration, and what was called binding is the same $U$ that
sets the clock rate. One coupling does both jobs — which is the content of
*energy has to be one*.

Two exact facts then fix $U$.

**(i) Isochrony is equivalent to a quadratic minimum.** For $U\sim q^n$,
$T \sim A^{1-n/2}$: only $n=2$ is amplitude-independent. Verified for
$n = 2, 4, 6$.

**(ii) The $G^*$ law is the pure quartic.** Integrating
$\ddot q = -(4\lambda/m)q^3$ gives
$$T\!\cdot\!A = 5.244115109 = \sqrt{\pi}\,G^*$$
to ten digits, at every amplitude tested $(0.15, 0.30, 0.50)$.

## 4. A working one-energy carrier exists — and it is isochronous

The $\varphi^4$ model $U = \tfrac{\lambda}{4}(\varphi^2-v^2)^2$ is one
bounded functional with no added potential. Its kink is an exact
configuration. (A single kink is topologically incompatible with periodic
boundaries; a well-separated kink–antikink pair is the consistent object.)
With $v=1$, $\lambda=0.1$: meson mass $m = 0.447214$, width $2.582$.

| check | result |
|---|---|
| stability, 4000 ticks | $\max|\Delta\varphi| = 5.7\times10^{-3}$ |
| internal mode measured | $\omega = 0.387987$ |
| predicted $(\sqrt3/2)m$ | $0.387298$ — **ratio 1.0018** |
| below continuum edge $m$? | yes: $0.388 < 0.447$ — **bound; band clearance holds** |

and, decisively for the programme:

| perturbation amplitude | 0.111 | 0.030 | 0.063 |
|---|---|---|---|
| measured $\omega$ | 0.387987 | 0.387987 | 0.387725 |

**constant to $7\times10^{-4}$ over a $3.7\times$ amplitude range.** The
carrier the one-energy requirement delivers is a *harmonic* clock.

## 5. The structural tension, stated precisely

This is the substantive finding, and it is a near-no-go.

- **Any stable configuration has a locally quadratic minimum.** If the
  Hessian of $E$ is positive definite, small oscillations about it are
  harmonic — isochronous — by definition. So a stable one-energy carrier
  gives $\pi$, not $G^*$.
- **The $G^*$ law requires the quadratic term to vanish** at the clock
  coordinate, leaving pure quartic. That is a *degenerate* minimum:
  marginal at second order.
- The naive way to reach the anharmonic regime — a large-amplitude
  localized lump — does not survive. Focusing quartic lumps collapse
  (blow-up within $\sim$8 ticks at $M=0$) or disperse ($6$–$25\times$
  width growth over 6000 ticks); defocusing ones disperse harder
  ($83$–$145\times$).

So stability and non-isochrony pull against each other, and the precise
requirement on a $G^*$ carrier is:

> **a non-symmetry zero mode of the Hessian, stabilized at fourth order.**

The clock coordinate must have vanishing quadratic stiffness — a
first-order flex — which is *blocked* at second order rather than at
first. That is exactly Connelly prestress stability, i.e. the self-stress
condition the carrier programme already identified for the MVC. The
derivation therefore closes a loop: the same structure the mechanical
search found is what the field formulation independently demands — but it
must now be realized **as a configuration of one energy functional**,
not as nodes joined by struts.

This also explains the corpus's standing framing, "$\pi$ native, $G^*$
priced," in structural rather than empirical terms. The harmonic clock is
native because *every* stable configuration has one. The lemniscatic clock
is priced because it requires a degenerate direction, and degeneracy is
not generic.

## 6. What is not established

That no such field configuration exists. §5 shows that naive lumps fail
and that stable configurations are generically harmonic; it does not
exclude a self-stressed field configuration with a blocked flex. Finding
one — or proving it impossible — is now the sharply-stated open item, and
it is a better-posed question than "find a native carrier."

Nothing here shows that the $\varphi^4$ kink dilates correctly either. It
is a working one-energy carrier with a bound internal mode; whether its
shape mode's frequency transforms as $1/\gamma$ universally is untested,
and is the natural next measurement now that a legitimate carrier is in
hand.

## 7. Reproduction

The kink numbers come from a $2048$-site lattice, $C=1/\sqrt3$, leapfrog,
kink–antikink separation $760$, $24000$ ticks, shape mode excited by
squeezing the width and read from the FFT peak of a probe two widths off
the kink centre.
