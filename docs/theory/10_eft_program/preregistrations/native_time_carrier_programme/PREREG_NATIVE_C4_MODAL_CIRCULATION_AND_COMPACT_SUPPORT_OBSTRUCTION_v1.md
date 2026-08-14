# FTD-0919 — Native `C4` modal circulation and compact-support obstruction v1

**Identifier:** `FTD-0919`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** exact source-free production action, generic source ledger, and
finite-support localization boundary; no numerical search and no engine change

## 1. Question

FTD-0918 established the native plaquette circulation observable

\[
\mathcal L_P=q\cdot p_r-r\cdot p_q
\]

but proved that one elementary plaquette leaks into its exterior. Does the
unchanged production free-field action contain a larger exact invariant
`C4` doublet that conserves this kind of handed circulation? If a global
modal carrier exists, can it be localized into any nonzero finite-support,
finite-dimensional clock body without adding a selected confining dynamics?

This protocol separates:

1. the exact algebraic conservation criterion;
2. global periodic modal witnesses;
3. finite-support localization; and
4. the source/boundary torque ledger.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/include/ftd/field_operators.h` | `25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48` |
| `AUDIT_NATIVE_FIELD_DISCRETE_ACTION.md` | `5EDC7F8C81456BEE4EEB061168154E8EF4D8347B8948C429BB40B8306FFC8AD8` |
| `ANALYSIS_NATIVE_PHASE_ACTION_CM_OPERATOR_v1.md` | `B559C98DB72FBB789E2B9318604A7AB5D788499F0C52771B4265DC53BC3F3DD9` |
| `THEOREM_LOCAL_FLUX_SELF_PAIR_TENSOR_RECURSION_v1.md` | `62A95FF322C99773D03002444376B9244A93CC19D01CF4400230277288CADAEB` |
| `THEOREM_NATIVE_PLAQUETTE_C4_CIRCULATION_AND_EMBEDDED_LEAKAGE_BOUNDARY_v1.md` | `3CD336B101BDA6A4F0E56CBFFC9428C203C5A68E037943408D762900FF58451F` |

The certificate fails closed on source drift.

## 3. Frozen finite-dimensional algebra

Let `J,P` denote the finite-volume real flux and wave-velocity vectors, and
let the source-free stiffness satisfy

\[
K^T=K.
\]

For any real skew generator `A^T=-A`, define

\[
\mathcal L_A=J^TAP.
\]

The production-order kick--drift with arbitrary projected impulse `U` is

\[
P^+=P-hKJ+U,
\qquad
J^+=J+hP^+.
\]

The certificate must derive, not assume, the exact balance

\[
\boxed{
\mathcal L_A^+-\mathcal L_A
=-\frac h2J^T[A,K]J+J^TAU.}
\]

Consequently, in the source-free sector, `L_A` is conserved for every state
if and only if

\[
[A,K]=0.
\]

For common post-drift damping `rho` and additive impulse/noise `eta`, the
certificate must also derive

\[
\mathcal L_{A,\mathrm{end}}
=\rho\left(
\mathcal L_A-\frac h2J^T[A,K]J+J^TAU
\right)+(J^+)^TA\eta.
\]

## 4. Frozen invariant-mode claims

The certificate must prove all of the following.

1. If orthonormal vectors `a,b` obey `Ka=kappa a` and `Kb=kappa b`, then

   \[
   A=ab^T-ba^T
   \]

   is skew, commutes with `K`, and generates an exact conserved modal
   circulation

   \[
   \mathcal L_{ab}=Q_aP_b-Q_bP_a.
   \]

2. Conversely, any nonzero skew `A` commuting with symmetric `K` rotates
   only within degenerate eigenspaces of `K`. A nonzero conserved
   circulation therefore requires stiffness degeneracy.

3. On the exact `L=4` periodic quotient, the two real modes

   \[
   a(x,y,z)=\sin(\pi x/2),\qquad
   b(x,y,z)=\sin(\pi y/2)
   \]

   are orthogonal, have equal norm, form an exact physical `C4` doublet, and
   are eigenmodes of the production 18-point Laplacian with eigenvalue `-2`.
   With `C_WAVE^2=1/3`, their stiffness is `kappa=2/3`, and their modal
   circulation is exactly conserved by the unit source-free tick.

4. The witness in claim 3 is global/delocalized on a finite periodic
   computational quotient. It is not a localized ontology-level clock body.

## 5. Frozen compact-support obstruction

For one scalar field component on the unbounded algebraic lattice scaffold,
let a finite-support field `f` be represented by its Laurent polynomial

\[
F(z_x,z_y,z_z)=\sum_x f_xz^x.
\]

The production stiffness is convolution by a nonconstant Laurent polynomial
`kappa(z)`. A compact-support eigenvector would satisfy

\[
(\kappa(z)-\lambda)F(z)=0.
\]

The certificate and theorem must use the integral-domain property of
`R[z_x^{+/-1},z_y^{+/-1},z_z^{+/-1}]` to conclude `F=0`. Thus:

1. the production free stiffness has no nonzero finite-support eigenvector;
2. it has no nonzero finite-dimensional invariant subspace whose vectors all
   share finite support; and
3. it has no nonzero finite-support finite-rank skew generator `A` satisfying
   `[A,K]=0`.

This is a compact-support theorem. It does not claim that dispersive packets,
periodic-box modes, maintained driven structures, nonlinear defects, or
selected confining dynamics are impossible.

## 6. Frozen band and one-tick order-four boundary

For `c_i=cos(k_i)`, freeze the exact production symbol

\[
L_{18}(k)=\frac23
(c_x+c_y+c_z+c_xc_y+c_yc_z+c_zc_x)-4.
\]

The bracket is multi-affine on `[-1,1]^3`, so its extrema occur at the eight
vertices. The certificate must prove

\[
-\frac{16}{3}\le L_{18}\le0,
\qquad
0\le\kappa_{18}=-\frac13L_{18}\le\frac{16}{9}<2.
\]

For the unit kick--drift family, exact order four requires `kappa=2`.
Therefore no source-free production mode executes an exact one-tick
quarter-turn. This does **not** exclude finite periods other than four or
phase gates sampled over multiple ticks.

## 7. Scope and interpretation firewalls

- The periodic quotient is a finite computational witness, not the uncontained
  substrate ontology.
- Discrete `C4` symmetry alone is not called a Noether theorem. The conserved
  charge follows from the continuous `O(2)` mixing of an exactly degenerate
  quadratic eigenspace.
- The generic source term `U`, common damping, and additive noise are booked
  explicitly. Gauss projection, genesis, evaporation, nonuniform drains, and
  matter reaction remain outside the source-free theorem.
- FTD-0841's selected onsite quartic remains a possible nonlinear confinement
  route but is not present in production and is not promoted here.
- No numerical near-miss, parameter fit, prime subset, empirical target, or
  formula substitution is permitted.
- `G*`, gamma, Born weights, Bell settings, measurement context, selector
  state, and desired outcomes are absent.
- No production source, CMake target, or default is changed.

## 8. Outcome map

- **Outcome A — global conserved modal circulation / compact local free-body
  obstruction:** all exact claims pass. Book the periodic/global modal
  circulation as native free-field content; close a nonzero finite-support
  finite-dimensional invariant circulation body negative for the unchanged
  source-free 18-point action. Leave maintained, driven, nonlinear, and
  selected-confinement routes open.
- **Outcome B — commutator law survives but localization theorem fails:**
  retain only the exact charge balance and periodic witness; issue no compact
  local obstruction.
- **Outcome C — invalid:** any source lock or exact algebraic claim fails.

Outcome A licenses no engine term. The next admissible route is an exact
source-balanced maintained region or a declared nonlinear confinement term
with energy, reaction, support, reversal, and work ledgers. `G*` cadence stays
downstream.

```text
NATIVE_CHARGE=L_A
FREE_CONSERVATION_CRITERION=COMMUTATOR_ZERO
PERIODIC_GLOBAL_C4_WITNESS=REQUIRED
FINITE_SUPPORT_EIGENMODE=FORBIDDEN_IF_OUTCOME_A
FINITE_SUPPORT_INVARIANT_DOUBLET=FORBIDDEN_IF_OUTCOME_A
FREE_ONE_TICK_ORDER_FOUR=FORBIDDEN_IF_OUTCOME_A
MAINTAINED_OR_NONLINEAR_LOCAL_CLOCK=OPEN
PRODUCTION_CHANGED=FALSE
GSTAR_READ=FALSE
GAMMA_DERIVED=FALSE
BORN_BELL_CONTEXT_READ=FALSE
STATUS=LOCKED_PRE_CERTIFICATE
```

**LOCKED CONTENT ENDS HERE.**
