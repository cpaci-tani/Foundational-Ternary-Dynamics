# Pre-registration — Production clock-indexed C4 twist census v1

**Identifier:** `FTD-0978`  
**Date locked:** 2026-08-12  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE FIRST EXECUTION]`  
**Expected classifier:** `Outcome B`

## 1. Question

Does the unchanged production de Broglie-clock plus weak-transmutation route
realize the physical twisted endpoint identification left open by FTD-0977,
or does it contain only a clock-blind order-two exchange of the left and right
substrates?

The census is deliberately narrower than a whole-engine no-go. It tests the
production paths that currently own:

1. the voxel clock variables `tau` and `phase`;
2. the imposed Klein--Gordon clock term;
3. the CPU and CUDA left/right weak-transmutation exchange;
4. the tick ordering connecting those mechanisms; and
5. the production energy ledger that could book their reaction or work.

No production file, engine type, toggle, coupling, tick phase, `G*` value,
Born target, or public interface may change. No numerical search, fit, or
near-miss comparison is permitted.

## 2. Frozen production sources

| Source | Frozen SHA-256 |
|---|---|
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `engine/include/ftd/term_toggles.h` | `2731A2BF1EF01456DFDFE4F1E20C8E64E3D839136BC633B13771D13360AC64AA` |
| `engine/src/transmutation_phases.cpp` | `4013A9B769199D54976347378FD03DFF6415B7F641F35D3FAE498125EB288043` |
| `engine/src/render_bridge.cpp` | `BFAD7886CB83A590F0AACA11C03CE25B1FF51D94B4C17B06F5D555E46C18D724` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/include/ftd/gpu_buffers.h` | `92AE9190121D278AE5FBA0A74F708063D566DBF0B4036B2DCADC1CAC41A535DF` |
| `engine/cuda/gpu_buffers.cu` | `9154CC003D3F8E25FC5BB3EC608417F464C09894440C97572004C9C0489FDDA4` |
| `engine/cuda/gpu_engine.cu` | `302D93022251F53668BFF556088AECAE3F44D1BF1FF5CCE469B4FDDD98D4A96D` |
| `engine/cuda/kernels_aux.cu` | `E385FCFC93A2188E094798FC3A2C0A0839A6139313D738EE2E69254C6921739C` |
| `engine/cuda/kernels_stencil_dual.cu` | `25365B176BB333009333E2B5A596F792E2245719D107E754CE3C6BF5BAE9F1C0` |
| `engine/include/ftd/render_bridge_diagnostics.h` | `5A9525591D3D818377E4688FBE4A57229B5CB7C36E62FF07D76941D814D57F69` |
| `engine/src/energy_ledger_compute.cpp` | `2E5138BA43F74624C47842E9C3B0372ADFA9288BFE175BFE75ED901F237DD61B` |

The controlling mathematical source is FTD-0977: a physical quarter
holonomy requires a clock-indexed twisted gluing or an equivalent retained
transition, not merely a regular local pure-gauge connection.

## 3. Registered production criteria

For this route to count as the FTD-0977 physical witness, every following
criterion must be present in the frozen production sources.

### 3.1 Clock indexing

The left/right transition predicate must read a local clock phase, proper
time, or an equivalent preregistered phase-crossing latch. Merely executing
inside the same global tick is not clock indexing.

### 3.2 Oriented quarter action

On one canonical relative pair, the transition must distinguish the two
orientations and realize a map conjugate to

\[
 J=\begin{pmatrix}0&-1\\1&0\end{pmatrix},\qquad
 J^2=-I,\qquad J^4=I.                                    \tag{1}
\]

An exchange whose relative action is only `-I` is a half-turn. It equals
both `J^2` and `(-J)^2`, so it loses the clockwise/counterclockwise bit that
the requested gearbox must retain.

### 3.3 Canonical reaction and transaction closure

The production state must contain the conjugate clock/reaction variable, and
the transition must book its impulse, work or reserve, retained history, and
inverse. An optional observation-only journal is not a production-state
inverse.

### 3.4 CPU/GPU consistency

The same result must hold on both CPU and CUDA paths. A host-only diagnostic
phase cannot control a device transition that has no phase input.

## 4. Frozen algebraic controls

For one Cartesian component, order the dual canonical variables as

\[
 z=(q_L,q_R,p_L,p_R)^T.
\]

The implemented simultaneous exchange is

\[
 S=\begin{pmatrix}
 0&1&0&0\\1&0&0&0\\0&0&0&1\\0&0&1&0
 \end{pmatrix}.                                           \tag{2}
\]

The certificate must verify exactly that `S` is symplectic and norm
preserving, but

\[
 S^2=I.                                                    \tag{3}
\]

Under common/relative coordinates

\[
 q_C={q_L+q_R\over\sqrt2},\quad q_D={q_L-q_R\over\sqrt2},
 \qquad
 p_C={p_L+p_R\over\sqrt2},\quad p_D={p_L-p_R\over\sqrt2}, \tag{4}
\]

the exchange must reduce to

\[
 (q_C,q_D,p_C,p_D)\mapsto(q_C,-q_D,p_C,-p_D).             \tag{5}
\]

Thus the relative pair sees `-I`, not either oriented generator `+J` or
`-J`.

The symmetric Klein--Gordon action must commute with `S`. This establishes
compatibility of the clock oscillator with exchange symmetry, but not a
clock-indexed holonomy.

## 5. Frozen source and reversibility tests

The certificate must establish from the frozen sources that:

1. `Voxel::phase` is declared read-only diagnostic state and is advanced
   after the weak phase by `phase += omega0 * delta_tau`;
2. the CPU weak predicate uses state, stress, threshold, seeded voxel RNG,
   and tick, but neither `phase` nor `tau`;
3. the CUDA weak kernel has the same dependency structure and receives no
   phase/proper-time argument;
4. the device state has `d_tau` but no `d_phase` buffer;
5. the imposed Klein--Gordon term acts with the same scalar coefficient on
   both left and right fluxes;
6. CPU and GPU both swap left/right flux and left/right wave velocity only
   after the weak predicate fires;
7. the conditional swap itself is invertible, but the full predicate-gated
   map need not be injective: if `P(x)=true` and `P(Sx)=false`, both `x` and
   `Sx` map to `Sx`;
8. the optional CPU history journal is explicitly observation-only; and
9. the production energy ledger contains no clock potential, connection,
   switching-work, or retained-history channel.

The non-injectivity control is a theorem about the implemented form
`F(x)=Sx if P(x), else x`; it does not assert that every runtime state realizes
that counterexample.

## 6. Frozen checks

- **G1:** protocol hash, all source hashes, and exact scope markers;
- **G2:** host clock state, update location, and tick ordering;
- **G3:** CPU weak predicate and exchange dependency census;
- **G4:** CUDA buffer, predicate, launcher, and exchange dependency census;
- **G5:** exact symplectic swap, order, common/relative decomposition, and
  oriented-quarter discriminator;
- **G6:** symmetric clock-force commutation and CPU/GPU phase-write symmetry;
- **G7:** conditional inverse versus predicate-gated non-injectivity and
  observation-only history boundary;
- **G8:** energy/reaction/work/history ledger omissions;
- **G9:** no `G*` cadence or Born/Bell target enters the audited route;
- **G10:** no production, CMake, type, toggle, coupling, or interface change.

All matrix and predicate controls are exact. Source absence claims are scoped
only to the frozen files and named route.

## 7. Frozen classifier

- **Outcome A — production C4 witness:** the existing route is clock indexed,
  realizes an oriented order-four action, retains its inverse/reaction/work,
  and is consistently represented on CPU and CUDA.
- **Outcome B — named route closed negative / capacity retained:** the clock
  and exchange exist, but are dynamically decoupled; the exchange is an
  order-two half-turn, and the production state/ledger lacks the required
  reaction and retained twist. This route does not realize FTD-0977, while a
  future explicit clock-indexed mapping-torus mechanism remains open.
- **Outcome C — partial witness:** at least one but not all production criteria
  is present, requiring a narrower unresolved classification.
- **Outcome D — invalid:** a hash, exact algebraic control, source marker, or
  scope gate fails.

The expected result is Outcome B. Passing it licenses no production change
and makes no claim that a different substrate mechanism is impossible.
