# PREREG — Terminal topological-charge transport test

**Prospective claim ID:** FTD-0398 (registry rechecked at lock time; FTD-0397 is the current maximum).  
**Tag:** [PRE-REGISTRATION — ENGINE MEASUREMENT] · LOCK-STD v1 · git tag `preregister-topological-charge-transport-v1`.  
**Parent:** FTD-0392 and `PREREG_HEDGEHOG_CHARGE_ROBUSTNESS_v1.md`. This test extends that instrument without changing its Berg–Lüscher charge convention. It is the final shell-geometry campaign for this mass-anchor route.

## 1. Frozen question and protocol

Did the charge that was radial at injection and trivial on the `R=1` shell at freeze move outward, or did the field cross through `|J|=0` and lose the charge?

The instrument reuses the exact FTD-0392 A/C/E seeds and dynamics: `A_baseline`, `C_hot`, and `E_cold`; `L=17`; CPU-forced `RenderBridge`; `wave_propagation`, `coupling`, `gauss_projection`, `genesis`, `damping`, and `selective_damping` ON; every other toggle OFF; `FTD_FORCE_GPU` unset. For each seed it uses a deterministic discovery pass to locate the first manifested site, then reruns the identical seed and measures shells about that fixed site from post-injection tick `t=0` through `t=8` inclusive. The discovery and measurement passes must both first manifest at tick 2 and at the same site.

The shell at radius `R` has exactly the six vertices `center +/- R e_x`, `center +/- R e_y`, `center +/- R e_z`, with the same eight outward-oriented triangular faces used by FTD-0392. Radii are exactly `R=1..6`. No cuboctahedral, interpolated, adaptive, or alternative shell is admissible.

For every `(seed,t,R)`, record the frozen CSV schema:

`seed,tick,radius,Q,min_j,valid,e_half,manifest_x,manifest_y,manifest_z`

Here `min_j` is the minimum `|J|` of the six vertices; `valid=1` iff every vertex has `|J|>1e-12`; `e_half=0.5*sum |J|^2` over the full lattice. Before manifestation the reported manifested coordinates are `(-1,-1,-1)`; the shell center nevertheless remains the deterministically discovered eventual manifestation site.

## 2. Frozen instruments

- Engine campaign: `engine/tests/campaign_topological_charge_transport.cpp`, SHA256 `44f8965d167231bad7019b2bbf79fc8f23356dc26a87f6f29d4db3bb11cae12c`.
- Recomputing verdict verifier: `scripts/proofs/verify_topological_charge_transport.py`, SHA256 `a73e9036f1946039e9eb0496bb7f0719669d020cec264f552e9bb2e2550906b2`.
- Canonical build: WSL2 Ubuntu-22.04 `engine/build_wsl`, target `campaign_topological_charge_transport`; `FTD_FORCE_GPU` must be unset. The target must be run twice and the CSV files must be byte-identical.

The verifier checks the exact 162-row grid, schema, `valid`/field-floor equivalence, the FTD-0392 freeze gate, boundary safety, and the frozen outcome predicates. It does not estimate a mass, search parameters, or change the charge convention.

## 3. Correctness gates

Correctness gates have absolute precedence; any failure gives **INVALID**.

| Gate | Frozen requirement |
|---|---|
| G1 | preregistration census GREEN; FTD-0398 next at tag cut |
| G2 | synthetic radial and inverted fields return `Q=+1` and `Q=-1` to `1e-12` for every `R=1..6` |
| G3 | common rigid rotation and independent positive vertex rescalings preserve `Q` to `1e-12` for every radius |
| G4 | all six radii are in bounds around the manifested site |
| G5 | A/C/E first manifest at tick 2 with exactly one site; `R=1` freeze charge satisfies `|Q|<=5e-9`, reproducing FTD-0392 |
| G6 | freeze `e_half` reproduces A/C/E values `1.368676308503`, `5.828246462835`, `0.540720277788` within `1e-9` |
| G7 | effective toggles match §1, execution is CPU-forced, and `FTD_FORCE_GPU` is unset |
| G8 | duplicate canonical executions are byte-identical; CSV grid/schema/finiteness and `valid <=> min_j>1e-12` pass |

Vacuity controls: G2/G3 require the instrument to distinguish sign and preserve known degree rather than return zero universally. G5 binds the new instrument to the prior real-engine measurement. Undefined shells cannot count as zero and cannot satisfy COLOCALIZED or TRANSPORTED. A merely nonzero value below the frozen charged bands supplies no topological evidence.

## 4. Frozen predicates and precedence

After all correctness gates pass, evaluate the following mutually exclusive rows in order:

1. **COLOCALIZED:** for every seed, one fixed radius `R<=2` is defined and has `|Q|>=0.95` at freeze `t=2` and at all four post-freeze ticks `t=3..6`.
2. **TRANSPORTED:** for every seed, let `r(t)` be the smallest defined radius with `|Q|>=0.5`. Starting from the first tick where `r(t)` exists, a later `r(t)` increases by at least two radii; from the first such outward move through `t=8`, no defined `R<=2` again has `|Q|>=0.5`.
3. **ZERO-CROSSING/DESTROYED:** for every seed, some radius that previously had defined `|Q|>=0.5` later becomes undefined; after that crossing, at least one enclosing shell is defined and every subsequently defined shell at that radius or larger through `t=8` has `|Q|<=0.05`.
4. **UNDERDETERMINED:** all gates pass but none of the three predicates above holds. This includes undefined, non-integer, intermittent, mixed-seed, and ambiguously transported behavior not satisfying the destroyed criterion.
5. **INVALID:** any correctness gate fails; evaluated before rows 1–4 despite its display position.

The ordered predicates are disjoint by precedence. COLOCALIZED cannot be TRANSPORTED because its fixed inner shell remains charged. TRANSPORTED precedes destruction if a record could otherwise contain both an outward move and a later zero crossing. ZERO-CROSSING requires the explicit definition-boundary witness and a non-vacuous later enclosing-shell set. Normative thresholds and this precedence outrank prose.

## 5. Licensed interpretation

- COLOCALIZED alone may open a separately locked analytic energy-bound proof.
- TRANSPORTED or ZERO-CROSSING/DESTROYED closes this topological charge as a local rest-mass anchor.
- UNDERDETERMINED supplies no mass evidence and licenses no new shell design.
- INVALID supplies no physical verdict and requires only repair of a failed correctness gate under a new lock if scientifically necessary.

No result derives mass. No particle label, `M_REST`, `m_e`, `alpha^11`, dispersion relation, or calibration is used. After this campaign, no alternative shell geometry may be designed for this route. FTD-0096 and every framework commitment retain their tags.

## 6. Execution window and executor

Executor: the current Codex repository session on branch `codex/invariant-quotient-roadmap-2026-07-20`. Execution window: from creation of tag `preregister-topological-charge-transport-v1` through exactly 72 hours after that tag's creation instant.

**LOCKED CONTENT ENDS HERE.** Normative changes require v2 before any execution.
