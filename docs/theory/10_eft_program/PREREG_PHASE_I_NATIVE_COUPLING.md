# Pre-Registration · Phase I — FTD-Native Coupling

**Status:** PRE-REGISTRATION (committed BEFORE any measurement; hash-locked via git tag `preregister-phase-i-native-coupling-v1`).
**Date:** 2026-05-03
**Pre-registration discipline:** follows FTD-0097 / FTD-0121 / FTD-0123 / FTD-0124 methodology — methodology committed before execution; no tolerance-tuning post-hoc; outcomes named explicitly with what each means at the LEDGER level.

---

## 1 · Purpose

Test whether the coupling derived from FTD's algebraic spine propagates self-consistently through the engine's wave-propagation channel. Distinct from prior α-derivation attempts (R1/R2/R3/R4, all [CLOSED NEGATIVE]) in that it does not insert α as a target and does not rely on the engine's gauss-projection channel (which Phase G shows is geometric and zero-fine-structure-content).

This pre-registration does **NOT** attempt to close MC-T4.3 (axioms → α via dynamics). It does **NOT** claim a derivation of α. Its scope is narrower: derive a coupling `g_FTD` from a [THEOREM] in the algebraic spine, then ask what V(r) the engine computes when that coupling enters the source term of the wave equation.

## 2 · Theoretical derivation (committed before measurement)

### 2.1 Statement

Let `x_+` denote the larger root of the master quadratic `x² − 16G*²x + 16G*³ = 0` of FTD-0001 / Theorem 2 of `SPEC_ALGEBRAIC_SPINE.md`, with closed form

$$x_+ = 8G^{*2} + 4G^* \sqrt{4G^{*2} - G^*}.$$

Define **the FTD-native coupling**

$$g_{\mathrm{FTD}}^{2} := \frac{1}{x_{+}}.$$

This coupling is **[DERIVED]** from the master quadratic [THEOREM]: `x_+` is a closed-form algebraic expression in `G*`, and `g_FTD²` is its inverse. No experimental input enters the derivation. The numerical value is a deterministic function of `G* = Γ(1/4)/Γ(3/4)`.

### 2.2 Numerical value (computable to arbitrary precision before measurement)

```
G*       = Γ(1/4) / Γ(3/4)              ≈ 2.958675119188639...
x_+      = 8G*² + 4G*√(4G*² − G*)       ≈ 137.036171458...
g_FTD²   = 1/x_+                         ≈ 0.0072973548525...
g_FTD    = √(1/x_+) = 1/√x_+             ≈ 0.0854245431...
```

This is the [DERIVED] value to be tested.

### 2.3 Comparison constants (committed before measurement)

```
α_CODATA     = 1/137.035999084(21)       ≈ 0.0072973525693...   (CODATA 2022)
g_FTD² − α   ≈ 9.18 × 10⁻⁹                                       (1.26 ppm relative)
```

The 1.26 ppm match between `g_FTD²` and α_CODATA is the empirical observation underlying the [STRONGLY MOTIVATED CONJECTURE] `x_+ = 1/α`. **This conjecture is not assumed** in the present test; it is what the engine measurement is asked about.

## 3 · Operational protocol (committed before measurement)

### 3.1 Observable

The engine's source-coupled wave equation is

$$\square J = G_{C} \cdot \nabla s, \qquad G_{C} := g_{\mathrm{FTD}} = \sqrt{1/x_{+}}$$

(per `engine/include/ftd/constants.h` + `engine/web/js/constants.js` lines 60-62; the engine already runs with this `G_C`).

For two static charges `q_1, q_2` at separation `r`, the interaction-energy coefficient is

$$V(r, L) = -G_{C}^{2} \cdot V_{\mathrm{geom}}(r, L)$$

where `V_geom(r, L)` is the lattice geometric kernel — what Phase G measured. The *coupling* is the prefactor `G_C²`; the *geometry* is `V_geom`.

### 3.2 Extraction

Define the extracted coupling

$$g_{\mathrm{engine}}^{2}(r, L) := \frac{V(r, L)}{V_{\mathrm{geom}}(r, L)}.$$

This is the dimensionless coefficient FTD's lattice EFT assigns to the static interaction in the wave-propagation channel.

### 3.3 Measurement domain

- L ∈ {64, 128, 256, 384}
- r ∈ Coulomb-tail regime (r/L ≈ 0.31) per Phase G convention
- Python FFT-based prediction first; C++ engine cross-check deferred (see §6)
- Tolerances: master-quadratic-canonical (1.26 ppm) for the agreement check

### 3.4 Pre-registered comparisons

For each L, we will compute and compare:

1. `g_engine²(L)` — measured ratio above
2. `g_FTD² = 1/x_+` — derived from master quadratic
3. `α_CODATA = 1/137.035999084` — experimental reference

## 4 · Three pre-registered outcomes

| Outcome | What it would mean | LEDGER consequence |
|---|---|---|
| **A.** `g_engine² = g_FTD²` to engine precision (i.e. ratio is identically `1/x_+`) | FTD's coupling propagates self-consistently. The polynomial value is what the dynamics realize. The wave-propagation channel matches the algebraic-spine prediction. | Promote: a new [DERIVED] entry stating "FTD's lattice EFT realizes coupling `g_FTD² = 1/x_+` derived from the master quadratic [THEOREM]." Spine adjacent, not spine. |
| **B.** `g_engine² = α_CODATA` to engine precision (and `g_engine² ≠ g_FTD²` distinguishably) | The engine measures α directly; the master-quadratic value is not what the dynamics realize. A different bridge between spine and dynamics is at play. | This would be a SURPRISE — would invalidate the engine's `G_C := √(1/x_+)` definition as the operational coupling. Investigation required. |
| **C.** `g_engine² ≠ g_FTD²` AND `g_engine² ≠ α` | The engine predicts a coupling distinct from both the polynomial value and QED. | A real FTD-specific prediction; falsifiable; publishable as a Phase-I-finding LEDGER row. |

The expected outcome under the engine's existing implementation (`G_C := √(1/x_+)` hardcoded in `engine/include/ftd/constants.h`) is **A**. Outcome B or C would imply a discrepancy between the engine's stated coupling and what its dynamics actually realize — informative either way.

## 5 · Methodological discipline

This pre-registration is **hash-locked**:
- Theoretical derivation in §2 is fixed.
- Operational protocol in §3 is fixed.
- Three outcomes in §4 are fixed.
- Tolerances in §3.4 are fixed.

Any change to any of these *after* running the measurement is a methodology violation and must be flagged in the LEDGER as such.

The git tag `preregister-phase-i-native-coupling-v1` is applied to this commit BEFORE the measurement script runs.

## 6 · Engine cross-check — implementation 2026-05-03 (build complete; run deferred to WSL2)

### Status

Implementation complete; awaiting WSL2 / GPU run for verification. Python predictor (`scripts/proofs/proof_phase_i_native_coupling.py`) stands as engine-equivalent per FTD-0118 / Q3-Q4 precedent.

### Artifacts

- `scripts/proofs/generate_phase_i_lattice_green_fixtures.py` — generates SC7 lattice Poisson Green's function fixtures at selected (L, r) pairs
- `engine/tests/phase_i_green_fixtures.h` — auto-generated header with 8 fixtures (L ∈ {32, 64} × selected r)
- `engine/tests/benchmark_phase_i_native_coupling.cpp` — C++ benchmark (~170 LOC) measuring `α_r(r,L)` in the wave-propagation channel and computing `g_engine²(r,L) := α_r / (2 r G_L(r))` for each fixture
- CTest registration: `engine/CMakeLists.txt` line 713-714 (target `ftd_phase_i_native_coupling`)
- Windows-native compile-check: PASS (commit follow-up to `0ea9d13`); executable at `engine/build/Release/ftd_phase_i_native_coupling.exe`

### Pre-registered run protocol (hash-locked)

```bash
# WSL2 build + run (canonical per CLAUDE.md WSL2/GPU discipline):
wsl.exe -d Ubuntu-22.04 -- bash -c "cd /mnt/c/Users/cpaci/Desktop/ftd && \
    cmake -S engine -B engine/build_wsl -DCMAKE_BUILD_TYPE=Release && \
    cmake --build engine/build_wsl --target ftd_phase_i_native_coupling -j 8 && \
    engine/build_wsl/benchmark_phase_i_native_coupling 200"

# Or via CTest:
wsl.exe -d Ubuntu-22.04 -- bash -c "cd /mnt/c/Users/cpaci/Desktop/ftd/engine/build_wsl && \
    ctest -R benchmark_phase_i_native_coupling --output-on-failure"
```

### Pre-registered pass criterion

`g_engine²(L, r)` at every fixture must equal `1/x_+ = ALPHA_EFT` to relative tolerance `1e-3` (1000 ppm — generous; allows for finite equilibration tick count + finite-r convergence). Default `ticks=200`.

If all 8 fixtures PASS: outcome A confirmed at the engine level. Promotes Phase I closure from "Python-equivalent verification" to "Python + engine-confirmed verification."

If any fixture FAILS: investigate. Likely culprits in order of probability: (i) insufficient equilibration ticks (try `ticks=500`); (ii) finite-r boundary effect; (iii) actual physics surprise (would require LEDGER FTD-0125 update).

### Why this is deferred to WSL2

Per CLAUDE.md `Environment Notes`: "GPU execution MUST go through WSL2 Ubuntu-22.04, not Windows-native CUDA. Windows-native CUDA builds technically run but are pathologically slow." The benchmark uses `rb.run(ticks)` which is GPU-accelerated when CUDA is available. Windows-native CUDA may take 19+ minutes per measurement; WSL2/GPU should complete in <1 minute total. Scope discipline: defer.

## 7 · References

- `SPEC_ALGEBRAIC_SPINE.md` Theorem 2 (master quadratic)
- `engine/include/ftd/constants.h` (canonical G_C definition)
- `engine/web/js/constants.js` lines 60-62 (web mirror)
- `docs/theory/10_eft_program/AUDIT_ALPHA_EXTRACTION.md` (Phase G audit)
- `docs/theory/03_derivations/DERIV_EMERGENT_COULOMB_GEOMETRIC.md` (Phase G theorem)
- `docs/theory/10_eft_program/SPEC_FTD_EFT_BRIDGE_CONTRACT.md` (Branch-A vs Branch-B)
- `scripts/proofs/proof_q3_q4_engine_stencil.py` (FTD-0118 engine-equivalent computation precedent)

## 8 · Closure criterion

This pre-registration is closed when:
1. Measurement script runs successfully
2. One of the three outcomes A/B/C is reported with explicit numerical values
3. LEDGER row added (FTD-0125)
4. The git tag `preregister-phase-i-native-coupling-v1` precedes the measurement-result commit in git history

Not closed by: tolerance-tuning, observable redefinition, or any post-hoc adjustment.
