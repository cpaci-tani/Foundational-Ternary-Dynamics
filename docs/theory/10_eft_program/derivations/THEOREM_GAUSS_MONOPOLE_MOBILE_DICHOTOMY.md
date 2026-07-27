# Theorem — Gauss-Monopole / Mobile-Dressing Dichotomy

**Record:** FTD-0563  
**Status:** [THEOREM — SELECTED MATCHED-FACE GAUSS SECTOR] + [THEOREM — RESTRICTED NATIVE LINEAR-RESPONSE COROLLARY] + [CLOSED NEGATIVE — FIXED FINITE RIGID LINEAR MONOPOLE CARRIER]  
**Scope:** finite site sources; selected oriented-face Gauss complex; restricted
native FTD-0429 linear response; production linear moving-source operator from
FTD-0558--0562. A common native matter--field action is not assumed or proved.

## 1. Periodic compatibility theorem

For the matched oriented-face divergence

\[
 (D E)(x)=\sum_{i=1}^3[E_i(x)-E_i(x-\hat i)],
\]

periodicity gives

\[
 \sum_x(D E)(x)=0
\]

by exact telescoping. Hence the periodic Gauss equation

\[
 D E=z\rho
\]

has the necessary solvability condition

\[
 \boxed{Q:=\sum_x\rho_x=0.}
\]

The production projector satisfies this condition by replacing `s` with
`s-Q/N`. Thus a periodic apparent point charge is a point source plus a
uniform compensating background. It can approximate a locally isolated source
on scales much smaller than the volume, but it is not globally charged.

The selected matched minimum-energy solver instead rejects non-neutral integer
sources. These are two implementations of the same zero-mode obstruction, not
evidence that net charge has emerged on the torus.

## 2. Open/infinite longitudinal solution

For a finite source, define

\[
 S(\mathbf k)=\sum_x\rho_xe^{i\mathbf k\cdot x},\qquad S(0)=Q,
\]

and the exact face symbols

\[
 d_i(\mathbf k)=1-e^{-ik_i},\qquad
 \lambda(\mathbf k)=\sum_i|d_i|^2.
\]

The minimum-energy longitudinal face field is

\[
 E_i(\mathbf k)=z\frac{d_i^*(\mathbf k)}{\lambda(\mathbf k)}S(\mathbf k).
\]

It obeys the exact identity

\[
 \boxed{\sqrt\lambda\,|E|=z|S|.}
\]

Since `sqrt(lambda)=kappa+O(kappa^3)` along
`k=kappa n`,

\[
 \boxed{\lim_{\kappa\to0}\kappa|E(\kappa\mathbf n)|=z|Q|.}
\]

Therefore the true longitudinal monopole coefficient is exactly the net site
polarity. If `Q=0` and the first nonzero moment has total order `m>=1`, then

\[
 S(\kappa\mathbf n)=\kappa^mP_m(\mathbf n)+O(\kappa^{m+1}),
\]

so the monopole estimator falls as `kappa^m`; there is no `1/kappa` field
singularity and no `1/r^2` Gauss tail.

## 3. Solenoidal additions do not manufacture charge

For every regular localized face field `C B` in the image of the matched curl,

\[
 D(CB)=0.
\]

Its flux through every contractible closed surface is therefore zero. Adding
it changes neither enclosed charge nor the longitudinal infrared coefficient.
A constant harmonic torus flux can pass through a non-contractible cycle, but
it also has zero local divergence and is not a local monopole.

## 4. Restricted native-response corollary

FTD-0429 derives, only in its frozen restricted linear sector,

\[
 Z(\mathbf k)=\frac{(\operatorname{div}J)_{\mathbf k}}{S(\mathbf k)}
 \longrightarrow 3G_C
\]

as `k -> 0`. Because this susceptibility is finite and nonzero, a neutral
finite source with `S=O(kappa^m)` also has
`(div J)=O(kappa^m)`. The restricted native response cannot turn a finite
neutral source into a nonzero Gauss monopole. This is not a proof about an
unknown nonlinear native carrier.

## 5. Conditional mobile-carrier dichotomy

Consider a candidate that identifies one fixed finite source profile with both
the Gauss source above and the rigid source driving the production linear field
operator:

- If `Q=0`, the candidate has no true Gauss monopole.
- If `Q!=0`, its open/infinite static field has a monopole, but FTD-0561 gives
  universal slow-hop forcing proportional to `|Q|T^-2`, and FTD-0562 proves
  that no nonzero fixed finite rigid profile cancels the complete slow-hop
  resonance surface. Its exactly co-moving linear dressing is not square
  summable.

Hence

\[
 \boxed{\text{fixed finite rigid linear carrier}
 \;\not\Rightarrow\;
 \text{true Gauss monopole plus exact radiationless co-motion}.}
\]

The closure is conditional on using the same finite source in the selected
Gauss sector and production linear source sector. FTD has not yet derived the
common native action that would force that identification.

## 6. What remains open

The theorem does not exclude effective long-range charge produced by a
nonlinear field self-source, a singular or topological defect, boundary charge,
nonlocal support, or an internally deforming self-consistent matter--field
orbit. Any such construction must explicitly show where its nonzero closed
flux enters Gauss's law; microscopic neutrality by itself cannot supply it.

