# ANALYSIS — Causal Isotropy is Amplitude-Dependent, and Restores as a Power of Time

**Status:** `[MEASURED — SCALING LAW, ONE-BODY SECTOR]` +
`[MEASURED — SUPPORT-EDGE AMPLITUDE IS SMALL IN THE SAMPLED RUN; EXACT SUPPORT REMAINS]` +
`[BOOKED — FTD-0811]`
**Date:** 2026-08-08 · **Artifact:** `scripts/experiments/temporal_interior/probe_causal_isotropy.py`
**Parents:** `ANALYSIS_CAUSAL_CELL_3D_v1.md` (the four regions),
`ANALYSIS_CONE_SPEED_CHARACTERIZED_v1.md` (which this **amends**, §7 there),
`DERIV_TWO_OWED_PROOFS_v1.md` §2 (the free-sector result this cross-checks).
**Production impact:** none. No constant is changed; no tag moves.

---

## 1. The tension this resolves

A discrete substrate has two causal boundaries, and they have *different
symmetry*:

- the **strict support**, set by the stencil. For M18 it is the
  cuboctahedron, whose radius runs from $t$ (face direction) to
  $\sqrt2\,t$ (edge direction) — anisotropic by
  $(\sqrt2-1)/\tfrac12(\sqrt2+1) = 34.3\%$;
- the **signal front**, set by the dispersion. Since
  $\Omega = C|k| + O(k^3)$ with the leading term exactly isotropic, the
  front is a *sphere* of radius $Ct$.

Both are real, so "is the causal structure isotropic?" has no unqualified
answer. It depends on the amplitude at which one can still see. This
document measures that dependence.

## 2. The measurement

The Green's function of a point source is probed along 600 Fibonacci
directions; along each, the outward-running envelope of $|\phi|$ gives a
unique crossing radius $R_u(\varepsilon)$, and the anisotropy of that
surface is $A(\varepsilon) = (R_{\max}-R_{\min})/\bar R$.

At $t = 24$ (front $13.856$, support $24.0$–$33.94$, peak $|\phi| = 0.137$):

| $\varepsilon$ | $\bar R$ | $A$ | rms$/\bar R$ | $[100]$ | $[110]$ | $[111]$ |
|---|---|---|---|---|---|---|
| $10^{-3}$ | 14.79 | **0.71 %** | 0.17 % | 14.76 | 14.77 | 14.76 |
| $10^{-5}$ | 18.27 | 1.48 % | 0.30 % | 18.20 | 18.25 | 18.32 |
| $10^{-8}$ | 21.30 | 2.71 % | 0.53 % | 20.97 | 21.35 | 21.22 |
| $10^{-12}$ | 24.30 | 4.82 % | 1.00 % | 23.85 | 24.84 | 24.11 |
| $10^{-15}$ | 26.08 | **7.17 %** | 1.50 % | 24.98 | 26.75 | 25.85 |

Two readings. At the signal front the causal surface is a sphere to
**0.71 %**, with the three symmetry directions agreeing to three decimal
places. And fifteen decades down it has reached only **7.2 %** — still a
factor of five short of the stencil's own $34.3\%$. **The cuboctahedral
anisotropy is a property of the support bound, not of the field, and is
not attained at any measurable amplitude.**

## 3. The scaling law

The front amplitude itself decays with $t$, so a fixed $\varepsilon$
ladder drifts relative to the front. The threshold is therefore set
self-normalizingly to $\eta \times$ (envelope at $r = Ct$):

| $t$ | envelope at front | $A(\eta{=}10^{-2})$ | $A(\eta{=}10^{-4})$ |
|---|---|---|---|
| 10 | $4.95\times10^{-3}$ | 3.21 % | 7.09 % |
| 14 | $2.94\times10^{-3}$ | 2.27 % | 4.85 % |
| 18 | $1.96\times10^{-3}$ | 2.07 % | 3.19 % |
| 24 | $1.24\times10^{-3}$ | 1.35 % | 2.44 % |
| 30 | $8.69\times10^{-4}$ | 0.99 % | 1.83 % |
| 36 | $6.48\times10^{-4}$ | **0.81 %** | **1.33 %** |

$$A(t) \sim t^{-1.09}\ (\eta = 10^{-2}), \qquad
  A(t) \sim t^{-1.28}\ (\eta = 10^{-4}).$$

> **Isotropy is restored in the continuum limit, as a power of elapsed
> time.** The exponents bracket and trend toward $-4/3$, which is the
> Airy-front prediction: a dispersive front broadens as $t^{1/3}$, so its
> effective wavenumber falls as $t^{-1/3}$, and an $O(k^4)$ anisotropy in
> the dispersion then displaces the front by $O(t^{-4/3})$. The deeper
> probe, which sits further into the steeply-falling precursor where the
> crossing is best conditioned, is the closer to the prediction.

*A conditioning caveat, stated because it looks like a contradiction.* At
$\eta = 1$ the measured anisotropy is *larger* ($25\% \to 10\%$) and
scales more weakly ($t^{-0.73}$). This is not a competing result: the
envelope is nearly flat at the front, so a small direction-to-direction
difference in amplitude maps to a large difference in crossing radius. The
$\eta = 1$ probe is ill-conditioned by construction and is reported only
so that its exclusion is on the record.

## 4. The sampled support boundary is weak, not inaccessible

The gap between the front ($Ct$) and the support edge ($\sqrt2\,t$) grows
linearly in $t$, while the precursor decays exponentially across it. The
relative amplitude *at* the support boundary therefore falls
geometrically:

| $t$ | 8 | 12 | 16 | 20 | 24 |
|---|---|---|---|---|---|
| $\max|\phi|_{\rm edge} / \max|\phi|$ | $2.5\times10^{-7}$ | $1.8\times10^{-12}$ | $2.7\times10^{-17}$ | $9.9\times10^{-22}$ | $2.0\times10^{-26}$ |

a fitted decay of **1.19 decades per tick** over these five sampled times.
The value $2.7\times10^{-17}$ is below the spacing of binary64 numbers
near unity, but it is not below binary64 representability; binary64 can
represent normal values down to about $10^{-308}$. Nor does this finite
sample establish an asymptotic decay law or a physical detection floor.

> **Consequence.** Holding the operator fixed at M18, the *declared causal
> neighbourhood* fixes where the field's exact zero begins. In the sampled
> point-source run the support-edge amplitude becomes very small relative
> to the peak; exact support remains mathematically distinguishable and its
> physical detectability has not been established.

## 5. What this amends

`ANALYSIS_CONE_SPEED_CHARACTERIZED_v1.md` §5 records a live tension: the
value $1/\sqrt3$ is forced by containment only under *octahedral*
causality, whereas P4 commits to Moore and the production operator is M18,
under both of which the containment bound is $1$ and $1/\sqrt3$ is a
strictly interior choice. It closes by recommending option 3 — retain the
value, justified as the strictest containment bound over all candidate
polytopes, and "carry the tension openly."

That recommendation stands, and this measurement sharpens *why* rather
than overturning it. The amendment is:

1. **The tension is real in the mathematics and small-amplitude in the
   sampled dynamics.** The containment requirement constrains an exact
   support boundary whose relative amplitude fell below $10^{-17}$ at one
   sampled time. That does not make the alternative causal declarations
   empirically equivalent in principle; they already differ at $t=1$.
2. **The min-containment justification is a conservatism argument, not a
   forcing argument** — which the doc already says by tagging the result a
   motivated `[IMPOSED]`, and which this makes precise. Containment is
   satisfied with enormous margin under every reading; what actually
   shapes the observable causal surface is the *dispersion*, and the
   dispersion is isotropic to $O(k^4)$ by construction.
3. **Scope guard, added.** The above holds with the operator fixed.
   Changing the *stencil* (options A and B in that document's comparison
   table) changes the dispersion, the CFL bound and the isotropy order
   simultaneously, and none of this analysis transfers to that case. The
   7-point stencil in particular is anisotropic already at $O(k^4)$, so
   its front would not be spherical at all.

No constant changes; `C_SPEED = 1/√3` stands where it stood.

## 6. What this does not establish

It is a **one-body** result. It concerns the propagation of a single free
excitation from a single event, which is the sector already closed by the
free-sector calculation ($|\Delta v/v| = (ka)^4/3240$); this is an
independent cross-check of that closure by a different method, not new
territory. The outstanding obligation is the **two-body** one — whether
the *comparison* of two separated clocks is boost-covariant — and that
remains `[OPEN]` and clock-gated, exactly as `DERIV_TWO_OWED_PROOFS_v1.md`
§2.7 records. An isotropic one-body cone is necessary for that, not
sufficient.

Nor does it bear on the Bell barrier, which is where the preferred-
foliation debt actually concentrates.

## 7. Reproduction

```
python scripts/experiments/temporal_interior/probe_causal_isotropy.py
```

A few minutes; deterministic (the source is a point). The $t=36$ arm
allocates a $109^3$ grid.
