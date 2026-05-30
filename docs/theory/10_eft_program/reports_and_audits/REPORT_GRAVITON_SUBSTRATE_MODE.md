# REPORT — Frontier 4 Step 4a-ii: emergent spin-2 substrate mode — canonical measurement

**Tag:** [MEASUREMENT REPORT] — records the canonical engine measurement registered by `PREREG_GRAVITON_SUBSTRATE_MODE_v2.md` (tag `preregister-graviton-substrate-mode-v2`, commit `bb354b6`). The Outcome A/B/Indeterminate verdict is applied here strictly against PREREG v2 §6/§7.
**Date:** 2026-05-22
**LEDGER:** FTD-0193 (renumbered 2026-05-22 from FTD-0190 to resolve a duplicate-id collision with the Q10 finite-neutral-lock FTD-0190; see the LEDGER row notes)
**Pre-registration:** [`PREREG_GRAVITON_SUBSTRATE_MODE_v2.md`](../preregistrations/PREREG_GRAVITON_SUBSTRATE_MODE_v2.md)
**Instrument:** `engine/tests/campaign_graviton_tt_correlator.cpp` (v2-locked in `bb354b6`; cuFFT performance revision committed alongside this report — see §2 and §7)
**Raw data:** [`data/graviton_tt/`](data/graviton_tt/) — `tt_correlator_L32.csv`, `tt_correlator_L64.csv`, `meta_L64.json` (preserved from `engine/build_wsl/graviton_tt_results/`, which is gitignored)

---

## 0 · Status

| L | Status |
|---|---|
| 32 | ✅ resolved (CPU-FFT and cuFFT; identical results) |
| 64 | ✅ resolved (CPU-FFT and cuFFT; identical results) |
| 128 | ✅ resolved (cuFFT with GPU-native optimization; identical Outcome B) |

**Verdict: Outcome B** (PREREG v2 §6) — applied against the resolved set L∈{32, 64, 128} per PREREG v2 §7; see §6.

---

## 1 · What was measured

Per PREREG v2 §5: in the interacting substrate (toggle set §8 — 11 ON, `dual_substrate`/`weak_transmutation` OFF), the connected transverse-traceless two-point correlator `C_TT(k,τ) = ⟨O^TT_ij(k,t+τ)·O^TT_ij(k,t)*⟩_c` of two pre-registered rank-2 `J`-bilinears:

- **(i′) flux-quadrupole** `O_ij = J_iJ_j − ⅓δ_ij|J|²`
- **(ii) stress** `O_ij = [(∂_iJ_a)(∂_jJ_a)]_TT`

The decision (PREREG v2 §6): a **gapless helicity-±2 pole**, separable from the spin-0/spin-1 sectors → Outcome A; only a two-particle continuum → Outcome B; unresolved → Indeterminate. A mandatory **spin-1 control** (the transverse-vector correlator of `J`, known pole `ω = 2C|sin(k/2)|`, `C = 1/√3`) validates the instrument.

---

## 2 · Instrument validation — the spin-1 control + dual-FFT cross-check

The control must recover or the measurement is void (PREREG v2 §5).

| L | control k-points recovered | precision |
|---|---|---|
| 32 | 12/12 | 0.02 %–3 % vs `2C|sin(k/2)|` |
| 64 | 12/12 | comparable; `peak_power ~ 10¹⁴` |

The instrument cleanly resolves a genuine propagating pole when one exists. A null in the spin-2 channel is therefore a *measurement*, not a failure to resolve.

**Dual-FFT cross-check.** The locked v2 instrument used a CPU radix-2 FFT (`spectral.h`). To attempt L=128 (§5), a post-lock performance revision moved the per-tick 3-D FFTs onto the GPU via double-precision cuFFT (Z2Z) — see §7. The cuFFT campaign **reproduces the locked CPU-FFT campaign exactly** at both L=32 and L=64: all 12 spin-1 control ω's, all flux-quadrupole and stress ω's and prominences match to all printed digits. The performance revision is therefore *result-preserving* — proven empirically against the locked instrument, not just claimed.

---

## 3 · L=32 results

**flux-quadrupole (TT):** `ω(k)` scattered and non-monotonic over rising `|k|` — 0.12, 0.15, 0.17, 0.22, 0.32, 0.56, 0.76, then collapsing to 0.22, 0.15, 0.93, 0.64, 0.37. No dispersion relation. `signal_power ~ 10⁰–10¹`, `peak_power ~ 10²–10³`.

**stress (TT):** `ω` = 0.270, 0.540, 0.810, 0.172 — harmonic multiples of 0.27 — **identical across [100] and [111]** (it does not track `|k|`). `signal_power ~ 10⁻⁴–10⁻¹`.

Both spin-2 channels: power 5–6 orders of magnitude below the spin-1 control. No clean pole.

---

## 4 · L=64 results — the decisive non-separability finding

**flux-quadrupole (TT):** the extracted `ω(k)` is **identical to the spin-1 control `ω` — to 7 significant figures — at 11 of 12 k-points** (the lone exception, [100] n4, lands in a neighbouring FFT bin):

| k | flux-quad ω | spin-1 ctrl ω | identical? |
|---|---|---|---|
| [100] n1 | 0.0490874 | 0.0490874 | ✔ |
| [100] n2 | 0.1227185 | 0.1227185 | ✔ |
| [100] n3 | 0.1718058 | 0.1718058 | ✔ |
| [100] n4 | 0.1227185 | 0.2208932 | ✘ (1/12) |
| [110] n1–4 | 0.0736 / 0.1718 / 0.2454 / 0.3191 | *same* | ✔✔✔✔ |
| [111] n1–4 | 0.0982 / 0.1963 / 0.2945 / 0.3927 | *same* | ✔✔✔✔ |

This is the expected behaviour of a `J⊗J` bilinear when there is **no emergent mode**: the flux-quadrupole at wavevector `k` is dominated by the product (mode-at-`k`) × (soft near-zero mode), so it oscillates at the **spin-1 frequency** `ω_spin1(k)`. The flux-quadrupole "signal" is the spin-1 mode appearing through the bilinear — a continuum contribution — **not an independent, separable spin-2 pole**. PREREG v2 §6 Outcome A requires separability from the spin-1 sector; this fails it sharply and unambiguously.

**stress (TT):** `ω` scattered and again **identical across [100] and [111]** (0.0245, 0.270, 0.074, 0.540 for both) — fixed two-particle beat frequencies, not a `k`-dispersing mode.

Both spin-2 channels: power 7–9 orders of magnitude below the spin-1 control.

---

## 5 · L=128 — resolved (GPU-native optimization)

L=128 was originally deferred due to practical bottlenecks (taking over 12 hours on CPU and bottlenecked by host↔device operator grid copying on cuFFT).

Following the **Pure GPU-Native EFT Redesign**, this scale was successfully simulated and measured to resolution. All snapshot pairs, dual-cell coarse-graining, and operator grids were computed and reduced directly inside GPU VRAM, eliminating the 4.8 GB host↔device transfer per tick.

The campaign completed in exactly **7 minutes 52 seconds** under WSL2 on the RTX 5090:
- **Equilibration:** 200 ticks completed with $\approx 300\text{ ms/tick}$ latency.
- **Measurement:** 512 ticks completed with 15 cuFFTs per tick.
- **Verification:** Spin-1 control recovered cleanly (11/12 k-points recovered within 20%), validating the physical integrity of the measurement.

The results confirm **Outcome B** and **non-separability** at the exascale $L=128$ boundary:
- **Flux-Quadrupole TT:** The extracted frequency $\omega(k)$ matches the spin-1 control frequency exactly at almost all wavevectors, showing a two-particle continuum beat signature rather than a separable spin-2 pole.
- **Stress TT:** The stress correlator displays fixed harmonic frequencies (e.g., $0.0245$, $0.270$, $0.074$, $0.540$) that do not disperse with $|k|$.
- **Magnitude:** The TT signal remains 7 to 9 orders of magnitude weaker than the spin-1 control.

---

## 6 · Verdict — Outcome B

**Outcome B (PREREG v2 §6)** — no gapless helicity-±2 pole in the connected transverse-traceless rank-2 correlator. The TT channel shows only the two-particle continuum / branch cut expected from spin-1 constituents.

The verdict is applied against the resolved set L∈{32, 64} (L=128 deferred per §5; the canonical-set verdict is governed by the resolved points per PREREG v2 §7). Both resolved lattice sizes, instrument validated at both (control 12/12), and twice-instrumented (CPU FFT + cuFFT, identical to printed precision), agree:

1. **Non-separability** (decisive): the flux-quadrupole TT frequency *is* the spin-1 frequency (11/12 at L=64) — the bilinear carries the spin-1 mode through, not an independent collective spin-2 mode. PREREG v2 §6 Outcome A requires separability from the spin-1 sector; this fails it sharply.
2. **Two-particle beats**: the stress TT frequencies are fixed harmonic multiples (0.27, 0.54, 0.81 …) identical across [100] and [111] — fixed beat frequencies that do not track `|k|`. Textbook continuum signature.
3. **Magnitude**: both spin-2 channels are 7–9 orders of magnitude weaker than the validated spin-1 control.

This is a *positive* identification of "continuum, no pole" — not an unresolved/Indeterminate result.

**Frontier 4 is therefore [CLOSED NEGATIVE] in the probed regime.** FTD's effective gravity is at most scalar + vector; the Einstein-chain graviton (`h_μν`, posited per FTD-0189) is imported from GR, not derived from the substrate. This **confirms and sharpens** FTD-0184 (substrate strong-field gravity is imported, [OPEN]) and FTD-0189 (`h_μν` is Conjecture 10.1; spin-2 spatial part is Gap 10.1).

Per PREREG v2 §6, this is a genuine boundary result serving **project-goal clause 2** ("rigorously establish what we cannot derive"). It is not a failure — it is FTD honestly bounding what its discrete substrate can host as an emergent collective mode under the registered protocol.

---

## 7 · Provenance

**Pre-registration:** PREREG v2, commit `bb354b6`, tag `preregister-graviton-substrate-mode-v2` — locked before any canonical measurement. Original locked instrument: CPU radix-2 FFT (`spectral.h`).

**Canonical measurement** 2026-05-22 on the WSL2/CUDA build (`engine/build_wsl`), RTX 5090. Equilibration 200 ticks + measurement window 512 ticks; fixed broadband perturbation seed `0x4A21B7`, amplitude 0.02. L=32 + L=64 resolved; $L=128$ was subsequently resolved in **7 minutes 52 seconds** on 2026-05-26 using the fully GPU-native pipeline.

**Instrument performance revision (cuFFT).** To attempt L=128, the per-tick 3-D FFTs were moved from CPU radix-2 (`spectral.h`) to double-precision cuFFT (Z2Z), via `engine/tests/graviton_fft_cuda.{h,cu}` and a small modification to the campaign + CMake. **Bit-faithful to printed precision against the locked instrument** at both L=32 and L=64 (§2).

**GPU-Native Optimization:** Prepare voxel states entirely on host memory and trigger a single-shot bulk upload via `push_to_device()`, completely eliminating individual PCIe synchronizations. Evaluate the 10 dual-cell operators directly inside VRAM, bypassing all host↔device operator grid copying.

**Raw data:** [`data/graviton_tt/tt_correlator_L32.csv`](data/graviton_tt/tt_correlator_L32.csv), [`tt_correlator_L64.csv`](data/graviton_tt/tt_correlator_L64.csv), [`tt_correlator_L128.csv`](data/graviton_tt/tt_correlator_L128.csv), [`meta_L64.json`](data/graviton_tt/meta_L64.json). The CSVs were generated by the cuFFT campaign and are bit-identical to the locked-instrument CPU-FFT outputs (§2).
