# FTD-0925 — Radius-two causal ternary bridge scaffold and affine `C4` field v1

**Identifier:** `FTD-0925`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** exact central-current network representation, minimum causal support
radius, a fixed radius-two equal-channel current, ternary live-current
compilation, four-tick subcell-remainder return, static-source residue, and an
affine static-plus-evanescent field orbit; no numerical search, fit, engine
mutation, ontology purchase, or `G*`/Born/Bell read

## 1. Question

FTD-0924 found the exact compact current for the ternary-dipole `C4` orbit,
but concentrated it at one void site with unit-tick speed squared `8`. Can
the same continuity transfer be distributed over a finite ternary scaffold
whose live current `j=s v` respects the selected production bandwidth? If so,
what is the minimum support radius, can the stored subcell remainder return
without moving any manifested site, and how must the scaffold's unavoidable
static electric source enter the field orbit and energy ledger?

The test must not call a prescribed velocity sequence autonomous. It may
establish exact reference hardware and identify the remaining generator debt.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| corrected-typesetting `THEOREM_TERNARY_DIPOLE_C4_CENTRAL_CONTINUITY_BRIDGE_CURRENT_AND_PRODUCTION_HUB_BOUNDARY_v1.md` | `0185C438DDB9CB5E061B54C2E1D20260615E367AF829314B1D2AA18C13803E94` |
| `proof_ternary_dipole_c4_central_continuity_current_hub_boundary.py` | `872EF5FAD66E3020A1586F7C0BD66E175ED2B3A38AE5BFB2D420443402FC40E2` |
| `THEOREM_NATIVE_HODGE_ENERGY_CONTINUITY.md` | `7849BFF214225723BFA52EA9034C34B22B94D204A78BE1D6DC6F97D065222868` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/src/render_bridge_phases/phase_movement.cpp` | `6149B37C5A28B8EE9B8544CAEC24006D0964D1C8F344CA63C68DC6536A47E8FB` |
| `engine/include/ftd/causal_kinematics.h` | `705501451985333D64128A0896216A137A2D836673AEB02E9ACE6DE4F2E53AA2` |
| `engine/include/ftd/field_operators.h` | `25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48` |

The certificate fails closed on source drift.

## 3. Frozen central-flow representation

For a site current component define the edge flow

\[
 f_i(c)={1\over2}j_i(c).
\]

It is the oriented edge on the parity sublattice joining `c-e_i` to
`c+e_i`. Its contribution to central divergence is `+f_i(c)` at the first
endpoint and `-f_i(c)` at the second. Thus

\[
 s_{n+1}-s_n+D j_n=0
\]

is exactly a unit network-flow problem on two independent parity classes.

For arm zero, `-delta s_0=s_0-s_1`: one unit flows from `+e_x` to `-e_x`,
and one unit flows from `-e_y` to `+e_y`.

The exact first-moment identity

\[
 \sum_x j_0(x)=-\sum_x x\,[-\delta s_0(x)]
 =2(e_y-e_x)
\]

must be verified. If `N` current sites satisfy `|j(x)|<1/sqrt(3)`, then

\[
 N>2\sqrt6,
\]

so every causal current has at least five nonzero sites. This is a global
cardinality lower bound, not the expected sharp bound.

## 4. Frozen radius ladder

Use current-center Chebyshev radius

\[
 R=\max_{j(x)\ne0}\|x\|_\infty.
\]

- At `R=0`, continuity uniquely fixes the FTD-0924 point current and speed
  squared `8`.
- At `R=1`, the parity-flow graph has no alternative route connecting either
  source/sink pair after its direct central edge is removed. The exact cut
  therefore still fixes `j_x(0)=-2` and `j_y(0)=2`, independent of any
  divergence-free current on the remaining radius-one sites.
- `R=2` is tested by the fixed construction below.

The registered causal-radius verdict is

\[
 R_{\min}=2
\]

if and only if the exact radius-one cut and the complete radius-two
construction both pass.

## 5. Frozen five-channel construction

For a unit flow from a vertex `a` to `a-2e_x`, use five paths:

1. the direct edge; and
2. four shortest three-edge detours through `a+2d`, where
   `d` is `+e_y`, `-e_y`, `+e_z`, or `-e_z`.

For the opposite `y` transfer use the direct edge and detours through
`d in {+e_x,-e_x,+e_z,-e_z}`. Every path carries flow `1/5`, so every scalar
edge-current component has magnitude `2/5`.

The certificate must construct the current by oriented path incidence, not by
hard-coding its final divergence. It must prove:

\[
 \operatorname{supp}j_0=19,
 \qquad
 j_{n+1}=S j_n,
 \qquad
 j_{n+2}=-j_n,
\]

and all four exact continuity equations.

The largest vector currents occur where two orthogonal path components share
a site:

\[
 \boxed{\max_x|j_n(x)|^2={8\over25}<{1\over3}},
 \qquad
 {1\over3}-{8\over25}={1\over75}.
\]

## 6. Frozen shortest-channel minimax theorem

Within the registered five-shortest-channel family, allow arbitrary
nonnegative path weights summing to one for each parity flow. Let `m` be the
largest Euclidean edge-flow norm at a shared current center. The overlap graph
decomposes into three matched pairs and one complete `K_(2,2)` block. Exact
Cauchy bounds give

\[
 2\le3\sqrt2\,m+2\sqrt2\,m=5\sqrt2\,m,
\]

so

\[
 m\ge{\sqrt2\over5}.
\]

Equality requires all ten path weights to equal `1/5`. Hence the registered
construction uniquely minimizes peak speed in this channel family and its
peak current is `2m=2sqrt(2)/5`.

This is not a global cardinality or all-radius-two minimax theorem. The
certificate must preserve that scope ceiling.

## 7. Frozen neutral ternary scaffold

The 19 current centers form these `C4` orbits:

- origin, size one;
- four `xy` diagonals;
- four `xy` radius-two axial sites;
- four sites in the `z=+1` side ring;
- four sites in the `z=-1` side ring; and
- the two fixed sites `+2e_z` and `-2e_z`.

Any ternary assignment nonzero on all 19 sites has odd total charge and cannot
be neutral. Add the zero-current fixed-axis site `+e_z`. The registered
20-site scaffold `h` assigns `+1` to the origin, the `xy` diagonals, the
radius-two `xy` axes, and `+2e_z`; it assigns `-1` to both side rings,
`-2e_z`, and the neutralizer `+e_z`.

The certificate must prove that `h` is ternary, `C4` invariant, neutral,
disjoint from every dipole endpoint, nonzero on all current centers, and
minimum neutral cardinality for this fixed 19-site current support.

Set

\[
 r_n=h+s_n,
 \qquad
 v_n(x)=\begin{cases}j_n(x)/h(x),&x\in\operatorname{supp}j_n,\\0,&\text{otherwise}.
 \end{cases}
\]

Then `r_n` is ternary and `r_n v_n=j_n` exactly. This compiles the bridge
through the existing live-current product without adopting a new current
type.

## 8. Frozen remainder/no-hop orbit

Starting from zero subcell remainder, prescribe the four velocities in the
order `v_0,v_1,-v_0,-v_1`. The certificate must evaluate every scaffold site
and every component after each partial sum. It must prove

\[
 \max|v_0|_{\rm component}\le{2\over5},
 \qquad
 \max|v_0+v_1|_{\rm component}\le{4\over5}<1,
\]

and exact return to zero after four ticks. Therefore the frozen production
movement threshold is never crossed and the manifested scaffold need not hop
while carrying the internal current.

This is a kinematic reference orbit. The certificate must leave the law that
updates `v_n` autonomously open.

## 9. Frozen static-source and affine-field split

No nonzero compact scalar scaffold has zero central gradient: in the Laurent
domain, `d_i h=0` for one nonzero central symbol already forces `h=0`.
Therefore the neutral `h` has an unavoidable static source

\[
 U_h=-\nabla_c h\ne0.
\]

It is `C4` invariant. The certificate must report its exact finite support and
norm but attach no fitted significance to those values.

For the energy-centered rotating source define

\[
 q_n=\nabla_c\bar s_n-\operatorname{curl}_c j_n,
 \qquad F_n=(2I-K)^{-1}q_n.
\]

Let `H` be the unique zero-mean periodic/static-range solution of

\[
 K H=U_h.
\]

Modewise existence follows because `U_h` has zero mean and the C18 stiffness
has only its constant zero mode. On the uncontained lattice the registered
Fourier power count must show `H in ell^2` in three dimensions; no exponential
tail is claimed for `H`.

Set

\[
 J_n=H+F_n,
 \qquad P_n=F_n+F_{n+1},
\]

and use total source

\[
 U_n=K H+(K-2I)F_n.
\]

The certificate must prove the exact affine kick--drift orbit

\[
 (J_n,P_n)\mapsto(J_{n+1},P_{n+1}).
\]

The static invariant representation is orthogonal to the rotating `C4`
doublet. Consequently the FTD-0576 midpoint field, interaction, and required
matter-reaction work must each vanish on every ideal arm. The static field
stores positive energy

\[
 E_h={1\over2}\langle H,K H\rangle>0,
\]

but its formation cost and the scaffold's own positive Hamiltonian remain
open.

## 10. Outcome rules

- **Outcome A — causal radius-two affine reference scaffold:** the radius-one
  cut, 19-site current, `8/25` speed bound, five-channel minimax theorem,
  20-site neutral ternary compilation, no-hop remainder return, unavoidable
  static source, affine field orbit, and ideal zero-work ledger all pass.
- **Outcome B — algebraic but noncausal scaffold:** continuity closes but the
  exact bandwidth or no-hop gate fails.
- **Outcome C — no registered radius-two scaffold:** path incidence,
  continuity, covariance, or live-current compilation fails.
- **Outcome D — invalid execution:** any source lock, exact count, production
  marker, or scope firewall fails. Book no theorem.

## 11. Required certificate gates

The certificate must cover:

1. all frozen hashes;
2. exact central-divergence/edge-incidence equivalence and first moment;
3. the global five-site lower bound and exact radius-zero/radius-one cut;
4. explicit five-path incidence for both parity transfers;
5. 19-site support, `C4` covariance, antipodes, and four continuity arms;
6. exact peak `8/25`, causal margin `1/75`, and channel-family minimax proof;
7. 19-site odd-neutrality obstruction and explicit 20-site neutral scaffold;
8. exact `r_n v_n=j_n` live compilation;
9. all per-site four-tick remainder partial sums and no-hop return;
10. static-gradient nonzero/support/norm and compact gradient-null no-go;
11. dynamic curl/source covariance, affine field orbit, and ideal work ledger;
12. positive static field energy conditional on the static solve;
13. production source, speed projection, movement threshold, and kick--drift
    markers;
14. unchanged engine/type/import status; and
15. no `G*`, gamma, Born/Bell, context, measurement, fit, sweep, near-miss, or
    formula-substitution read.

## 12. Frozen scope ceiling

Success does not derive the velocity update, source recoil, a positive
scaffold Hamiltonian, formation, reset, perturbation recovery, mobility,
physical scale, global cardinality minimum, all-radius-two minimax optimum,
`G*`, gamma, Born frequencies, Bell correlations, measurement context, or
preferred-tick hiding. The equal-channel current and ternary sign pattern are
reference selections inside the existing types, not new axioms and not
production evidence.
