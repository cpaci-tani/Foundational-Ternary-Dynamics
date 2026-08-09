# ANALYSIS — What a Distance Potential Is, and Why the Clock Gate Was Mis-Diagnosed

**Status:** `[DERIVED — TWO INDEPENDENT LIMITS SEPARATED]` +
`[MEASURED — MVC OPERATING POINT VIOLATES BOTH]` +
`[CORRECTION — supersedes the 2026-08-08 claim that the MVC is categorically the wrong kind of object]` +
`[BOOKED — FTD-0813]`
**Date:** 2026-08-08 · **Artifact:** `scripts/experiments/temporal_interior/probe_potential_validity.py`
**Parents:** `ANALYSIS_MASSIVE_CONE_AND_DILATION_v1.md`,
`ANALYSIS_COMPOSITE_CONE_INHERITANCE_v1.md`,
`DERIV_TWO_OWED_PROOFS_v1.md` §2.7 amendment (**which this corrects**).
**Origin:** the owner's question — *is a single voxel a distance potential?*
**Production impact:** none. No constant is changed; no tag moves.

---

## 1. A voxel is not a potential

A single voxel is a state ($s\in\{-1,0,+1\}$, flux $J$) together with a
local update rule. It carries no notion of distance and no notion of
potential. Both are **emergent two-body descriptions**, obtained by
localizing two sources, integrating out the field between them, and taking
the propagation time to zero. There is no scale at which one voxel *is* a
distance potential; the potential is a statement about a relation, and a
relation needs two relata and the field between them.

## 2. Two independent limits hide in "distance potential"

Writing $V(|q_i - q_j|)$ takes two separate limits, controlled by
different small parameters:

| | limit | replaces | controlled by |
|---|---|---|---|
| **(1)** | constituent dispersion | $\omega = \sqrt{c^2k^2+M^2}\ \to\ p^2/2m$ | $v/c$ |
| **(2)** | binding retardation | retarded field exchange $\to$ instantaneous $V(r)$ | $\omega r/c$ |

**Only (1) destroys time dilation, and hydrogen is the proof.** The
Coulomb potential is instantaneous to $O(\alpha^2)$ — limit (2) is taken,
and is an excellent approximation at $\omega a_0/c = \alpha/2 \approx
3.6\times10^{-3}$ — yet atoms dilate *exactly*. The dilation comes from the
electron's relativistic dispersion, not from retardation of the binding.

This is precisely what the composite calculation already showed:
$\delta_{\rm comp}$ is a momentum-weighted average of the **constituents'**
$\delta_a$, with the binding supplying only the weights. The binding's own
character enters at higher order.

## 3. The MVC violates both limits at its own operating point

| $A$ | $T$ | $v_{\max}/C$ | $(v/C)^2$ | $\omega$ | $\omega r/C$ ($r{=}1$) | $r{=}3$ |
|---|---|---|---|---|---|---|
| 0.12 | 43.70 | 0.025 | 0.0006 | 0.1438 | 0.249 | 0.747 |
| 0.20 | 26.22 | 0.069 | 0.0048 | 0.2396 | 0.415 | 1.245 |
| **0.30** | **17.48** | **0.156** | **0.0243** | **0.3594** | **0.623** | **1.868** |
| 0.50 | 10.49 | 0.433 | 0.1875 | 0.5991 | 1.038 | 3.113 |

Against systems where the potential description *is* licensed:

| system | $\omega r/c$ | potential description |
|---|---|---|
| Earth–Sun orbit | $9.9\times10^{-5}$ | licensed |
| hydrogen atom | $3.6\times10^{-3}$ | licensed |
| heavy nucleus | $0.30$ | not licensed |
| **MVC at $A=0.30$** | $0.62$ | **not licensed** |
| MVC, $r=3$ | $1.87$ | not licensed |

The MVC is two to three orders of magnitude further from the quasi-static
regime than an atom, and **more relativistic internally than a heavy
nucleus**.

## 4. The correction

The 2026-08-08 amendment to `DERIV_TWO_OWED_PROOFS_v1.md` §2.7 stated that
the MVC "is the wrong kind of object, and a replacement must be bound by
the substrate field rather than by a distance potential." **Both halves of
that are wrong, and §2 above says why.**

- The axis named was the wrong one. Retardation of the binding is not
  what supplies dilation; hydrogen has an instantaneous binding and dilates
  exactly. What supplies dilation is the *constituents'* dispersion.
- The MVC is therefore not categorically unsuitable. It is
  **under-modelled**: its nodes were given Newtonian point-mass dispersion
  $p^2/2m$, which is the $c\to\infty$ limit of the constituent dispersion,
  and that single substitution is what makes it Galilean.

What survives unchanged is the narrow operational point: the MVC **as
written** is Galilean and would report $T(v) = T(0)$ exactly, so it cannot
be used as a dilation instrument in its present form.

**Revised statement of the gate.** The fix is a modelling change, not a
carrier search: give the nodes the lattice dispersion instead of $p^2/2m$,
and by the composite result the carrier inherits dilation automatically,
with the binding supplying only the momentum weights. The physics
currently discarded is of order $(v/C)^2 = 2.4\%$ at $A = 0.30$ — not
$10^{-40}$, and comfortably measurable.

> ⚠ **RETRACTED same day** — `ANALYSIS_COMPOSITE_CLOCK_DILATION_v1.md`
> ran the substitution. It is **necessary but not sufficient**, and the
> paragraph above is withdrawn.
>
> With Newtonian nodes the internal clock is K-independent to
> $8.7\times10^{-6}$ while $\gamma$ reaches $1.40$ — so the substitution
> is indeed necessary. But with lattice nodes the dilation exponent $p$ in
> $\Omega(K)/\Omega(0)=\gamma^{\,p}$ is **not universal**: it spans
> $[-2.700, -0.937]$ across constituent mass and binding fraction, where
> relativity admits $p=-1$ for every clock. A material-dependent rate is
> not dilation.
>
> The cause is the *other* limit, dismissed in §2 above. The relative
> effective mass obeys $\mu_K = \mu_0\gamma^3$ exactly (verified), but a
> static lab-frame well does not Lorentz-contract and supplies no
> compensating material-independent factor. **Hydrogen dilates exactly
> because its binding fraction is $\sim10^{-5}$, not because non-covariant
> binding is harmless** — at binding fractions $0.04$–$0.31$ it is the
> leading error.
>
> Corrected: the constituent dispersion controls whether the clock dilates
> **at all**; the covariance of the binding controls whether the dilation
> is **universal**. Both are required. §2's table below should be read with
> that amendment.

## 5. The obstruction that does not dissolve

At every amplitude in the table the internal frequency lies **inside the
propagating band** ($\omega < 2\arcsin C = 1.2310$). A carrier oscillating
there couples to travelling substrate modes and radiates, so it is a
resonance rather than a stable clock. This is the separately recorded C2
band-clearance requirement, whose stated cost ($\varepsilon \ge 4.22$ at
$A_{\max} = 0.5$) is already flagged as unaffordable.

That obstruction is real, is untouched by §4, and is now the *sole*
identified blocker on the composite-boost item — a sharper and more
tractable statement than "no suitable carrier exists." The two failure
modes should not be conflated again:

| issue | status |
|---|---|
| nodes Newtonian $\Rightarrow$ no dilation | **modelling error; fixable by substitution** |
| $\omega$ inside the band $\Rightarrow$ radiates | **genuine obstruction; C2, cost recorded** |

## 6. Reproduction

```
python scripts/experiments/temporal_interior/probe_potential_validity.py
```

Seconds; closed-form throughout.
