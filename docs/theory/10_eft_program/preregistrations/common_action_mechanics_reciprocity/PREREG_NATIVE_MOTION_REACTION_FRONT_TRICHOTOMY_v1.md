# Preregistration — Native Motion / Reaction-Front Trichotomy (FTD-0585)

**Status:** `[LOCKED — RUN OF RECORD NOT YET EXECUTED]`  
**Date:** 2026-07-26  
**Production effect:** none.

## 1. Question

The frozen engine can change manifested support in two different ways:

1. transport a state through a face current;
2. destroy polarity at one site and create it at another through a reaction
   source.

This gate asks whether those mechanisms can be distinguished exactly, whether
reaction-free matter at rest acquires kinematics from native field evolution,
and whether evaporation/genesis preserves hidden voxel kinematics that can
masquerade as newly generated motion.

An exploratory compile/smoke run preceded this document. No tolerance,
physical parameter, source rule, or verdict was changed from that run. This
lock governs the versioned run of record and the independent proof.

## 2. Frozen sources

| source | SHA-256 |
|---|---|
| `engine/src/render_bridge.cpp` | `A822E0FAFAF71FE5458B2A7450868A8414B1C8564089BF6C6484FC34B7559359` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/src/render_bridge_phases/phase_forces.cpp` | `F7A855DC3ED3BF9882807CF7C8D1A35CF66864433B711CA5CA4B9CB836549322` |
| `engine/src/render_bridge_phases/phase_movement.cpp` | `6149B37C5A28B8EE9B8544CAEC24006D0964D1C8F344CA63C68DC6536A47E8FB` |
| `engine/src/transmutation_phases.cpp` | `4013A9B769199D54976347378FD03DFF6415B7F641F35D3FAE498125EB288043` |
| `engine/src/strong_stress_energy.cpp` | `A9A38B8D0FE6FAA9692CED77AC29841E9FB41596E7E16DB2F45F20E4F2C69F94` |
| `engine/include/ftd/term_toggles.h` | `2731A2BF1EF01456DFDFE4F1E20C8E64E3D839136BC633B13771D13360AC64AA` |
| `engine/include/ftd/eft/dual_cell_continuity.h` | `3DF32601AD46761A2870FFFF0DB9D65CC5C267EC7866668333F0BEC8E176DF43` |
| `engine/src/eft/dual_cell_continuity.cpp` | `90559DDFFE622991D958E6D04A034C470CB4AD8491F83A3BD33771AD0D7BE6D1` |

Registered instrument hashes at lock:

| instrument | SHA-256 |
|---|---|
| `engine/include/ftd/eft/native_motion_reaction_front.h` | `4192A26F4405C15E4E9BEE9BBEA9925E32CC4B58AACC98C5FC913ECE654D1175` |
| `engine/src/eft/native_motion_reaction_front.cpp` | `36C04AC5CE5F652CA157C095CFF5AEF6E7AB14F6FF44098FCCDD0DF941CBEC95` |
| `engine/tests/test_native_motion_reaction_front.cpp` | `348EA25A637C562A3074971B5235BBD7FD823A0AEA9E483E57EB1844C85A8296` |

## 3. Exact identities

Use the native convention

\[
 \Delta\rho+\operatorname{div}I=S.
\]

On a non-wrapping support patch, define signed first moment

\[
 M=\sum_x x\rho_x.
\]

Summation by parts gives

\[
 \boxed{\Delta M=\sum_f I_f+\sum_x xS_x}.
\]

The same endpoint snapshots `q at a -> q at a+e` will be represented twice:

- transport: `I(a,a+e)=q`, `S=0`;
- reaction support translation: `I=0`, `S_a=-q`, `S_{a+e}=q`.

Both must close continuity, global balance, and first moment below `1e-12`,
while the current and source norms remain different.

## 4. Registered arms

1. **Reaction-free rest:** `L=9`, both polarities, all six face directions,
   native wave propagation plus state-flux coupling, movement enabled, all
   reactions and forces disabled, 32 ticks. Velocity, remainder, displacement,
   movement events, and reaction events must remain exactly zero.
2. **Ballistic sensitivity:** the same 12 arms with initialized speed
   `C_SPEED/2`, movement only, 24 ticks. Every arm must produce at least three
   legitimate hops.
3. **Ledger discriminator:** both polarities, six face directions, and three
   translated copies: 36 transport and 36 reaction fixtures, giving 72 moment
   identities. All residuals must be at most `1e-12`; endpoint snapshots must
   be identical; transport must have current norm one/source norm zero and the
   reaction ledger current norm zero/source norm two.
4. **Stale-kinematics reachability:** 12 live CPU arms. A moving manifested
   site must evaporate within 256 ticks under the frozen stochastic rule; its
   velocity and remainder must remain bit-exact in the void; deterministic
   supercritical genesis of both polarities must remanifest the site with those
   same kinematics. No force or movement phase is enabled.

## 5. Verdicts

- `TRANSPORT_REACTION_FRONT_DISTINGUISHED`: all exact ledger gates pass.
- `REACTION_FRONT_NOT_PARTICLE_WORLDLINE`: support translation using nonzero
  `S` is not promoted to transported conserved matter.
- `STALE_VOID_KINEMATICS_CONFIRMED`: the live evaporation/genesis cycle
  reuses velocity/remainder stored on a void voxel.
- `RECIPROCAL_NATIVE_PARTICLE_MOTION_STILL_CLOSED`: reaction-free rest matter
  has no native field-to-matter impulse and no selected force is promoted.
- `INVALID`: any frozen hash changes, any gate is relaxed, or any observer
  changes production state.

This gate does not prohibit a dissipative traveling structure. It prohibits
calling endpoint support motion a conserved particle worldline without an
event-resolved current and a reciprocal energy/action ledger.
