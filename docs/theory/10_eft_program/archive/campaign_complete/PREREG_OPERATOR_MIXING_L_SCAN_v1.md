# PRE-REGISTRATION — Operator-Mixing Matrix L-Scan Extension (R3a)

**Tag:** [PRE-REGISTRATION]
**Date:** 2026-05-05 (committed before any L ≥ 64 measurement runs)
**Hash-lock target tag:** `preregister-operator-mixing-l-scan-v1`
**LEDGER row reservation:** FTD-0140 (R3a primary), FTD-0141 (R3a Wilson-eigendecomposition extension), FTD-0142 (R3b dim-6 operator coefficients).
**Supersedes:** none. Extends — does **not** replace — `PROTOCOL_OPERATOR_MIXING_MATRIX.md` (FTD-0098/0099/0100, L=16/32 baseline). The L-scan campaign uses the same operator basis, blocking definition, and matrix assembly; only ensemble size + L grid + b grid are extended.
**Author:** FTD EFT program (R3a sub-phase of the FTD-EFT roadmap; see `~/.claude/plans/we-fixed-a-lot-composed-reef.md`).
**Companion docs:** `PROTOCOL_OPERATOR_MIXING_MATRIX.md`, `SPEC_OPERATOR_BASIS_COMPLETE.md`, `SPEC_FTD_NATIVE_BLOCKING_MAP.md`, `DECISION_FIELD_BASIS.md`, `DECISION_GAUSS_REPRESENTATION.md`, `engine/docs/DESIGN_RNG_PORTABILITY.md`.

> **Pre-registration discipline.** Every threshold, lattice size, seed value, sample count, and acceptance criterion below is committed *before* any L ≥ 64 measurement is run. This document will be tagged with `git tag preregister-operator-mixing-l-scan-v1 <commit-hash>` immediately after commit, locking the SHA256 content hash. No post-hoc edits to thresholds or expectations are permitted; surprising measurements are reported honestly and theory must explain them.

---

## §1 — Why this pre-registration

R3a is the first sub-phase of the central R3 deliverable of the FTD-EFT roadmap: write down the explicit nonlinear blocked $S_\text{eff}[J, s]$. R3 cannot proceed without continuum-limit-leading data on the operator-mixing matrix beyond the existing L=16/32 baseline. This pre-registration locks the L-scan extension so measurements at L ∈ {64, 96, 128} are predictively committed — not retrofitted to whatever came out.

The extension is meaningful because:

- The 2026-04-26 baseline (FTD-0098/0099/0100) measured M_ab(b=2) at L=16/32 only. AUDIT_CONTINUUM_LIMIT noted "cond(S) monotonically improving (factor 18 over L=16→L=64)" but the L=64 sample size (2 seeds, 8 b=2 blocks) was bootstrap-noise-limited. Per `AUDIT_OPERATOR_SPECTRUM.md`'s 2026-04-25 note, all five operators classified as "relevant" (Δ < D = 4) at L=32 — the marginal/irrelevant tier was not recovered, and "full classification requires L ≥ 64 + multi-scenario ensemble".
- Post-2026-05-05 the engine has bit-exact CPUGPU stochastic operations via the SplitMix64 portability fix (commits `c1a4f88` + `c8e03a5`). Measurements are now per-voxel deterministic rather than ensemble-equivalent; tighter tolerances are defensible.
- R3d's $S_\text{eff}$ write-up needs at least 3 L points to fit the leading-order Wilson coefficient drift; 4 points (16, 32, 64, 96, 128) gives one redundant constraint as a sanity-check.

---

## §2 — Pre-registered scope

**Lattice sizes.** Production: L ∈ {64, 96, 128}. (L=16 and L=32 are inherited from FTD-0098/0099/0100.)

**Blocking factors.** b ∈ {2, 4}. Both factors per L. No b=8 in this pre-reg (would require L ≥ 64; 8³=512 b=4 cells at L=64 is the minimum).

**Operator basis.** Six operators per `PROTOCOL_OPERATOR_MIXING_MATRIX.md` §2 (J·J, (∇·J)², (∇×J)², J·∇(∇·J), (J·J)², s·s). **No additions, no deletions in this pre-reg.** The 12 dim-6 operators from `SPEC_EFT_RECOVERY_PROGRAM.md` §6 are explicitly held for the R3b sub-phase pre-reg, NOT this one.

**Backend.** GPU (WSL2 RTX 5090) via `engine/build_wsl/`. CPU run is a parity reference at smoke-only (L=8) for cross-validation; production data is GPU-canonical per `DECISION_GAUSS_REPRESENTATION.md`.

**Field basis.** Collocated $(s, J)$ at lattice vertices per `DECISION_FIELD_BASIS.md`.

**Calibration.** Boundary-injection at `inj-mult=1.0` per FTD-0100's F2 closure. The previous 3.0× injection saturates Var(s²); 1.0× is the canonical operating point.

---

## §3 — Pre-registered ensemble parameters per L

| Parameter | L=64 | L=96 | L=128 |
|---|---|---|---|
| Burn-in N_BURN | 200 | 250 | 300 |
| Samples per seed N_SAMPLES | 40 | 40 | 40 |
| Sample stride | 5 ticks | 5 ticks | 5 ticks |
| Number of seeds N_SEEDS | 5 | 5 | 5 |
| Total samples N_total | 200 | 200 | 200 |
| Per-seed RNG seed | 0xF10412E5 + s·0x100 | (same family) | (same family) |
| Toggles | wave + gauss + genesis + langevin | same | same |
| Langevin T | 0.005 | 0.005 | 0.005 |
| Langevin γ | 0.02 | 0.02 | 0.02 |
| Dual substrate | OFF | OFF | OFF |
| Initial seed flux | $(K_\text{GENESIS}, 0, 0)$ at lattice center, `inj-mult=1.0` | same | same |
| Estimated wall time per L (RTX 5090, clean) | ~6h | ~12h | ~24h |

Total estimated wall time for the full L-scan: ~42h GPU compute. Under 50% external contention (typical), ~84h. Pre-registered tolerance: campaign may take up to 7 days from launch under contention; failures to complete in 14 days trigger a methodology review.

The seed family `0xF10412E5 + s·0x100` matches FTD-0098/0099/0100 exactly. Same seeds at every L; the same s (∈ {0,...,N_SEEDS-1}) gives a different physical run at different L (lattice initialisation differs by L) but the SplitMix64 stream salting is consistent.

**Smoke runs** (L=8, before each production L) confirm the campaign binary builds and emits well-formed CSV. Smoke is not part of the pre-registered measurement; production is L ∈ {64, 96, 128}.

---

## §4 — Pre-registered acceptance criteria

For each (L, b) configuration, the following criteria must hold for the result to count as measurement-grade. Failure of any criterion at any (L, b) is reported honestly and triggers the [PARTIAL] tag rather than [MEASUREMENT].

### §4.1 Numerical integrity

- $\max_{i,t}|\nabla\cdot\mathbf{J} - \rho| \leq 10^{-7}$ at end of every tick on every snapshot (Gauss-residual tolerance per `DECISION_GAUSS_REPRESENTATION.md`).
- Bootstrap resampling (1000 iterations) produces convergent variance: bootstrap-stderr / sample-stderr ratio ∈ [0.9, 1.1] at every entry of M_ab.
- Wilson eigenvalues are real and finite at every (L, b).

### §4.2 Theorem-grade diagonals

Per `THEOREM_BLOCKING_DIAGONAL_IDENTITIES.md`:

- $M_{JJ}(b)$ should be exactly $b^3$ (volume-weighted scaling) ± machine epsilon.
- $M_{(J\cdot J)^2,(J\cdot J)^2}(b)$ should be exactly $b^3 \cdot $ baseline at the canonical operating point.

These are theorem-locked; their failure would indicate either a numerical bug or a violation of the blocking-diagonal identity itself (unlikely; the theorem is robust).

### §4.3 cond(S) monotonicity

cond(S) is expected to **monotonically decrease** with L (less finite-sample noise at larger L). Pre-registered: cond(S) at L=128 should be ≤ cond(S) at L=64. Violation flags either an unexpected operator-noise structure or a campaign-stride / sample-size mismatch.

### §4.4 Operator-dimension classification (R3c-relevant)

Per L=32 baseline, all five "non-trivial" operators (excluding s·s) classified as "relevant" (Δ < 4). The pre-registered hypothesis for L ∈ {64, 96, 128}:

- **At L = 64**: at least one operator emerges as marginal (Δ ≈ 4 ± 0.5) or irrelevant (Δ > 4.5).
- **At L = 96**: at least two operators emerge as marginal/irrelevant.
- **At L = 128**: marginal/irrelevant tier should resolve to ≥ 3 operators.

Failure of this hypothesis means "the L-scan does not separate the operator tiers cleanly under the chosen ensemble parameters." That outcome is informative — it would suggest the FTD-native operator basis is more degenerate than expected and the R3 $S_\text{eff}$ closure needs a different operator family.

### §4.5 Wilson-coefficient stability

Per FTD-0099 the b=2 vs b=4 comparison ("RG semigroup" check) failed at L ∈ {16, 32} with relerr 1.61–1.80×, attributed to bootstrap noise on the 4³–8³ b=4 grid. Pre-registered:

- At L=64 with b=4 (16³ b=4 grid), the relerr should drop below 0.5×.
- At L=128 with b=4 (32³ b=4 grid), the relerr should be ≤ 0.10×.

If achieved, this closes the FTD-0099 F5 open item ("RG semigroup test"). Failure is reported as a structural property of the FTD-native flow rather than a measurement bug.

### §4.6 Off-diagonal entries

Per FTD-0100 the off-diagonal $M_{(J\cdot J)^2, s^2}(b=2) = 6.47$ at L=16. Pre-registered: this entry's L-trend should fit a polynomial in $1/L$ to within 5% residual across L ∈ {16, 32, 64, 96, 128}.

---

## §5 — Pre-registered output artefacts

For each (L, b) configuration, the campaign writes:

```
engine/results/operator_mixing_2026-05-05_l_scan/L<L>_b<b>/
├── meta.json              — config + commit hash + git tag
├── M_ab.csv               — 6×6 mixing matrix entries with bootstrap stderr
├── eigenvalues.csv        — Wilson eigendecomp (real, sorted)
├── cond_S.txt             — condition number
├── snapshots/             — 200 native dual-cell snapshots (40 per seed × 5 seeds)
└── ANALYSIS.md            — per-config narrative summary, [PARTIAL]/[MEASUREMENT] verdict
```

The campaign-runner is `engine/tests/campaign_operator_mixing_2026-04-26.cpp` extended to take CLI flags `--L <int> --b <int> --inj-mult <double>`. If the binary does not yet support these flags, the pre-reg holds and the binary is extended (without changing its core measurement code) at run time.

Aggregated cross-L analysis lands in `ANALYSIS_OPERATOR_MIXING_L_SCAN.md` after R3a closure.

---

## §6 — Backend specification (BH-F5/F8/F9 anchor)

This pre-registration is committed **at HEAD `00f41fe`**, post the BH-F5/F8/F9 RNG portability closure (commits `c1a4f88` + `c8e03a5`). Specifically:

- Genesis Boltzmann probability uses SplitMix64 stream `voxel_uniform(seed, idx, tick, GenesisManifest)` — bit-exact CPUGPU.
- Genesis zero-curl spin fallback uses `voxel_uniform(seed, idx, tick, GenesisSpin) < 0.5 ? +1 : -1` — bit-exact CPUGPU (BH-F8 fix).
- Langevin OU noise uses `voxel_normal(seed, idx, tick, LangevinNoiseX/Y/Z)` Box-Muller per axis — bit-exact CPUGPU.

Per-voxel CPUGPU agreement under stochastic toggles holds at machine epsilon for all measurements covered by this pre-reg. The campaign runs on GPU (canonical) with CPU smoke as cross-validation.

---

## §7 — Risk register

| Risk | Severity | Mitigation |
|---|---|---|
| GPU contention under external load extends wall-clock 2–5× | Medium | Pre-registered tolerance up to 14 days from launch; failure triggers methodology review, not result-massaging. |
| Bootstrap sample-size noise dominates Wilson eigenvalue variation | Low | N_SEEDS = 5 + N_SAMPLES = 40 per seed → 200 total samples, 1000-iter bootstrap (per FTD-0098 baseline). At L ≥ 64 the b=4 grid is 16³ ≥ 4096 cells — far above the FTD-0099 noise floor. |
| Operator dimension classification doesn't separate cleanly | Medium | Pre-registered as honest [PARTIAL] outcome (§4.4). Informative either way. |
| RNG portability changes alter pre-Option-A baseline | Low | Pre-reg is committed POST-Option-A. The L=16/L=32 historical FTD-0098/0099/0100 numbers are pre-Option-A; we do NOT use them as predictions, only as structural references. The L-scan is internally consistent under the post-Option-A backend. |
| Campaign binary needs extension for L > 64 / b=4 | Low | Existing `campaign_operator_mixing_2026-04-26.cpp` accepts `--L` and `--b` (per its 2026-04-26 commit message); confirmed at this pre-reg's anchor commit. If extension is needed, it lands as a separate engine commit BEFORE measurement, with hash-lock note. |
| dim-6 operator basis (R3b) leaks into R3a | High | Explicit pre-reg scope statement (§2): six-operator basis only. Adding a seventh operator post-hoc would invalidate this pre-reg. |

---

## §8 — Hash-lock

Immediately after this file is committed:

```bash
cd /c/Users/cpaci/Desktop/ftd
git tag preregister-operator-mixing-l-scan-v1 <commit-sha>
sha256sum docs/theory/10_eft_program/PREREG_OPERATOR_MIXING_L_SCAN_v1.md
```

The SHA256 hash is recorded in `REF_PREREGISTER_MANIFEST.md` and locks the document content. Any subsequent edit to thresholds, parameters, or acceptance criteria invalidates the pre-registration; a new pre-reg (v2) must be issued before further measurement.

The git tag is a local annotation, not pushed to remote (per project's no-AI-attribution + minimal-remote-state policy). Future measurement output's `meta.json` cites this tag's commit-sha as the pre-registration anchor.

---

## §9 — What this pre-reg does NOT cover

To prevent scope creep:

- **R3b dim-6 operator measurements**: separate pre-reg, written before R3b launches.
- **R3c relevant/marginal/irrelevant classification**: produced by R3d analysis; not measured separately.
- **R3d $S_\text{eff}$ write-up**: not a measurement; no pre-reg needed.
- **R4 β(g, L) extraction**: separate pre-reg, after R3 lands.
- **R5 inter-scale work**: separate pre-reg per scale-transition.
- **Sensitivity studies**: changing T, γ, or `inj-mult` away from the canonical values produces *exploratory* data that is NOT covered by this pre-reg. Sensitivity runs use a separate "EXPLR" tag and are clearly distinguished from production.

---

## §10 — Launch authorization

User authorized R3a scope = "L ∈ {64, 96, 128}, b ∈ {2, 4} (Recommended)" via the AskUserQuestion answers on 2026-05-05. The user has *not* yet authorized the actual launch — the GPU is currently at 94% external utilization, and the user picked option (1) "Pre-register the campaign now, launch later."

This pre-reg lands the bounded-now work. The launch is queued for the next clean-GPU session.

When launching: confirm the git tag `preregister-operator-mixing-l-scan-v1` exists and that the campaign binary's commit-sha matches the pre-reg's anchor. Any drift between tag-time and launch-time content invalidates the pre-reg.
