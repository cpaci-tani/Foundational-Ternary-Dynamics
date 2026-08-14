# FTD-0881 — Reversible checkerboard Gauss-record preparation v1

**Identifier:** `FTD-0881`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN]`  
**Date:** 2026-08-11  
**Parents:** `FTD-0852`, `FTD-0872`, `FTD-0874`, `FTD-0876`, `FTD-0880`  
**Production status:** unchanged; isolated matched-face reference dynamics only

## 1. Registered question

Can the exact static Gauss record of FTD-0880 be formed dynamically without
evaluating a pseudoinverse inside any update, using only:

1. the local six-face Gauss residual at one cell;
2. the cell's actual neutral ternary source;
3. global tick/checkerboard parity; and
4. a fresh signed environment amplitude carried away on the already selected
   oriented history rail?

The registered candidate is a reversible residual/environment quarter-turn.
The certificate must decide whether alternating disjoint checkerboard layers
converge from empty flux to the unique minimum-energy matched Gauss record,
whether complete retained history makes every finite preparation exactly
reversible, and what energy and capacity are paid.

This is not a Born-weight generator, a measurement selector, a production
Gauss replacement, a finite-time exact solver, or a `G*` clock gearbox.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_GAUSS_RECORD_CANONICAL_REDUCTION_AND_PRODUCTION_PROJECTOR_BOUNDARY_v1.md` | `47B878F85674DC3FCCAE3DC109EA94BC4DB3B520B8E35AC85F42FF7B2F544D95` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_ALTERNATING_ORIENTED_TERNARY_PARITY_RAIL_AND_ONE_SHOT_BOUNDARY_v1.md` | `E70F2AD61BFA1C8BBFF4EA03DCF0312B8F96224ECF2453FDF4B81B0FEA845CA1` |
| `engine/include/ftd/eft/matched_gauss_transport.h` | `1E07F87A0EBD0D1830D0632B82C2BD65497EBEAE7BB152EA02C5AAE19328B033` |
| `engine/src/eft/matched_gauss_transport.cpp` | `12BF98040BB45AD6CD9A409A93C842101C400CEEE6242E9B9352158A33A9D028` |
| `engine/include/ftd/eft/oriented_ternary_quarter_turn.h` | `46CD15943F5EB8EDBBCE4676CDE558A7C2B08556E1AC64E7C9720D30FFEB68E1` |
| `engine/include/ftd/eft/alternating_oriented_ternary_parity_rail.h` | `E62026FA4228CFB8FB798EBF2E0C68011E6ABA6328050F80F9FD0573275604DD` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md` | `7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7` |

Any mismatch invalidates the run. The certificate may read only these frozen
sources and this protocol. Deterministic exact-arithmetic evaluation on the
registered `L=4` probe is permitted. Numerical near-miss searches, coefficient
fitting, target tuning, and post-run threshold changes are forbidden.

## 3. Frozen finite-probe class

Let `L>=4` be even and let the periodic cubic cells be colored by

\[
 \chi(x)=x_1+x_2+x_3\pmod 2.
\]

Let `D` be the FTD-0880 positive-face incidence divergence. Its cell row
`d_x` has six coefficients `+1,-1`, hence

\[
 \|d_x\|^2=6.
\]

Rows of one color have disjoint face support and are therefore mutually
orthogonal. For a fixed neutral ternary source `q_x=g s_x`, define

\[
 r_x=d_xJ-q_x.                                                \tag{1}
\]

The protocol is restricted to neutral periodic probes. A mean-subtracted
background for nonneutral probes and an uncontained isolated-charge limit
remain open.

## 4. Frozen reversible local gate

At active cell `x`, let `e_x` be an incoming signed environment amplitude in
the same residual units as `r_x`. Apply

\[
 \begin{aligned}
 J'&=J+\frac{d_x^T}{6}(e_x-r_x),\\
 e'_x&=-r_x.                                                   \tag{2}
 \end{aligned}
\]

Because `d_xJ'-q_x=e_x`, equation (2) is precisely the oriented quarter-turn

\[
 (r_x,e_x)\longmapsto(e_x,-r_x).                              \tag{3}
\]

on the normalized two-plane `(r_x/sqrt(6),e_x/sqrt(6))`, with the five
tangential face directions fixed. Its inverse is

\[
 r_x=-e'_x,\qquad e_x=d_xJ'-q_x,qquad
 J=J'+\frac{d_x^T}{6}(-e'_x-e_x).                             \tag{4}
\]

All gates of one checkerboard color commute because their face supports are
disjoint. A **fresh layer** has `e_x=0` at every active cell. It therefore
sets every active residual to zero and is exactly the orthogonal projection
onto

\[
 \mathcal A_c=\{J:D_cJ=q_c\}.                                 \tag{5}
\]

The outgoing signed amplitudes `e'_x=-r_x` are shifted onto a retained
history rail. Reusing a nonzero incoming port is not a projection; freshness
is a physical readiness condition, not a disposable convention.

## 5. Frozen recursion and convergence claim

Starting from `J_0=0`, apply the color-zero and color-one fresh layers in that
order every sweep:

\[
 J_{m+1}=P_1P_0J_m.                                           \tag{6}
\]

For any compatible solution `J_*`, translation by `J_*` converts `P_c` into
the orthogonal projector onto `ker D_c`. Finite-dimensional principal-angle
decomposition then gives

\[
 (P_1P_0)^mJ_0\longrightarrow P_{\mathcal A_0\cap\mathcal A_1}J_0.
\]

Every update from zero lies in `im D^T`. Since

\[
 (\mathcal A_0\cap\mathcal A_1)\cap\operatorname{im}D^T
 =\{D^TL^+q\},                                                \tag{7}
\]

the limit is exactly the FTD-0880 minimum-energy record `J_s=D^TL^+q`.
No gate evaluates `L^+`; the inverse Laplacian appears only in identifying the
global limit.

At a completed odd layer, the compatible even residual obeys

\[
 r_E^{(m+1)}=\frac{BB^T}{36}r_E^{(m)},                         \tag{8}

\]

where `B` is the even-to-odd nearest-neighbour adjacency block. Neutrality
removes the unique singular-value-six dependency mode. On the registered
`L=4` cubic probe the remaining adjacency singular values are at most four,
so the exact full-sweep residual contraction factor is at most

\[
 \rho_4=\frac{4^2}{6^2}=\frac49.                              \tag{9}

\]

FTD-0880's no-uniform-local-right-inverse theorem implies that no fixed number
of local layers can prepare `D^TL^+q` exactly for every probe size. Exact
generic completion in this architecture is asymptotic or requires a number
of layers growing with the probe; finite tolerance uses finite retained
history.

## 6. Frozen information and energy ledger

For any exact solution `J_s`, one gate preserves

\[
 \frac12\|J-J_s\|^2+\frac{e_x^2}{12},                         \tag{10}

\]

with all previously exported amplitudes included additively. Consequently a
finite sequence is exactly reversible when the signed layer history and
parities are retained.

The physical face-field plus current-port energy changes locally by

\[
 \Delta E_{J+e}
 =\frac{q_x}{6}(e_x-r_x)=:w_x.                                \tag{11}

\]

`w_x` is booked as work supplied by the static source/controller. Summing it
over retained layers gives the exact audit

\[
 E_J^{(N)}+E_{\rm env}^{(N)}-W_{\rm src}^{(N)}=0               \tag{12}

\]

for the registered empty initial field and fresh zero environment.

In the converged limit, (10)--(12) force

\[
 E_{\rm env}^{(\infty)}=E_J^{(\infty)}
 =\frac12\|J_s\|^2,qquad
 W_{\rm src}^{(\infty)}=\|J_s\|^2.                           \tag{13}

\]

Thus the reference mechanism has an exact self-dual split: half of the source
work remains as the actual static record field and half leaves as signed
history/environment energy. This equality follows from the reversible
quarter-turn and fresh-zero boundary condition; it is not a general theorem
about all physical preparation mechanisms.

Dropping any nonzero outgoing `e'_x` makes the corresponding gate
noninjective. A finite cyclic environment eventually reintroduces old
amplitudes and ceases to implement fresh projections. An outward/unbounded
rail, a finite-tolerance capacity declaration, or an autonomous recycling
mechanism is therefore required.

## 7. Registered certificate gates

The source-locked certificate must report exactly sixty checks.

### Provenance

- **C1--C7:** the seven source hashes match section 2.
- **C8:** this protocol hash matches the pre-run lock embedded in the frozen
  certificate before its first execution.

### Local cubic geometry and gate algebra

- **C9:** the registered even periodic probe is bipartite.
- **C10:** every face is incident on one cell of each color.
- **C11:** active rows of either color have disjoint face support.
- **C12:** every incidence row has squared norm six.
- **C13:** normalized active rows are orthonormal.
- **C14:** equation (2) is implemented exactly.
- **C15:** the post-gate residual equals the incoming environment amplitude.
- **C16:** the outgoing environment amplitude is the negative old residual.
- **C17:** equation (4) recovers arbitrary registered rational inputs exactly.
- **C18:** the normalized residual/environment block has `R^2=-I`, determinant
  one, and positive orientation.

### Layer locality and reversible history

- **C19:** same-color gates commute.
- **C20:** a fresh layer is the exact affine orthogonal projection (5).
- **C21:** every gate reads only six incident faces, local `q_x`, parity, and
  its environment port.
- **C22:** simultaneous sign reversal of `(J,q,e)` commutes with the gate.
- **C23:** every outgoing signed residual is retained in the layer history.
- **C24:** reversing one retained layer recovers the prior flux exactly.
- **C25:** reversing all registered layers in reverse order recovers the empty
  field exactly.
- **C26:** no local gate evaluates a pseudoinverse, global sum, probability,
  or measurement setting.
- **C27:** a nonzero incoming environment amplitude generally prevents exact
  active-cell projection.
- **C28:** fresh exact operation consumes `V/2` environment ports per layer;
  finite capacity/backpressure is not hidden.

### Convergence and nonlocality boundary

- **C29:** the fresh even layer is `P_0`.
- **C30:** the fresh odd layer is `P_1`.
- **C31:** translation by a compatible solution gives linear orthogonal
  projectors onto `ker D_0` and `ker D_1`.
- **C32:** each translated layer is norm-nonincreasing.
- **C33:** equality through both layers outside the common kernel is excluded.
- **C34:** the finite-dimensional alternating sequence converges to the common
  affine intersection.
- **C35:** zero-start updates remain in `im D^T`.
- **C36:** equation (7) identifies the unique limit as `D^TL^+q`.
- **C37:** the exact-arithmetic registered `L=4` neutral dipole sequence obeys
  the frozen `4/9` full-sweep residual bound.
- **C38:** the complete registered finite sequence reverses exactly.
- **C39:** the even/odd cross Gram matrix is `-B/6`.
- **C40:** equation (8) is exact at completed odd layers.
- **C41:** neutrality removes the uniform dependency mode.
- **C42:** the registered `L=4` nontrivial contraction ceiling is exactly
  `4/9`.
- **C43:** a size-independent finite exact preparation would contradict the
  FTD-0880 uniformly-local-right-inverse no-go.
- **C44:** generic exact completion is not claimed at any fixed finite sweep.

### Energy, information, and scope

- **C45:** equation (10) is preserved by one gate.
- **C46:** centered field plus all retained environment energy is preserved by
  every registered finite sequence.
- **C47:** equation (11) equals the direct physical field/port energy change.
- **C48:** the local source-work entry closes each gate exactly.
- **C49:** equation (12) telescopes over the registered history.
- **C50:** the converged environment energy equals the static field energy.
- **C51:** converged source work equals twice the static field energy.
- **C52:** dropping a nonzero outgoing residual creates an explicit collision.
- **C53:** the mechanism reads no Born weight, probability target, outcome, or
  remote measurement setting.
- **C54:** the residual/environment quarter-turn and history transport consume
  the existing `SEL-CA-PHASE-RAIL`; no new selected type is added.
- **C55:** no production `Voxel`, Gauss toggle, or tick phase is modified.
- **C56:** moving-source continuity coupling remains open.
- **C57:** nonneutral, odd-periodic, finite-boundary, and uncontained probes
  remain open.
- **C58:** autonomous port freshness, stopping, recycling, positive source
  reservoir microdynamics, robustness, and `G*` synchronization remain open.
- **C59:** every frozen scope marker below is present.
- **C60:** the terminal verdict is emitted only if C1--C59 all pass.

## 8. Frozen outcome rule

- **Outcome A:** `60/60`; book the scoped theorem and an isolated `ftd::eft`
  witness. Dynamic reference preparation closes conditionally on a neutral
  even periodic matched complex and a fresh retained environment rail.
- **Outcome B:** provenance passes but any mathematical gate fails; book the
  counterexample and no theorem.
- **Execution invalid:** any hash mismatch, exception, wrong check count, or
  scope-marker failure; preserve the run and preregister any repair.

No post-run threshold, probe, recurrence, source, or scope change is allowed.

## 9. Frozen interpretation

If Outcome A occurs, the permitted claims are:

- **[THEOREM, CONDITIONAL]** alternating local fresh residual/environment
  quarter-turns converge from empty flux to the exact minimum-energy matched
  Gauss record;
- **[THEOREM]** every finite retained-history sequence is exactly reversible;
- **[THEOREM]** the work ledger is local and exact, with the reference limit
  split equally between retained field energy and exported history energy;
- **[CLOSED NEGATIVE]** any size-independent finite number of these local
  layers prepares the exact record on every probe;
- **[SELECTION — EXISTING TYPE]** the gate orientation, checkerboard schedule,
  and signed history rail consume `SEL-CA-PHASE-RAIL` without minting a type;
- **[OPEN]** autonomous environment freshness/recycling, positive source
  reservoir dynamics, finite-boundary/uncontained extension, moving sources,
  production migration, robustness, stopping, scale, and separate quartic
  `G*` synchronization.

The result is reference dynamics, not substrate Born evidence and not a
whole-framework completeness result.

## 10. Scope markers

```text
GAUSS_PREPARATION_STATUS=SELECTED_REVERSIBLE_REFERENCE_EXISTING_TYPE
LOCAL_GATE_INPUTS=SIX_FACES_LOCAL_TERNARY_SOURCE_PARITY_ENVIRONMENT
PSEUDOINVERSE_IN_LOCAL_GATE=NO
FINITE_HISTORY_REVERSIBILITY=EXACT
GENERIC_FIXED_FINITE_SWEEP_COMPLETION=NO
ENVIRONMENT_FRESHNESS=REQUIRED
LIMIT_ENERGY_SPLIT=FIELD_HALF_HISTORY_HALF
PRODUCTION_COUPLING=NONE
GSTAR_ROLE=SEPARATE_CALENDAR
BORN_BELL_STATUS=UNTOUCHED
```

## 11. Pre-run lock

The exact SHA-256 of this byte-frozen protocol must be embedded in the
certificate and recorded in the preregistration manifest before first
execution. Later outcome prose must not alter this evidence hash.
