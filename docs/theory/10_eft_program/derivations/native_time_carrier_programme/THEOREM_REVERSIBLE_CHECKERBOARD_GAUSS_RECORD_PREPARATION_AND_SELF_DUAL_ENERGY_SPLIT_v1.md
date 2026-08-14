# Theorem — Reversible checkerboard Gauss-record preparation and self-dual energy split v1

**Identifier:** `FTD-0881` / repaired execution `FTD-0882`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — CONDITIONAL REVERSIBLE LOCAL RECORD PREPARATION]` +
`[THEOREM — EXACT RETAINED-HISTORY INVERSE]` +
`[THEOREM — REFERENCE FIELD/HISTORY ENERGY EQUALITY]` +
`[CLOSED NEGATIVE — SIZE-INDEPENDENT FINITE EXACT LOCAL PREPARATION]` +
`[SELECTION — EXISTING PHASE-RAIL TYPE]` +
`[OPEN — AUTONOMOUS FRESHNESS/RECYCLING, POSITIVE SOURCE RESERVOIR,
MOVING SOURCES, BOUNDARIES, PRODUCTION, G*]`

## 1. Verdict

FTD-0880 established the static matched Gauss record

\[
 J_s=D^TL^+q,\qquad DJ_s=q,\qquad J_s\in\operatorname{im}D^T,
\]

but its direct construction used the relational pseudoinverse `L^+`. The
question left open was whether local dynamics could form the same record
without hiding that solve in one update.

There is a minimal conditional answer. On an even periodic cubic matched-face
probe with neutral fixed ternary charge, alternate the two cell parities. At
each active cell, rotate its six-face Gauss residual into a fresh signed
environment amplitude. Every gate is local and exactly reversible. With fresh
zero ports, each parity layer is an orthogonal affine projection; alternating
layers converge from empty flux to `J_s` without evaluating `L^+` inside any
gate.

The price is exact and non-negotiable within this realization:

- every half-layer consumes `V/2` fresh signed environment ports;
- exact generic completion is asymptotic or requires a layer count growing
  with probe size;
- the retained signed history is required for exact reversal; and
- for the empty-field/fresh-zero preparation, the limiting environment energy
  equals the final record-field energy. The static source supplies twice the
  retained field energy: half remains in the actual record and half is carried
  away as reversible history.

This is the first target-blind dynamic reference preparation of the FTD-0880
record. It is not an autonomous finite closed substrate mechanism, a
production Gauss replacement, a Born rule, or a `G*` gearbox.

## 2. Even matched complex and local residual

Let `L>=4` be even. Color the periodic cubic cells by

\[
 \chi(x)=x_1+x_2+x_3\pmod2.
\]

Each positive-axis face joins one even and one odd cell. Let `d_x` be the row
of the matched incidence divergence `D` at cell `x`. It contains one incoming
and one outgoing coefficient for each axis, so

\[
 \|d_x\|^2=6.                                                  \tag{1}
\]

No two cells of one color share a face. Therefore

\[
 \langle d_x,d_y\rangle=6\delta_{xy}
 \quad\text{whenever}\quad\chi(x)=\chi(y).                    \tag{2}
\]

For a neutral actual source `s_x in {-1,0,+1}` and declared coupling `g`, set

\[
 q_x=gs_x,qquad \sum_xq_x=0,qquad r_x=d_xJ-q_x.              \tag{3}
\]

The gate reads only `r_x`, hence only the six incident faces and the local
actual source. No global mean, pseudoinverse, probability, outcome target, or
remote measurement context enters it.

## 3. Residual/environment quarter-turn

Let `e_x` be a signed incoming environment amplitude measured in the same
units as `r_x`. Define

\[
 \boxed{
 J'=J+\frac{d_x^T}{6}(e_x-r_x),\qquad e'_x=-r_x.}              \tag{4}
\]

Equation (1) gives

\[
 d_xJ'-q_x=e_x.                                                \tag{5}
\]

Thus, on the normalized plane

\[
 u_x=\frac{r_x}{\sqrt6},\qquad a_x=\frac{e_x}{\sqrt6},
\]

the gate is

\[
 (u_x,a_x)\longmapsto(a_x,-u_x).                              \tag{6}
\]

It is the inverse orientation of the already selected FTD-0872 quarter-turn:
orthogonal, determinant `+1`, fourth order, and exactly reversible. The five
face directions tangent to `d_x` are unchanged.

From the output, recover

\[
 e_x=d_xJ'-q_x,qquad r_x=-e'_x,qquad
 J=J'+\frac{d_x^T}{6}(-e'_x-e_x).                             \tag{7}
\]

Hence the full gate is injective for every input. Dropping `e'_x` is not: for
any scalar `t`, the two inputs `J` and `J+t d_x^T` have the same fresh-port
projection but different outgoing residuals.

## 4. Checkerboard layers

All gates of one color have disjoint support and commute. With fresh incoming
ports `e_x=0`, the color-`c` layer is

\[
 P_cJ=J-\frac16D_c^T(D_cJ-q_c).                               \tag{8}
\]

By (2), this is the orthogonal affine projection onto

\[
 \mathcal A_c=\{J:D_cJ=q_c\}.                                 \tag{9}
\]

Use existing integer tick parity to alternate

\[
 J_{m+1}=P_1P_0J_m,qquad J_0=0.                              \tag{10}
\]

The schedule is axis-free: it uses the bipartite cell color, not a selected
spatial direction. Swapping the names of the two colors shifts the schedule by
one half-layer and changes no fixed point.

Each outgoing amplitude is shifted onto the already selected oriented history
rail. Retaining the ordered list

\[
 \mathcal H_N=(c_0,e'_0;c_1,e'_1;\ldots;c_{N-1},e'_{N-1})
\]

makes every finite sequence exactly reversible by applying (7) in reverse
layer order. The actual field subsystem may contract while the full
field-plus-history map remains bijective.

## 5. Convergence to the minimum-energy record

Choose any compatible solution `J_*`. Translation by it turns (8) into

\[
 P_cJ-J_*=Q_c(J-J_*),qquad
 Q_c=I-\frac16D_c^TD_c,                                       \tag{11}
\]

where `Q_c` is the orthogonal projector onto `ker D_c`.

The finite-dimensional principal-angle decomposition of two subspaces splits
`Q_1Q_0` into their common intersection plus two-dimensional blocks with
contraction factors strictly smaller than one. Equivalently, if both
projection steps preserve the norm of a vector, equality in the orthogonal-
projection inequalities forces that vector into both kernels. Therefore

\[
 (Q_1Q_0)^m\longrightarrow P_{\ker D_0\cap\ker D_1}
 =P_{\ker D}.                                                  \tag{12}
\]

Every update in (8) is a linear combination of rows of `D`, so the zero-start
sequence stays in `im D^T`. The affine intersection has exactly one point in
that subspace:

\[
 \mathcal A_0\cap\mathcal A_1\cap\operatorname{im}D^T
 =\{D^TL^+q\}.                                                 \tag{13}
\]

Combining (10)--(13),

\[
 \boxed{J_m\longrightarrow J_s=D^TL^+q.}                      \tag{14}
\]

The pseudoinverse labels the limit; it is never evaluated by a local gate.

## 6. Exact rate identity and finite-time boundary

Let `B` be the even-to-odd nearest-neighbour adjacency block. Cross-color
incidence rows obey

\[
 \frac1{6}D_ED_O^T=-\frac{B}{6}.                              \tag{15}
\]

Immediately after an odd layer, `r_O=0`. The next completed sweep therefore
obeys the exact recurrence

\[
 r_E^{(m+1)}=\frac{BB^T}{36}r_E^{(m)}.                         \tag{16}
\]

The singular value `6` is the single row-dependency mode of the connected
bipartite graph. Since total Gauss residual is zero and the odd residual has
just been cleared, neutrality removes that uniform even component. Every
remaining finite-probe singular value is strictly below six, giving geometric
convergence.

For the locked `L=4` probe, the cubic adjacency spectrum has absolute values

\[
 \{0,2,4,6\}.
\]

After the dependency mode is removed,

\[
 \|r_E^{(m+1)}\|\le\frac49\|r_E^{(m)}\|.                      \tag{17}
\]

The exact-arithmetic FTD-0881 dipole witness satisfies (16)--(17) through all
eight registered sweeps and reverses all sixteen half-layers exactly.

There cannot be a size-independent finite exact sweep count. If `K` local
layers prepared `D^TL^+q` for every probe, the output at a face would depend
only on a radius-`O(K)` source neighborhood. That would be a uniformly finite-
range right inverse of `D`, contradicting FTD-0880. The rate also approaches
one on long-wavelength modes as the probe grows. The relational solve has
become local propagation in time, not disappeared.

## 7. Exact energy and work ledger

Because every exact solution satisfies `d_xJ_s=q_x`, (4) preserves

\[
 \frac12\|J-J_s\|^2+\frac{e_x^2}{12}.                         \tag{18}
\]

After `N` retained layers, define

\[
 E_{\rm hist}^{(N)}
 =\sum_{\ell<N}\sum_{x\in c_\ell}\frac{(e'_{\ell x})^2}{12}.
\]

For empty initial flux and fresh zero ports, (18) telescopes to

\[
 \frac12\|J_N-J_s\|^2+E_{\rm hist}^{(N)}
 =\frac12\|J_s\|^2.                                          \tag{19}
\]

This centered invariant is not used as a hidden update target. Its physical
work form is local. One gate changes face-field plus current-port energy by

\[
 \Delta\left(\frac12\|J\|^2+\frac{e_x^2}{12}\right)
 =\frac{q_x}{6}(e_x-r_x)=:w_x.                                \tag{20}
\]

Book `w_x` as work drawn from the static source/controller. For every finite
history,

\[
 \frac12\|J_N\|^2+E_{\rm hist}^{(N)}-W_{\rm src}^{(N)}=0,
 \qquad W_{\rm src}^{(N)}=\sum_{\ell,x}w_{\ell x}.            \tag{21}
\]

Taking the limit (14) in (19)--(21) yields

\[
 \boxed{
 E_{\rm hist}^{(\infty)}
 =E_{\rm field}^{(\infty)}
 =\frac12\|J_s\|^2,qquad
 W_{\rm src}^{(\infty)}=\|J_s\|^2.}                          \tag{22}
\]

This is an exact self-dual energy split in the registered preparation:

```text
actual static record energy = exported signed-history energy
                             = one half of supplied source work.
```

It follows from the quarter-turn plus empty-field/fresh-zero boundary. It is
not asserted for arbitrary preparation dynamics, matter, brains, galaxies, or
the universe as a whole.

## 8. What is natural and what remains selected

The following parts are forced after the matched-face representation and
record target have been selected:

- six-face normalization `1/6` from the cubic incidence row norm;
- the two disjoint checkerboard color classes;
- convergence to the unique minimum-energy longitudinal record;
- exact inverse reconstruction from retained signed history; and
- the work and energy identities (18)--(22).

The orientation of the residual/environment quarter-turn and the outward
history rail reuse the existing `SEL-CA-PHASE-RAIL` reference architecture.
No sixth selected v2 type is minted.

The gate is target-blind in the relevant sense: it reads the actual local
source `q_x`, not a Born probability, desired outcome, remote setting, or
precomputed `J_s`. Reading `q_x` is physical source coupling, not target-
probability leakage.

## 9. Exact remaining boundary

### Closed positive

- local reversible residual/environment gate;
- disjoint checkerboard scheduling;
- exact finite-history inverse;
- convergence from empty flux to the FTD-0880 static record;
- exact `L=4` contraction ceiling `4/9` after dependency removal;
- local source-work ledger; and
- limiting equality of retained field and exported history energy.

### Closed negative

- a fixed probe-independent finite number of these local layers is an exact
  Gauss-record solver for all sizes;
- dropping nonzero outgoing residuals preserves injectivity; and
- a finite cyclic environment remains a fresh zero environment indefinitely.

### Still open

- a substrate-native source of fresh ready ports or an autonomous reversible
  recycling mechanism;
- a positive canonical source-reservoir microdynamics realizing the signed
  work ledger without an external controller;
- autonomous local stopping/compliance without a global residual readout;
- moving charges and exact continuity-current coupling during preparation;
- nonneutral, odd-periodic, open-boundary, and uncontained probes;
- finite-capacity backpressure, routing, collision handling, noise, and
  robustness;
- production migration to a matched face/link complex;
- physical amplitude and time scales;
- synchronization to the separate quartic `G*` calendar; and
- Born recovery, Bell laboratory recovery, Lorentz hiding, and completeness.

The largest next question is no longer “can a local reversible preparation
exist?” It is whether the substrate can generate and recycle the fresh signed
environment rail and source work autonomously without target coding or an
unbounded hidden reservoir.

## 10. Verification record

The frozen FTD-0881 protocol SHA-256 is
`50816F74F87D6120C871031D25EF704479B3E4873EB4F108080516C74E298942`.
The frozen parent certificate SHA-256 is
`99B570E8E8CFD8FB7474060F3B0114281F2C2F02E92F47BA77E33139414EB634`.
Its first locked execution reported `58/60`: C34 used an unnormalized
line-wrapped Markdown marker and C60 failed dependently. No substantive gate
failed.

FTD-0882 froze a one-substitution whitespace-normalization repair. Its
protocol SHA-256 is
`BD9E7DB871EEDD590A6CBFADD2B8F07AC38118433DC34925477292D91257B989`;
its wrapper SHA-256 is
`118D9CA893432777744C7F771062E440B451D78F11502F3BA13A5A33A12B22F7`.
The repaired inherited certificate passes `60/60` with terminal markers:

```text
REVERSIBLE_CHECKERBOARD_GAUSS_PREPARATION_THEOREM
LOCAL_RESIDUAL_ENVIRONMENT_GATE=ORIENTED_QUARTER_TURN
FINITE_HISTORY_REVERSIBILITY=EXACT
MINIMUM_ENERGY_RECORD_LIMIT=EXACT
GENERIC_FIXED_FINITE_SWEEP_COMPLETION=NO
LIMIT_ENERGY_SPLIT=FIELD_HALF_HISTORY_HALF
PSEUDOINVERSE_IN_LOCAL_GATE=NO
PRODUCTION_COUPLING=NONE
GSTAR_ROLE=SEPARATE_CALENDAR
BORN_BELL_STATUS=UNTOUCHED
```
