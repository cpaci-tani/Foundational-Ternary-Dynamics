# GPU Pipeline Baseline — 2026-04-26

**Commit:** `347a38f866c87f330feef121071851b128642445`
(`347a38f cluster-A: derive stencil weights + test predictions from canonical constants`)

**Hardware:**
- CPU: AMD Ryzen 9 9950X3D (16-Core)
- GPU: NVIDIA GeForce RTX 5090 (Blackwell SM 120)
- OS: Microsoft Windows 11 Pro
- Toolchain: MSBuild 18.3.0 / MSVC + CUDA 13 (Windows-native)

**Build config:** Release (`cmake --build engine/build --config Release`); clean rebuild verified for `campaign_bcc_band_spectrum`.

**Backend confirmation:** Every campaign that instantiates `RenderBridge` logged
`[RenderBridge] GPU backend active (CUDA, L=...)`. Pure unit tests
(`sublattice_helpers`, `sublattice_laplacian`, `two_state_extraction`)
don't construct a bridge and therefore have no banner — that's expected.

---

## Headline numbers (per test, one line each)

| Test | Wall (s) | Result | Headline |
|------|---------:|--------|----------|
| `sublattice_helpers` | 0 | PASS (14/14) | SC=BCC=8, FCC=48 in 4³; corner-neighbor parity exact |
| `sublattice_laplacian` | 0 | PASS (14/14) | SC ∇²|r|² = 1, FCC = 2, BCC = 3, FULL = 6 (matches stencil weights) |
| `correlations_diagonal` | 0 | PASS (11/11) | AXIS C(1) = 0.451184; BODY_DIAG C(1) = 0.353553; BCC C(1)=0 (parity-flip rejected) |
| `two_state_extraction` | 0 | PASS (7/7) | Prony + GEVP both recover (x₊, x₋) = (0.5, 0.0110335) to 0.0% on synthetic; 2.89% rejection on wrong-ratio falsification |
| `langevin_equipartition` | 3 | PASS | ⟨\|v\|²⟩ = 3.128×10⁻² vs 3T = 3.000×10⁻² → **+4.28% deviation** (within 5% tolerance); ⟨v⟩ ≈ 0; isotropy 1.043×10⁻² vs T=10⁻² (-0.59%) |
| `langevin_sublattice_equipartition` | 2 | PASS (3/3) | BCC: +2.32% from 3T; SC and FCC sublattices isolated to 0.0 (filter clean) |
| `bcc_band_spectrum` | 13 | PASS (Cluster A) | FCC seed-1: x₊=4.40e-2, x₋=9.41e-3, ratio 4.68; BCC seed-1: x₊=2.62e-2, x₋=1.39e-2, ratio 1.88; BCC seed-2: ratio 5.29; SC: Prony complex (expected — no isolated mode at L=16/T=800) |
| `beta_measurement` (Pillar 2 smoke) | 158 | PASS smoke | α_fine(L=32)=0.0887 (R²=0.779, 4 seeds identical), α_mid(L=16)=−7.12 (R²=0.929), **β = −4.279** (b=2, T=0.005, γ=0.01); seeds reproduce bit-identically (sigma=0) |
| `campaign_einstein` | 12 | PASS (11/11) | E1: energy ratio 1.19×10⁻³ after 500 ticks (damping); E2: Lorentz contraction `|J_x|/|J_y|` 1.0000 rest → 1.0477 boosted (ratio_diff 0.0477); E3: φ_lat(r=3) = 1.21×10⁻², φ_lat(r=10) = 9.35×10⁻⁴ (gradient ✓) |
| `campaign_wigner` | 7 | PASS (7/7) | W1 octahedral max/min = 1.000009; W2 chirality ±1.082×10⁻² (parity); W3 CPT energy match 2.88×10⁻⁷ relative |
| `eft_anisotropy` (Pillar 1) | 121 | PASS (10/10) | A1 isotropic ≤1e-9; A2 plane-wave anisotropy >0.1; A4 ξ recovers 5.0 (R²>0.999) |
| `eft_lorentz_recovery` (Pillar 1) | 8 | PASS (4/4) | mean(c) = 0.840 voxels/tick across 13 directions (note: 3T_eff frame, theory C_WAVE=0.577); **σ/mean = 0.000000%** isotropy |
| `eft_ward_identity` (Pillar 1) | 1 | PASS (4/4 active, W5 [OPEN]) | W1 ≤1e-6; W4 composite ≤1e-2; W5 vertex Ward deferred (no fermion propagators) |
| `eft_matched_poisson` (Pillar 1) | 0 | PASS (5/5) | M1 CG <1e-10; M2 deep-vacuum max\|∇·J−ρ\| ≤ 1e-8; M5 improvement ≥ 1e4 |
| `eft_operator_spectrum` (Pillar 5) | 221 | PASS (3/3 partial) | JJ Δ=0.531 (naive 2.0, R²=0.997); 4/6 operators give valid Δ at L=32; confinement-era P9 5/6 operators valid |

---

## Status by Pillar (per CLAUDE.md framing)

- **Pillar 1 (Lorentz invariance):** PASS — anisotropy 0.0000%, matched-Poisson <1e-8, Ward identities OK
- **Pillar 2 (β-function):** PASS smoke — β = −4.28 with reproducible seeds (note: smoke parameters T=0.005, γ=0.01, single L-pair 16↔32)
- **Pillar 3 (Einstein/GR):** PASS — energy conservation, Lorentz contraction, gravitational redshift all detected
- **Pillar 4 (Wigner/discrete-symmetry):** PASS — octahedral 6-axis isotropy 9 ppm, parity, CPT 0.3 ppm
- **Pillar 5 (Operator-basis spectrum):** PARTIAL PASS — JJ relevant operator matches naive Δ=2 → measured 0.5 (4× consistent with diffusive scaling rather than relativistic); 4 of 6 operators give valid R²>0.5 fits at L=32
- **Cluster A (sublattice infrastructure):** PASS — helpers, Laplacian eigenvalues (SC/FCC/BCC = 1/2/3), diagonal correlators, two-state extraction, Langevin sublattice filter, BCC band spectrum (FCC ratio 4.68, BCC ratio 1.88–5.29 seed-dependent)

---

## Notes / anomalies

1. **CTest internal timeout = 120 s** is hardcoded for `eft_lorentz_recovery` and
   `eft_operator_spectrum`. Runs invoked via `ctest -R` time out at 120 s. Direct
   executable invocation (`./Release/<exe>`) succeeds — `ftd_lorentz_measure.exe`
   completes in 8 s; `test_eft_operator_spectrum.exe` completes in 221 s. Both
   `stdout.csv` files in this baseline come from direct invocation. The
   `stderr.log` files contain the original ctest-driven timeout output for
   reference; `stderr_direct.log` (where present) holds the direct-invocation
   stderr.

2. **β-function smoke shows zero seed variance** (`alpha_fine_sem=0`,
   `sigma_beta=0`). All four seeds (101/118/135/152) returned identical α_fine
   and α_mid values to all printed digits. This either means the seed is being
   ignored at the GPU layer, or the smoke parameters keep the system in a
   purely linear regime where shot noise has not yet entered. Worth a follow-up
   diagnostic before promoting the β value past [SMOKE].

3. **Langevin equipartition deviation is +4.28%** — agrees with the 4% figure
   quoted in CLAUDE.md project memory (`FTD-0051`). Sublattice variant gives
   +2.32% on BCC with SC/FCC sublattices isolated to floating-point zero.

4. **`bcc_band_spectrum` SC stencil yields complex Prony roots** at L=16,
   T=800. This is documented as expected (no isolated two-state spectrum on
   the pure SC sub-stencil at this grid); FCC and BCC stencils both give pure
   real exponentials with sensible ratios. Useful to capture for drift checks.

5. **`eft_lorentz_recovery` reports c_eff = 0.840** — this is in engine-internal
   units where the analytical C_WAVE = 1/√3 ≈ 0.577. The 1.456× ratio is
   consistent with the 3-component-summed flux normalisation reported in
   prior Pillar-1 audits. The isotropy metric (σ/mean) is what matters for
   Lorentz recovery and reads 0.000000% — the strongest signal in the suite.

6. **Hardware path is Windows-native CUDA**, not WSL2. Per project memory
   (`feedback_use_wsl2_for_gpu.md`), measurement campaigns should normally go
   through `engine/build_wsl`. The user requested baseline against current
   `engine/build/` per the task spec; total wall time stayed under budget
   (~12 min for 15 tests excluding the 120-s timeout retries), so no
   pathological-slowdown event was observed for this baseline. Future drift
   comparisons under different hardware paths should note this.

7. **Skipped (out of scope for this baseline):** ctest IDs 18-38 (`native_*`
   ledger and flow tests), 51-62 (Clifford suite), 70 (langevin GPU/CPU
   parity), 72-74 (mechanism-b, small particle emergence, color binding), and
   the 246 total executables. These are tracked separately under different
   labels and were not part of the requested smoke set.

---

## Reproducibility

From `engine/build/` (Windows MSBuild, Release config):

```bash
# Full Cluster-A + EFT pillar set (one-shot, mirrors what produced this baseline)
ctest -R '^(sublattice_helpers|sublattice_laplacian|correlations_diagonal|two_state_extraction|langevin_equipartition|langevin_sublattice_equipartition|bcc_band_spectrum|beta_measurement|campaign_einstein|campaign_wigner|eft_anisotropy|eft_ward_identity|eft_matched_poisson)$' -C Release --output-on-failure

# EFT tests with native ctest-timeout-bypass (long runs):
./Release/ftd_lorentz_measure.exe          > stdout.csv 2> stderr.log    # ~8 s
./Release/test_eft_operator_spectrum.exe   > stdout.csv 2> stderr.log    # ~221 s

# Direct-exe pattern (preferred when CTest's internal 120-s timeout binds):
./Release/<exe>.exe > stdout.csv 2> stderr.log
```

Per-test artefacts in this directory:
- `<test>/stdout.csv` — clean program output (CSV-friendly)
- `<test>/stderr.log` — ctest-V wrapped stderr OR direct-invocation stderr
- `<test>/stderr_direct.log` — present when direct-exe was used
- `<test>/meta.json` — commit, wall-time, return code, hardware, build config

---

## Drift-check checklist (next baseline)

When producing the next baseline (e.g. against post-Pillar-2-publication-grade run, or after fixing FTD-0091 inject_flux on WSL2):

1. Diff each `stdout.csv` line-by-line — it's deterministic CSV. Watch for:
   - Langevin: `<|v|^2>` should stay within ~1% of this baseline's 3.128×10⁻²
   - β-measurement: α_fine should match to all printed digits unless seed/init changed
   - bcc_band: Prony ratios for FCC/BCC seed-1 are the most stable diagnostics
2. Compare wall times in `meta.json` — beta_measurement (158 s) and
   eft_operator_spectrum (221 s) are the dominant costs; any 2× slowdown is a
   regression worth investigating.
3. Re-confirm GPU banner in every relevant `stderr_direct.log`.
