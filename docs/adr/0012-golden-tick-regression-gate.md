# 0012 — Golden-tick regression gate (Phase 4 physics-touching extractions)

**Status:** Accepted

## Context

Phase 4 of the refactor sweep decomposed the four fat phase methods of
`RenderBridge` (`phase_write` 286 LOC, `phase_forces` 239 LOC, `phase_read`
142 LOC, `phase_movement` 114 LOC) into focused free-function TUs under
`engine/src/render_bridge_phases/`. This is the highest-risk JS-or-C++
extraction in the sweep — these methods carry the load-bearing physics
of the engine. A subtle reordering or off-by-one would silently change
energy conservation, force directions, particle motion, etc. Test suites
that look for "obvious" failures wouldn't catch a 10⁻⁹ drift accumulating
over 100 ticks into a visible bug elsewhere.

## Decision

Before any extraction begins, land a **bit-exact byte-hash regression
test** that fingerprints 100 ticks of a deterministic scenario. Every
subsequent commit must reproduce the hash exactly.

`engine/tests/test_render_bridge_golden.cpp`:
- `RenderBridge(L=17)`, force CPU, seed RNG with 42
- Fixed toggle profile (Logic6-like) honoring `validate()` deps
- Inject 3 manifested particles + 1 flux pulse at known coords
- Run exactly 100 ticks
- 64-bit FNV-1a hash over: every voxel's state + flux + wave_vel +
  velocity, every `EnergyAudit` field, manifested-site state
- Historical acceptance pin (superseded): assert
  `hash == 0xb604d81a3d79366eULL` (re-baselined for the
  audit m1 `gauss_violation` scope fix — per-voxel state/flux/wave_vel/velocity
  byte-identical, only the two gauss audit scalars changed; re-pinned
  after OpenMP race fixes in poisson_solvers.cpp + phase_write.cpp;
  prior pins were `0xcd957b601d47868a` @ L=16, then `0xebaa6f314f66db3f` and
  `0x56fa28acb5b9fe88` @ L=17)

CTest label `golden`; ~0.22s wall at L=17.

## Consequences

- (+) Phases 4a/4b/4c each extracted hundreds of LOC of physics code
  with bit-exact preservation, verified at commit time
- (+) Phases 5/6/7 also held the gate (CUDA split, toggle table refactor,
  test infra extraction)
- (+) Regression target for any future physics-adjacent change
- (−) Adding new physics requires a deliberate gate-rebaseline commit
  (capture new hash → freeze) before extraction work; this is by design

**Scoping caveat (added 2026-07-01 — red-team-confirmed gap: this gate has
been described corpus-wide with the unqualified word "physics," which
overstates its actual coverage).** The frozen scenario (L=17, 100 ticks,
seed 42, Logic6-like toggle profile) runs with roughly 14 phenomenological
subsystems toggled OFF by default (color/strong force, gravity, Langevin
thermostat, weak transmutation, dual-substrate, and others — see
`engine/include/ftd/term_toggles.h`). A regression in any OFF-by-default
subsystem, or any behavior that only manifests at L>17 or beyond 100 ticks,
passes this gate untouched. "The golden hash is preserved" means exactly
what it says — the specific fingerprinted quantities at this one frozen
configuration are byte-identical — not that "physics" in the unqualified,
general sense is verified. Campaigns exercising a toggled-off subsystem
need their own regression coverage; this gate does not provide it.

**Amendment (2026-07-02, engine revision program 0.5): multi-profile gate.**
The scoping caveat above is now partially closed. The hash-fold harness was
extracted verbatim to `engine/tests/support/golden_hash.h` (extraction
verified bit-exact against `0xb604d81a3d79366e`), and a SECOND pinned
profile landed: `test_render_bridge_golden_default.cpp` — identical harness
geometry but ZERO toggle writes, i.e. the `TermToggles{}` shipping defaults
(dual_substrate, selective_damping, weak_transmutation, damping, gravity,
lorentz_force ON), folded with the EXTENDED hash (original fields +
per-voxel `flux_L/R`, `wave_vel_L/R`, `latency`). Pinned
`GOLDEN_HASH_DEFAULT = 0x115a6350fcbe39a0` (3 consecutive runs + OMP=1
identical). Consequences: (a) the default-ON extension paths are now
bit-exact-gated; (b) changing any toggle DEFAULT now moves this hash, so
default changes require a stated rebaseline commit; (c) the re-baseline
policy applies per-profile — each pinned constant is independent. Further
profiles (boundary modes, L=9, GPU) are registered under the same policy as
they land (revision tickets 0.6/0.7).

**Amendment (2026-07-02, engine revision program 0.9 option a): gauge golden
profile.** The SU(2)/SU(3) gauge sector was wired into the tick behind
`su2_gauge`/`su3_gauge` (default OFF — every prior pin verified bit-identical
before and after wiring). New pinned profile in `test_gauge_links.cpp`:
`GAUGE_GOLDEN_HASH = 0xa4dec20d1dd94ec8` — the L=17 / seed-42 / 100-tick
harness with BOTH gauge toggles ON, links seeded by the standard deterministic
perturbation (`tests/support/gauge_test_utils.h`; identity links are exactly
stationary under the staple update, so the profile must start off the fixed
point), folded over ALL link variables (`hash_all_links`), NOT over the
substrate. The substrate fold is asserted UNCHANGED vs defaults in the same
test (the sector is write-only — nothing downstream consumes the links), so
this profile gates the link dynamics without spending any substrate golden
surface. Captured on MSVC `/fp:precise`, stable ×3 + OMP_NUM_THREADS=1, and
reproduced bit-identically on WSL2-gcc (`-ffp-contract=off`). GPU behavior is
gated by `test_gauge_gpu_parity` (element-wise CPU/GPU tolerance ~1e-15 +
bit-exact GPU run-to-run determinism), not by a pinned GPU link hash — the
CPU↔GPU FMA-contraction and product-association differences are documented in
that test.

**Amendment (2026-08-18): current clean-profile split pins.**
The current pins owned by `test_render_bridge_golden.cpp` are:
`GOLDEN_HASH = 0xc54ffbeda5a3ea63`,
`GOLDEN_STATE_HASH = 0xe9633be07656e741`, and
`GOLDEN_AUDIT_HASH = 0x48bd8b3fc2efdba3`. The state and audit folds distinguish
trajectory changes from diagnostic-only changes; the combined fold covers both.
These three pins apply only to that test's frozen L=17, seed-42, 100-tick toggle
profile and its folded fields. They do not cover off-profile toggles, larger
lattices, or longer horizons. Other golden profiles retain independent pins and
scope. All earlier hashes above remain as superseded provenance, not current
pins.

## Alternatives considered

- Hand-rolled per-quantity assertions — rejected: hashes catch
  permutation bugs and byte-level corruption that field-level checks miss
- GPU determinism — rejected: cuRAND non-determinism and floating-point
  reduction order leak in. CPU forcing with deterministic SplitMix64 RNG
  is the only stable substrate for a strict gate.
- Larger lattice (L=64) — rejected: 0.20s wall at L=16 keeps the gate
  cheap enough to run on every commit

## References

- Files: `engine/tests/test_render_bridge_golden.cpp`,
  `engine/CMakeLists.txt` (`ftd_add_test render_bridge_golden LABELS unit;golden`)
- Cross-refs: CONTRACTS.md §5 (corrected 2026-07-01 — was mis-cited as "§12", a section number that does not exist in the current 6-section CONTRACTS.md; the golden hash is actually documented under §5 Telemetry Contract), ADR-0008 (R1-R5 phase extraction precedent)
