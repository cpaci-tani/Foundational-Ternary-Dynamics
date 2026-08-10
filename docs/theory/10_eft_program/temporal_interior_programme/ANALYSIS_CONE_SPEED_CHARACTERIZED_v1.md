# ANALYSIS — The Cone Speed, Characterized (and the Tension It Exposes)

**Status:** `[DERIVED — TWO EXACT CHARACTERIZATIONS OF 1/√3]` +
`[CLOSED NEGATIVE — 1/√3 IS NOT THE STABILITY BOUND]` +
`[OPEN — FORCING REQUIRES A PRIOR CHOICE THAT CONTRADICTS P4]` +
`[AMENDED 2026-08-08 — THE TENSION IS DYNAMICALLY EMPTY; see §7]` +
`[BOOKED — FTD-0810]`
**Date:** 2026-08-08 · **Artifact:** `scripts/experiments/temporal_interior/derive_cone_speed.py`
**Origin:** the owner's proposal that the cone speed follows from how
influence spreads — "you can either have a square cone spread like a
pyramid or a cone… the square doesn't leave area unconsidered."
**Parents:** FTD-0407 (`C_SPEED = 1/√3` `[SELECTION]`),
`AUDIT_LORENTZ_RECOVERY_HARD.md`, `DERIV_TWO_OWED_PROOFS_v1.md` §2.7.
**Production impact:** none. No constant is changed; no tag moves.

---

## 1. What was wanted

`C_SPEED = 1/√3` is a declared selection: the corpus records that it is
*not* CFL-forced and that the production stencil permits `c ≤ √3/2`. The
question is whether a geometric principle about causal spreading forces
it.

## 2. The stability bound, exactly (for contrast)

For the M18 symbol `L(k)`, writing `u_i = cos k_i`, the extremum over the
Brillouin zone sits at the cube corners of `u`. Exhaustively:

| `k` | `L` |
|---|---|
| $(\pi,\pi,0)$ and permutations | $-16/3$ ← minimum |
| $(\pi,\pi,\pi)$; $(\pi,0,0)$ and perms | $-4$ |
| $(0,0,0)$ | $0$ |

So `max|L| = 16/3`, attained on the **face diagonal**, and leapfrog
stability `(C/2)√(max|L|) ≤ 1` gives
$$C \le \tfrac{\sqrt3}{2} = 0.866025\ldots$$
confirming that `1/√3 = 0.577350…` is **not** the stability saturation.

## 3. The causal polytopes, and the containment principle

In one tick influence reaches a finite set of sites; its convex hull is
the **causal polytope**, and after `n` ticks the reachable region is the
`n`-fold dilation. An isotropic light cone of speed `c` remains inside
that region for all time **iff `c ≤` the polytope's inradius**. Computed
exactly:

| neighbourhood | polytope | inradius |
|---|---|---|
| von Neumann (6) | octahedron | $1/\sqrt3 = 0.577350$ |
| M18 face+edge (18) | cuboctahedron | $1$ |
| Moore (26) | cube | $1$ |

> **Result.** $1/\sqrt3$ is *exactly* the largest isotropic speed whose
> light cone is contained in the **octahedral** causal cone. Under
> cuboctahedral or cubic causality the containment bound is $1$.

## 4. The $1/\sqrt D$ coincidence — the owner's "Gaussian spread"

Two apparently different quantities are the same number:

- the octahedron's inradius at unit circumradius: its facets are
  $\sum_i \pm x_i = 1$, at distance $1/\sqrt D$ from the origin;
- the per-axis component of an isotropic unit spread:
  $\sigma_{\rm tot}^2 = \sum_i \sigma_i^2 = D\sigma^2 = 1
  \Rightarrow \sigma = 1/\sqrt D$. Equivalently, the RMS component of a
  uniformly random unit vector along a fixed axis is $1/\sqrt D$, since
  $\mathbb{E}[n_i^2] = 1/D$.

Both distribute one isotropic unit over $D$ orthogonal axes, which is why
they agree; at $D=3$ both give $0.577350$. This is the precise content of
the "Gaussian spread" reading, and it is a genuine identification rather
than a numerical coincidence.

*A refinement of the owner's framing.* The two halves of the proposal
point in different directions. The **cube/pyramid** reading — the causal
region as a box that "leaves no area unconsidered" — gives inradius $1$,
i.e. $c = 1$, not $1/\sqrt3$. It is the **per-axis / Gaussian** reading
that yields $1/\sqrt3$, and it does so exactly. The instinct that the
answer is about how an isotropic spread decomposes over the axes is
correct; the polytope that realizes it is the octahedron, not the cube.

## 4a. The origin of the constant, found

*(Added 2026-08-08; `compare_cone_speed_options.py`.)*

For the **7-point (von Neumann) Laplacian**, $L_7 = 2\sum_i\cos k_i - 6$
attains $\max|L_7| = 12$ at $(\pi,\pi,\pi)$, so leapfrog stability gives
$$C \le 2/\sqrt{12} = 1/\sqrt3 .$$
That is the *same* number as the octahedral containment bound. **For the
von Neumann stencil, stability and causal containment coincide exactly at
$1/\sqrt3$** — the constant is doubly forced there, and this is the
textbook 3-D CFL condition.

The engine, however, runs the **18-point** Laplacian, whose stability
bound is $\sqrt3/2$. So $1/\sqrt3$ is a **legacy of the 7-point scheme**,
retained when the stencil was upgraded to buy $O(k^4)$ isotropy. That
fully explains its "selection" status: it was forced, under a stencil no
longer in use.

Two exact relations worth recording:
$$\frac{1/\sqrt3}{\sqrt3/2} = \frac23 \quad\text{exactly},$$
so the engine runs at two-thirds of its stability limit — a conventional
safety factor; and
$$1/\sqrt3 = \min\big\{\text{containment bounds over all candidate
polytopes}\big\},$$
since the octahedron gives $1/\sqrt3$ and both the cuboctahedron and the
cube give $1$.

**The isotropy/forcing trade-off.** The two desiderata pull apart:

| stencil | leading anisotropy | CFL bound | own containment bound | binding |
|---|---|---|---|---|
| 7-point | $O(k^4)$, **anisotropic** | $1/\sqrt3$ | $1/\sqrt3$ (octahedron) | both, $=1/\sqrt3$ |
| 18-point | $O(k^4)$ **isotropic**; anisotropy at $O(k^6)$ | $\sqrt3/2$ | $1$ (cuboctahedron) | CFL, $=\sqrt3/2$ |

The stencil that forces the cone speed is the one whose anisotropy is
bad; the stencil with good isotropy does not force the cone speed. One
cannot have both from this pair.

## 5. Verdict: characterized, not discharged — and a tension exposed

$1/\sqrt3$ now has an exact geometric meaning where before it had none.
But forcing it requires adopting **octahedral (6-neighbour) causality**,
and:

- the production wave operator is the **18-point** stencil, whose reach
  is the cuboctahedron (inradius 1);
- **P4 commits the framework to Moore (26) causality**, whose polytope is
  the cube (inradius 1).

Under either of the framework's own structures the containment bound is
$1$, and $1/\sqrt3$ is *not* forced — it is a strictly interior choice.

> **The selection therefore reduces to a prior selection: which
> neighbourhood defines causal reach.** That is a decidable
> constitutional question rather than a free numerical parameter, which
> is a real improvement on its previous status, but it is not a
> derivation of the constant.

Three ways the item can now be closed, each with a stated price:

1. **Adopt octahedral causality.** Then $1/\sqrt3$ is forced exactly.
   Price: an amendment to P4, and an account of why the 18-point
   Laplacian may exceed the causal polytope (numerical precursors would
   have to be argued unphysical).
2. **Keep Moore/cuboctahedral causality and saturate.** Then $c = 1$ is
   the containment-forced value, $\sqrt3/2$ the stability-forced one, and
   whichever is adopted the present constant must change — with every
   downstream number that depends on it, including the band top
   $2\arcsin C$ and the anisotropy coefficient.
3. **Keep $1/\sqrt3$ as a declared interior selection**, now with its
   exact characterization recorded, and carry the tension openly.

The third is the status quo and is what this document leaves in place.
**No constant is changed here.**

## 6. Comprehensive comparison

Computed downstream consequences (`compare_cone_speed_options.py`). Note
first that **$C=1$ is dynamically forbidden**: it exceeds the M18
stability bound, so "saturate cubic containment" is not an available
option. Only two saturating values exist.

| | **A.** 7-point, $C=1/\sqrt3$ | **B.** 18-point, $C=\sqrt3/2$ | **C.** 18-point, $C=1/\sqrt3$ *(status quo)* |
|---|---|---|---|
| cone speed forced? | **yes, doubly** (CFL $=$ containment) | yes, by CFL | not by its own CFL; yes as the strictest containment bound |
| leading anisotropy | $O(k^4)$ — **bad** | $(ka)^4/3240$ | $(ka)^4/3240$ |
| stability margin $C/C_{\rm CFL}$ | $1.00$ (marginal) | $1.00$ (marginal) | $\mathbf{2/3}$ |
| light cone inside octahedron? | yes | **no** | **yes** (strictest reading) |
| axis band top $2\arcsin C$ | $1.2310$ | $2.0944$ | $1.2310$ |
| full band top | $\pi$ (marginal) | $\pi$ (marginal) | $1.4595$ |
| dispersion coeff $(C^2-1)/24$ | $-0.0278$ | $-0.0104$ | $-0.0278$ |
| MVC clock: $\varepsilon$ needed at $A_{\max}{=}0.5$ | $4.22$ | $\mathbf{12.22}$ | $4.22$ |
| tick $t_{\rm phys}$ | $3.11\times10^{-44}$ s | $4.67\times10^{-44}$ s | $3.11\times10^{-44}$ s |
| migration cost | rewrite the Laplacian; **lose isotropy** | every golden; every dimensional prediction $\times 3/2$ | **none** |

Three facts settle the ranking.

1. **The anisotropy coefficient is exactly $C$-independent** — all three
   columns give $-1/3240$. The free-sector Lorentz result of §2 does not
   depend on this choice at all, so nothing already derived is at stake.
2. **Raising $C$ makes the clock strictly harder.** The C2 band-clearance
   requirement grows as $(2\arcsin C)^2$, so option B nearly triples the
   already-unaffordable $\varepsilon$. It buys a forced constant by
   worsening the programme's binding open problem.
3. **Options A and B both sit exactly on the stability edge.** Marginal
   stability is not a safe operating point for a nonlinear code; the
   status quo's factor of $2/3$ is the conventional margin.

**Recommendation: retain $1/\sqrt3$, and upgrade its justification rather
than its value.** It is not an arbitrary interior point. It is
$$1/\sqrt3 = \min\{\text{containment bounds over every candidate causal
polytope}\},$$
so the light cone is contained in the causal region under *all* readings
— octahedral, cuboctahedral and cubic — and no commitment about which
neighbourhood is causally fundamental has to be made. It simultaneously
carries a two-thirds stability margin and preserves the $O(k^4)$
isotropy that the 18-point stencil was adopted for. The honest label is
therefore not `[SELECTION]` but **the strictest causal containment
bound, polytope-independent** — a motivated `[IMPOSED]` in the sense the
project already endorses, with its falsifier being any demonstration that
a coarser causal polytope is the physically correct one.

---

## 7. Amendment, 2026-08-08 — the tension is real but dynamically empty

*Added after `ANALYSIS_CAUSAL_ISOTROPY_SCALING_v1.md`
(`scripts/experiments/temporal_interior/probe_causal_isotropy.py`) measured what the
competing polytopes actually control. The §5 verdict and the §6
recommendation both stand; this sharpens why, and adds a scope guard.*

The tension of §5 is a disagreement about **which boundary is the causal
one**. The measurement asks what the field does at that boundary, and the
answer removes the empirical stakes.

**(a) The observable causal surface is set by the dispersion, not the
polytope.** The strict support is a cuboctahedron, anisotropic by $34.3\%$
between the face and edge directions. The field's own causal surface is
not: at the signal front it is a sphere to $0.71\%$ at $t=24$, and even
fifteen decades down in amplitude it has reached only $7.2\%$ — a factor
of five short of the stencil's own shape, which is therefore never
attained at any measurable amplitude. Isotropy moreover *improves* with
elapsed time, as $A(t)\sim t^{-1.1}$ to $t^{-1.3}$, consistent with the
Airy-front prediction $t^{-4/3}$.

**(b) The disputed boundary is weak in the sampled profile.** The
relative amplitude at the strict support edge falls **1.19 decades per
tick** — $2.5\times10^{-7}$ at $t=8$, $2.7\times10^{-17}$ at $t=16$,
$2.0\times10^{-26}$ at $t=24$ — crossing the double-precision relative
spacing near unity at about sixteen ticks ($\sim5\times10^{-43}$ s under
the stated calibration). This is not a representability limit: binary64
can store values far below $10^{-26}$. Holding the operator fixed at M18,
the declared causal neighbourhood changes where the field's exact zero
begins; the finite-time profile shows strong suppression there, not
dynamical inaccessibility.

**Consequence for §5.** Options 1 and 2 there are presented as live
alternatives with stated prices. They remain mathematically distinct — the
containment bound genuinely differs by reading, and $1/\sqrt3$ genuinely
is forced only under the octahedral one — and are exactly distinguishable
from support (already at $t=1$). The reported run does not determine
whether that distinction is physically observable at later calibrated
times. The §6 recommendation (retain $1/\sqrt3$; label it a motivated
`[IMPOSED]` justified as the strictest containment bound) is unaffected;
the sampled suppression is evidence about effective contours, not a
forcing argument.

**Scope guard — this amendment does not transfer to a stencil change.**
Everything above holds with the *operator* fixed at M18 and only the
*declared causal neighbourhood* varying. Options A and B of the §6
comparison table change the operator, and therefore change the dispersion,
the CFL bound and the isotropy order together. The 7-point stencil is
anisotropic already at $O(k^4)$, so its signal front would not be
spherical at all and none of the isotropy measurements above would carry
over. The comparison table's ranking is untouched.

**Still one-body.** This is the propagation of a single free excitation
and is an independent cross-check of the free-sector closure, not new
territory. The two-body obligation (`DERIV_TWO_OWED_PROOFS_v1.md` §2.7)
is untouched and remains `[OPEN]` and clock-gated.
