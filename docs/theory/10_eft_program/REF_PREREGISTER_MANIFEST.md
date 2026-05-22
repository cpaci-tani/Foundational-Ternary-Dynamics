# Pre-Registration Manifest

**Purpose:** single authoritative table mapping every pre-registered
FTD measurement to (a) the git tag committed BEFORE the run, (b) the
commit SHA the tag points at, (c) the script and any flags used, (d)
the output directory the campaign emits to, and (e) the analysis
document that interprets the result.

**Why it lives here:** the `engine/results/` gitignore default makes
new campaign outputs **local-only** by default — analysis docs cite
result paths that won't exist in a fresh clone. This manifest gives
posterity a recipe for reproducing each campaign from a tagged
commit.

**Discipline:** SHA256 of every pre-registered measurement script is
recorded in the corresponding analysis document (e.g.
`AUDIT_LOOK_ELSEWHERE_RESULTS.md`). The git tag locks the SHA at
pre-registration time. To verify a tag's commit hasn't drifted, run:

```sh
git rev-list -n1 <tag-name>     # commit SHA
git tag -l <tag-name>            # tag listing
```

---

## Pre-registered campaigns (2026-04-27 / 2026-04-28 cycle)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0097** look-elsewhere scan | `preregister-look-elsewhere-scan-v1` | `f11dcaa` | `tools/scan_look_elsewhere.py` | `--epsilon 1e-3,1e-4` | `engine/results/look_elsewhere_2026-04-27/` | [`AUDIT_LOOK_ELSEWHERE_RESULTS.md`](../07_assessment/AUDIT_LOOK_ELSEWHERE_RESULTS.md) |
| **FTD-0105** lemniscatic 2-sphere test | `preregister-lemniscatic-v1` | `7bc2185` | `engine/build_wsl/benchmark_black_hole_thermo` | `--lemniscatic-mode` | `engine/results/lemniscatic_*` | LEDGER row FTD-0105 |
| **FTD-0106** G\*/π asymmetry scan | `preregister-gstar-asymmetry-v1` | `edd1349` | (theory-only catalog committed; engine measurements deferred) | n/a | n/a yet | LEDGER row FTD-0106 |
| **FTD-0107** emergent-spectrum L=64 G1 | `preregister-emergent-spectrum-g1` | `37ea371` | `engine/build/campaign_emergent_spectrum_2026-04-27` | `--L 64 --output-dir=engine/results/emergent_spectrum_2026-04-27_L64 --N-samples 5 --N-seeds 5` | `engine/results/emergent_spectrum_2026-04-27_L64/` | [`ANALYSIS_EMERGENT_SPECTRUM_G1.md`](archive/campaign_complete/ANALYSIS_EMERGENT_SPECTRUM_G1.md) |
| **FTD-0107** emergent-spectrum L=128 G2 | `preregister-emergent-spectrum-g2` | (this commit) | `engine/build_wsl/campaign_emergent_spectrum_2026-04-27` | `--L=128 --seeds=5 --samples=50 --burn=200 --stride=50 --output-dir=engine/results/emergent_spectrum_2026-04-28_L128/` | `engine/results/emergent_spectrum_2026-04-28_L128/` | [`PROTOCOL_EMERGENT_SPECTRUM_G2.md`](archive/campaign_complete/PROTOCOL_EMERGENT_SPECTRUM_G2.md) (analysis pending) |

The launcher script `engine/tools/run_emergent_spectrum_g1.sh` wraps
the FTD-0107 invocation; see `commit a0983ca` for the script body.

## FQCR (Finite Quarter-Conjugacy Recurrence) Model IV uniqueness scan (2026-05-06; scan queued)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0143** FQCR (4,6;3,2) uniqueness scan | `preregister-fqcr-quotient-uniqueness-v1` | `557593e` | (scan-runner not yet authored; sketched in pre-reg §5 — extends `tools/scan_look_elsewhere.py` with FQCR-readout inner loop) | (k, d, ℓ, m) ∈ {2,...,8}^4; tolerances {1e-3, 1e-4, 1e-5, 1e-6}; targets = 20 FTD-0097 spine targets | `engine/results/fqcr_quotient_uniqueness_2026-05-06_l_scan/` | `PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md` (pre-reg) → `ANALYSIS_FQCR_QUOTIENT_UNIQUENESS.md` (post-launch) |

Pre-reg SHA256: `94bc4cd74cbf90017996bf90a19f0bbeaae7937f8c47a6317b3409f58c268a1f`.

Backend: pure Python via mpmath (no engine GPU required). Scan execution ~1-2 hours wall on a single CPU core.

When launching: confirm `git rev-list -n1 preregister-fqcr-quotient-uniqueness-v1` resolves to `557593e` and that the scan-runner's content hash is recorded against this anchor at runtime per FTD-0097's precedent.

## Alpha arithmetic generativity Test 4 (2026-05-20; candidate inventory queued)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0185** alpha arithmetic generativity | `preregister-alpha-arithmetic-generativity-v1` | (pending commit/tag) | none; desk-audit target declaration gate | No numerical search. Candidate must publish target declaration before comparison; `x_- ≈ N_c` excluded as the prize | n/a until a candidate declaration exists | `PREREG_ALPHA_ARITHMETIC_GENERATIVITY_v1.md` → candidate declaration or no-candidate report |

Pre-reg SHA256: `b222c2a0873fa21dcf28b87111ecab5de8753ec3a4a38e3074d038b6f3d06a27`. This pre-registration locks the rules for Test 4, not a measurement script.

## R3a operator-mixing L-scan (2026-05-05; campaign queued)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0140** R3a operator-mixing L-scan | `preregister-operator-mixing-l-scan-v1` | `f3fa700` | `engine/build_wsl/campaign_operator_mixing_2026-04-26` | `--L <64\|96\|128> --b <2\|4> --inj-mult 1.0` (6 configs total) | `engine/results/operator_mixing_2026-05-05_l_scan/L<L>_b<b>/` | [`PREREG_OPERATOR_MIXING_L_SCAN_v1.md`](archive/campaign_complete/PREREG_OPERATOR_MIXING_L_SCAN_v1.md) (pre-reg) → `ANALYSIS_OPERATOR_MIXING_L_SCAN.md` (post-launch) |

Pre-reg SHA256: `290005066803b2cada8be9820c50f35ef3f810ae61fba53d436d9a393a5c2f0d`.

Backend anchor: HEAD `00f41fe` post BH-F5/F8/F9 RNG portability closure (commits `c1a4f88` + `c8e03a5`). Per-voxel CPU↔GPU bit-exact at unit mass under stochastic toggles. The campaign launches when GPU is clear (currently at 94% external contention; user picked "pre-register now, launch later" on 2026-05-05).

When launching: confirm `git rev-list -n1 preregister-operator-mixing-l-scan-v1` resolves to `f3fa700` and that the campaign binary's commit-sha matches that anchor.

## Earlier campaigns (pre-2026-04-27, no pre-reg tag yet)

These campaigns precede the pre-registration discipline (introduced
2026-04-27) and don't have `preregister-*` tags. Their analysis
documents still cite specific commit ranges + result directories;
manually trace via `git log --follow` if reproducing.

| FTD ID | Date | Output dir | Analysis doc |
|---|---|---|---|
| FTD-0098–0102 operator-mixing baseline | 2026-04-26 | `engine/results/operator_mixing_2026-04-26/` | LEDGER rows |
| FTD-0103 continuum-limit | 2026-04-26 | `engine/results/baseline_2026-04-26/` (campaign_continuum subset) | LEDGER row FTD-0103 |
| FTD-0104 topology atlas | 2026-04-26 | `engine/results/baseline_2026-04-26/` (campaign_topology subset) | LEDGER row FTD-0104 |
| FTD-0093 Mechanism C closure | 2026-04-27 | `engine/results/baseline_2026-04-26/bcc_band_spectrum/` | [`AUDIT_LINK8_CLOSURE.md`](archive/closed_negative/AUDIT_LINK8_CLOSURE.md) cross-ref |

---

## Structural / dynamical discriminator -- boundary theorem Stage 1 (2026-05-20)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0186** structural/dynamical discriminator | `preregister-structural-dynamical-discriminator-v1` | `75ebe56` | `scripts/proofs/proof_structural_dynamical_partition.py` | desk classification of the LEDGER record; no numerical search | n/a (classification is a theory doc) | `PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v1.md` (pre-reg) -> `FOUND_STRUCTURAL_DYNAMICAL_DISCRIMINATOR.md` (Stage-1 result) |

Pre-reg SHA256: `a6562dca56154401e7a2cfb8785266cef0d5b4ee70d3755797762ddffa3e538d`.

When auditing: confirm `git rev-list -n1 preregister-structural-dynamical-discriminator-v1` resolves to `75ebe56`. The discriminator definition (pre-reg S2) and falsifier (S4) were locked before the classification was run; a v2 is required to tighten the S4-A1 falsifier wording (see `FOUND_STRUCTURAL_DYNAMICAL_DISCRIMINATOR.md` S5).

---

## Finite neutral lock -- finite-closure SM-shadow audit (Q10) (2026-05-22)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0190** finite neutral lock (Q10) | `preregister-finite-neutral-lock-v1` | tag `preregister-finite-neutral-lock-v1` | [`audit_finite_neutral_lock.py`](../../../scripts/proofs/audit_finite_neutral_lock.py) -- frozen-catalog enumeration (pre-reg §4); no numerical search, no near-miss scan | n/a | n/a (desk audit) | [`PREREG_FINITE_NEUTRAL_LOCK_v1.md`](../08_structural/PREREG_FINITE_NEUTRAL_LOCK_v1.md) (pre-reg) -> [`AUDIT_FINITE_NEUTRAL_LOCK.md`](../08_structural/AUDIT_FINITE_NEUTRAL_LOCK.md) (result: UNDERDETERMINED) |

Pre-reg SHA256: `41c3f86584270d59fd25736bfec3cee3efb6a656d34f12be44b93272e57ae346`.

When auditing: confirm `git rev-list -n1 preregister-finite-neutral-lock-v1` resolves to the commit that introduced `PREREG_FINITE_NEUTRAL_LOCK_v1.md`, and that the file's SHA256 still matches the value above (`sha256sum docs/theory/08_structural/PREREG_FINITE_NEUTRAL_LOCK_v1.md`). The question Q10, definitions D1-D6, the FROZEN admissible search space (pre-reg §4), the (1,2)_{1/2} benchmark (§5), the three pre-blessed outcomes (§6), and the falsifier F-a..F-e (§7) were all locked before the audit was run. The pre-reg doc lives in `08_structural/` (the structural cluster), not in `10_eft_program/`.

---

## Colour-singlet rank -- electroweak-rank audit (Q11) (2026-05-22)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0191** colour-singlet rank (Q11) | `preregister-colour-singlet-rank-v1` | tag `preregister-colour-singlet-rank-v1` | [`audit_colour_singlet_rank.py`](../../../scripts/proofs/audit_colour_singlet_rank.py) -- frozen-catalog enumeration (pre-reg §4 = Q10 §4); no numerical search | n/a | n/a (desk audit) | [`PREREG_COLOUR_SINGLET_RANK_v1.md`](../08_structural/PREREG_COLOUR_SINGLET_RANK_v1.md) (pre-reg) -> [`AUDIT_COLOUR_SINGLET_RANK.md`](../08_structural/AUDIT_COLOUR_SINGLET_RANK.md) (result: UNDERDETERMINED) |

Pre-reg SHA256: `08c55b8e060332a2311be7ae6dedf5d48cbf1af861db627195d1dd2f8a886dbe`.

When auditing: confirm `git rev-list -n1 preregister-colour-singlet-rank-v1` resolves to the commit that introduced `PREREG_COLOUR_SINGLET_RANK_v1.md`, and that the file's SHA256 still matches the value above. Q11 is the successor to Q10 (FTD-0190): its verdict decides whether FTD-0190 lifts to FOUND, stays UNDERDETERMINED, or closes negative. The question, definitions D1-D6, the frozen catalog (§4), the benchmark (§5), the three outcomes (§6), and the falsifier F-a..F-f (§7) were all locked before the audit was run.

---

## How to add a new pre-registration row

1. **Pre-register** before measurement:
   - Decide the script + flags + expected outcome.
   - Commit the script (and any pre-registration prose). Compute its
     SHA256 (`sha256sum tools/<script>.py` or equivalent for C++
     campaigns) and record it in the campaign's pre-reg analysis doc
     stub.
   - Create a lightweight git tag pointing at the pre-reg commit:
     ```sh
     git tag preregister-<name>-v1 -m "Pre-reg for FTD-NNNN: <description>"
     git push origin preregister-<name>-v1
     ```

2. **Run** the measurement against the tagged commit. Save output to
   `engine/results/<campaign_name>_YYYY-MM-DD/`. The directory is
   gitignored by default; track only the analysis-doc-cited subset
   with `git add -f <path>`.

3. **Add a row to this manifest** populating all six columns. Cite
   the analysis doc and the LEDGER row.

4. **Don't retroactively pre-register**. If a measurement was run
   before the tag, don't backfill — record it in the "earlier
   campaigns" table above instead. The discipline only works if
   pre-registration genuinely precedes measurement.

---

## Verification recipe (reproducing a tagged campaign from scratch)

```sh
# 1. Check out the pre-registration commit (read-only inspection).
git checkout <pre-reg tag or commit SHA>

# 2. Verify script SHA matches what the analysis doc recorded.
sha256sum <script>      # compare against analysis doc

# 3. Build and run.
#    (Native CTest build / WSL2 build / WASM build — per CLAUDE.md.)

# 4. Compare output to analysis doc's reported numbers.
#    Bit-for-bit reproducibility is not guaranteed across machines
#    (RNG seeding modulo platform), but statistical equivalence of
#    the reported summary statistics is.

# 5. Return to main:
git checkout main
```

---

## Cross-references

- [`CLAUDE.md`](../../../CLAUDE.md) §"NEW INFRASTRUCTURE 2026-04-27" —
  introduces the pre-registration discipline.
- [`docs/WHERE_WE_LEFT_OFF.md`](../../WHERE_WE_LEFT_OFF.md) §10 —
  bird's-eye assessment, includes the structural-bridge gap that
  motivates further pre-registered campaigns.
- [`07_assessment/LEDGER.md`](../07_assessment/LEDGER.md) — single
  source of truth for claim status; each FTD-NNNN row cross-references
  its pre-reg tag (when present) and analysis doc.
- [`CHANGELOG.md`](../../../CHANGELOG.md) "Measurement output → pre-
  registration tag mapping" — short summary table mirroring this
  manifest's rows for the 2026-04-27 cycle.
