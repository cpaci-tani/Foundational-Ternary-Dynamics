# Theorem — Radius-two causal ternary bridge scaffold, affine `C4` field, and autonomy boundary v1

**Identifier:** `FTD-0925`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — MINIMUM CAUSAL CURRENT-CENTER RADIUS TWO]` +
`[REFERENCE CONSTRUCTION — 20-SITE NEUTRAL TERNARY LIVE-CURRENT SCAFFOLD]` +
`[THEOREM — EXACT FOUR-TICK NO-HOP REMAINDER RETURN]` +
`[REFERENCE CONSTRUCTION — AFFINE STATIC-PLUS-EVANESCENT C4 FIELD ORBIT]` +
`[THEOREM — ZERO IDEAL FIELD/INTERACTION/MATTER WORK]` +
`[OPEN — AUTONOMOUS VELOCITY GENERATOR/SCAFFOLD HAMILTONIAN/FORMATION]`

## 1. Result

The exact FTD-0924 bridge current can be compiled through the existing
ternary state and velocity types without violating the selected production
speed bound.

The construction splits each of the two parity-sublattice transfers among
five paths: one direct path and four shortest transverse detours. Giving each
path flow `1/5` produces a `C4`-covariant current supported on 19 sites inside
Chebyshev radius two. Its exact peak speed is

\[
 \boxed{\max_x|v_n(x)|^2={8\over25}<{1\over3}},
 \qquad
 {1\over3}-{8\over25}={1\over75}.
\]

Radius two is minimum. With current centers restricted to radius zero or one,
the direct central edge is the only parity-graph connection between each
source/sink pair, so continuity still fixes the FTD-0924 center current
`(-2,2,0)` and speed squared eight.

The 19 current sites cannot themselves carry a neutral ternary assignment
because their cardinality is odd. One additional zero-current site on the
rotation axis gives a 20-site neutral, `C4`-invariant ternary scaffold `h`.
For

\[
 r_n=h+s_n,
 \qquad v_n={j_n\over h}
\]

on the current support, the unchanged live product closes exactly:

\[
 \boxed{r_nv_n=j_n},
 \qquad
 \boxed{r_{n+1}-r_n+\operatorname{div}_cj_n=0}.
\]

The velocity sequence is pointwise `v_0,v_1,-v_0,-v_1`. Starting from zero
subcell remainder, every component remains strictly between `-1` and `+1`
and the remainder returns exactly to zero after four ticks. The scaffold can
therefore carry the prescribed internal current without any manifested site
crossing a lattice-cell boundary.

The remaining cost is exact. Every nonzero compact scalar scaffold has
nonzero central gradient. The neutral `h` adds a static electric source with
vector support 45 and norm squared 22. The complete field clock is therefore
an affine body: a positive-energy static `ell^2` field halo plus the rotating
evanescent `C4` doublet. The ideal FTD-0576 field, interaction, and
matter-reaction work remain zero on every arm.

This is causal continuity-compatible reference hardware, not yet an
autonomous clock. The velocity update, formation of the 20-site scaffold,
its own positive Hamiltonian, reset, and perturbation recovery remain open.

## 2. Central current as a parity-sublattice flow

For a current component at center `c`, define

\[
 f_i(c)={j_i(c)\over2}.
\]

The central derivative gives

\[
 D_ij_i(c-e_i)=+f_i(c),
 \qquad
 D_ij_i(c+e_i)=-f_i(c).
\]

Thus `f_i(c)` is an oriented edge joining `c-e_i` to `c+e_i` on one of the
eight site-parity sublattices. Central continuity is exactly a network-flow
equation on those graphs.

For the first dipole transition,

\[
 -\delta s_0=s_0-s_1.
\]

Its `x`-parity component is a unit flow from `+e_x` to `-e_x`; its
`y`-parity component is a unit flow from `-e_y` to `+e_y`.

The exact first-moment identity is

\[
 \sum_xj_0(x)
 =-\sum_xx[-\delta s_0(x)]
 =2(e_y-e_x).
\]

Therefore

\[
 \left|\sum_xj_0(x)\right|=2\sqrt2.
\]

If `N` sites all obey the strict flat bound `|j(x)|<1/sqrt(3)`, the triangle
inequality gives

\[
 2\sqrt2<{N\over\sqrt3}.
\]

Hence every causal current has at least five nonzero sites. This is a global
lower bound, not the sharp cardinality theorem.

## 3. Exact minimum radius

Define current-center radius

\[
 R=\max_{j(x)\ne0}\|x\|_\infty.
\]

At `R=0`, continuity gives the unique point current

\[
 j_0(0)=2(e_y-e_x),
 \qquad |j_0(0)|^2=8.
\]

At `R=1`, remove the direct `x` edge centered at the origin. Exact graph
enumeration shows that `+e_x` and `-e_x` lie in disconnected components; the
only cut edge is the removed central edge. The same statement holds for the
`y` transfer. Summing continuity over either cut therefore fixes

\[
 j_x(0)=-2,
 \qquad j_y(0)=2
\]

even after adding arbitrary divergence-free radius-one currents. Radius one
still has speed squared eight and is noncausal.

Section 4 constructs a causal radius-two current. Therefore

\[
 \boxed{R_{\min}=2.}
\]

This minimum concerns the maximum distance of current centers from the
rotation center under the frozen central operator and unit-tick bandwidth.

## 4. Five-channel radius-two current

For the `x`-parity flow, use the direct path from `+e_x` to `-e_x` and four
three-edge paths

\[
 +e_x\to +e_x+2d\to-e_x+2d\to-e_x,
 \qquad
 d\in\{+e_y,-e_y,+e_z,-e_z\}.
\]

For the `y`-parity flow, use the direct path from `-e_y` to `+e_y` and

\[
 -e_y\to-e_y+2d\to+e_y+2d\to+e_y,
 \qquad
 d\in\{+e_x,-e_x,+e_z,-e_z\}.
\]

Each path carries flow `1/5`; the corresponding site-current component has
magnitude `2/5`. Oriented path incidence gives the divergence directly and
the exact certificate independently recomputes it using the central
difference. Both routes give

\[
 \operatorname{div}_cj_0=s_0-s_1.
\]

The combined support consists of:

- the origin;
- four `xy` diagonals;
- four radius-two axial sites in the `xy` plane;
- four side-ring sites at `z=+1`;
- four side-ring sites at `z=-1`; and
- the two fixed sites `+2e_z` and `-2e_z`.

Thus

\[
 |\operatorname{supp}j_0|=19.
\]

At sites where two orthogonal path components meet, the current is

\[
 (\pm2/5,\pm2/5,0)
\]

up to cubic rotation, giving the peak squared norm `8/25`. All other
nonzero sites carry one component of magnitude `2/5`.

Rotating the entire path incidence gives

\[
 j_{n+1}=Sj_n,
 \qquad j_{n+2}=-j_n,
\]

and closes all four continuity equations exactly.

## 5. Exact minimax result in the registered channel family

Allow arbitrary nonnegative weights on the five shortest channels for each
parity flow, with each set summing to one. Let `m` be the largest Euclidean
edge-flow norm at a shared current center.

The shared-site graph decomposes into three matched channel pairs and one
complete bipartite `K_(2,2)` block. For each matched pair, Cauchy gives a
weight sum at most `sqrt(2)m`, hence `3sqrt(2)m` in total. If `A` and `B` are
the largest weights on the two sides of the `K_(2,2)` block, every crossing
obeys `A^2+B^2<=m^2`, so its four weights sum to at most
`2(A+B)<=2sqrt(2)m`.

The two unit-flow normalizations therefore imply

\[
 2\le5\sqrt2\,m,
 \qquad
 \boxed{m\ge{\sqrt2\over5}}.
\]

Equality in every step requires all ten channel weights to be `1/5`. Hence
the equal-channel construction uniquely minimizes peak speed in this frozen
five-shortest-channel family:

\[
 \boxed{|j|_{\max}=2m={2\sqrt2\over5}}.
\]

This does not establish the global minimum support count or minimax optimum
among every possible radius-two network with cycles or longer paths.

## 6. Neutral ternary live-current compilation

Every one of the 19 current centers must be manifested if the current is to
be the existing product `s v`. A nonzero ternary assignment on an odd number
of sites has odd total charge, so those 19 sites cannot be neutral.

Assign `+1` to:

- the origin;
- the four `xy` diagonals;
- the four radius-two `xy` axes; and
- `+2e_z`.

Assign `-1` to:

- both four-site `z=+1` and `z=-1` side rings;
- `-2e_z`; and
- one additional zero-current neutralizer at `+e_z`.

The resulting static field `h` has ten positive and ten negative sites. It is
neutral, ternary, `C4` invariant, disjoint from every rotating dipole
endpoint, and nonzero at every current center. Since the 19-site assignment
cannot be neutral, one additional site is cardinality-minimal for this fixed
current support.

Define

\[
 r_n=h+s_n
\]

and set `v_n=j_n/h` on the 19 current centers and zero at the neutralizer and
dipole endpoints. Because the supports of `h` and `s_n` are disjoint,
`r_n` remains ternary. Directly,

\[
 r_nv_n=hv_n=j_n.
\]

Since `h` is static, the exact live continuity law follows:

\[
 r_{n+1}-r_n+D(r_nv_n)
 =s_{n+1}-s_n+Dj_n=0.
\]

No independent current type is required by this reference construction.

## 7. Four-tick no-hop remainder orbit

The velocity fields obey

\[
 v_2=-v_0,
 \qquad v_3=-v_1.
\]

Starting with zero production remainder, the four partial sums are

\[
 v_0,
 \qquad v_0+v_1,
 \qquad v_1,
 \qquad0.
\]

The certificate exhausts every scaffold site and component and obtains

\[
 \max|v_0|_{\rm component}={2\over5},
 \qquad
 \max|v_0+v_1|_{\rm component}={4\over5}<1.
\]

Therefore none of the production movement tests `remainder>=1` or
`remainder<=-1` fires, and the internal remainder closes after four ticks.
The state scaffold can remain at fixed lattice sites without using the
production `locked` flag.

This result is conditional on the registered velocity sequence being
supplied. Production forces have not been shown to generate or stabilize it.

## 8. Unavoidable static electric source

The live scaffold is actual ternary state, so it enters the same electric
source as every other state:

\[
 U_h=-\nabla_ch.
\]

For a compact Laurent polynomial `h`, the equation `d_xh=0` and the absence
of zero divisors imply `h=0`. Thus no nonzero compact static scaffold can be
invisible to the central gradient.

For the explicit neutral scaffold,

\[
 \boxed{|\operatorname{supp}_v\nabla_ch|=45},
 \qquad
 \boxed{\|\nabla_ch\|_2^2=22}.
\]

This source is `C4` invariant rather than antipodal. The current curl, by
contrast, remains in the rotating doublet. Its exact values are

\[
 |\operatorname{supp}_v\operatorname{curl}_cj_0|=36,
 \qquad
 \|\operatorname{curl}_cj_0\|_2^2={72\over25}.
\]

The static source cannot be discarded as irrelevant background: it must have
its own field response and formation energy.

## 9. Exact affine field orbit

Use the FTD-0576 midpoint source seed

\[
 q_n=\nabla_c\bar s_n-\operatorname{curl}_cj_n,
 \qquad
 F_n=(2I-K)^{-1}q_n.
\]

The seed is compact and satisfies

\[
 q_{n+1}=Sq_n,
 \qquad q_{n+2}=-q_n.
\]

FTD-0923's outside-band theorem therefore gives the same evanescent tail
bound for the rotating component. Let `H` solve the static equation

\[
 KH=U_h=-\nabla_ch.
\]

On a periodic quotient, `U_h` has zero mean and `K` has only the constant zero
mode, so there is a unique zero-mean solution. On the uncontained lattice,
neutrality gives `h_hat(k)=O(|k|)`, the central gradient supplies another
`O(|k|)`, and `K(k)=O(|k|^2)`. Hence `H_hat(k)=O(1)` near zero and
`H in ell^2` in three dimensions. No exponential static tail is claimed.

Define

\[
 J_n=H+F_n,
 \qquad
 P_n=F_n+F_{n+1},
\]

with source

\[
 U_n=KH+(K-2I)F_n.
\]

Then

\[
\begin{aligned}
 P_n-KJ_n+U_n
 &=P_n-2F_n
 =P_{n+1},\\
 J_n+P_{n+1}
 &=H+F_{n+1}=J_{n+1}.
\end{aligned}
\]

Thus

\[
 \boxed{(J_n,P_n)\mapsto(J_{n+1},P_{n+1})}
\]

is an exact affine `C4` orbit: `H` is fixed while `F_n` rotates.

## 10. Energy and work

The static source and field occupy the invariant representation of `C4`; the
rotating source, field, and work-coordinate increments occupy a doublet with
`S^2=-I`. Orthogonality of inequivalent real representations gives

\[
 \langle U_h,F_{n+1}\rangle=0.
\]

The dynamic return work also vanishes by the FTD-0924 self-adjoint/skew
argument. Therefore total field work is zero on every arm.

The complete density and work coordinate both advance by the same orthogonal
action:

\[
 r_{n+1}=Sr_n,
 \qquad R_{n+1}=SR_n,
\]

because `h` and `H` are invariant. The interaction energy

\[
 -\langle r_n,DR_n\rangle
\]

is constant. FTD-0576's exact total-energy identity then fixes the ideal
matter-reaction work to zero as well:

\[
 \boxed{
 \Delta H_f=\Delta U_{\rm int}=\Delta H_m=0}.
\]

The static field has positive stored energy

\[
 E_h={1\over2}\langle H,KH\rangle>0
\]

on the zero-mean range. This is field storage created by the scaffold, not a
derived source battery. The energy required to form `h` and `H` is not paid
by the reference recurrence.

## 11. What dynamics remain missing

FTD-0924's continuity and bandwidth obstruction is now retired for a finite
existing-type reference scaffold. The remaining source-core problem is not
the existence of a causal current. It is the autonomous production of that
current:

1. derive a local reversible update that maps `v_n` to `v_(n+1)`;
2. react the field impulse onto that update without reading the desired arm;
3. give the 20-site scaffold and its static halo a positive formation and
   storage ledger;
4. recover the orbit after admissible perturbations;
5. release/reset it without unbooked erasure; and
6. show the construction can form from production dynamics rather than a
   prescribed initial condition.

The next smallest exact candidate is a local quarter-turn Hamiltonian on the
19 scaffold velocity/remainder pairs, constrained to generate the registered
path-current orbit and to pay the static-halo preparation work. Failure would
not revive the independent-current requirement; it would locate the missing
type in the velocity generator or source Hamiltonian instead.

`G*` remains downstream. The present carrier has an exact four-tick internal
cycle, but no critical-quartic envelope or lemniscatic cadence has been
derived from it.

## 12. Epistemic boundary

The equal five-channel routing and the ternary sign assignment are selected
reference realizations inside existing state/velocity types. They are not
claimed unique among all radius-two currents. The theorem derives minimum
radius and the minimax result only in the frozen five-shortest-channel class.

It does not derive an autonomous velocity law, scaffold Hamiltonian,
formation, source recoil, reset, perturbation recovery, mobility, physical
scale, global minimum site count, all-radius-two minimax optimum, `G*`,
gamma, Born frequencies, Bell correlations, measurement context, or
preferred-tick hiding. It adopts no independent current type and changes no
engine source, CMake target, toggle, default, import, or selected ontology
type.

## 13. Verification

The locked preregistration has SHA-256
`627C6F1583A1E07F03A1BAB01B9C7AA59D670A861DC650FF72CAD8100586EFBE`.

The exact certificate is
`scripts/proofs/proof_radius_two_causal_ternary_bridge_scaffold_affine_c4_field.py`,
SHA-256
`62F7E3B5EA37FD8B00CC736CF2A507260313D8F5724E1A0562CEB4B870F9E1DC`.

It passes `132/132` gates and reports

```text
OUTCOME=A_CAUSAL_RADIUS_TWO_AFFINE_REFERENCE_SCAFFOLD
MINIMUM_CAUSAL_CURRENT_RADIUS=2
CURRENT_SUPPORT=19
NEUTRAL_TERNARY_SCAFFOLD_SUPPORT=20
PEAK_SPEED_SQUARED=8/25
CAUSAL_MARGIN_SQUARED=1/75
FOUR_TICK_REMAINDER_RETURN=EXACT_NO_HOP
STATIC_GRADIENT_SUPPORT=45
STATIC_GRADIENT_NORM_SQUARED=22
DYNAMIC_CURL_SUPPORT=36
DYNAMIC_CURL_NORM_SQUARED=72/25
AFFINE_STATIC_PLUS_EVANESCENT_C4_ORBIT=EXACT
IDEAL_FIELD_INTERACTION_MATTER_WORK=ZERO_EACH_TICK
AUTONOMOUS_VELOCITY_GENERATOR=OPEN
INDEPENDENT_CURRENT_TYPE_ADOPTED=FALSE
PRODUCTION_CHANGED=FALSE
GSTAR_USED=FALSE
BORN_BELL_CONTEXT_USED=FALSE
```
