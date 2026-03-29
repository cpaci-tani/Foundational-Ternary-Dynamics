# Detector Information Loss Test Suite — Design Spec

**Date:** 2026-03-27
**Status:** Approved
**Location:** `scripts/experiments/detector_information_loss/`

---

## Purpose

Demonstrate quantitatively how much valuable data a simple boolean detector slit screen destroys in a double-slit experiment. The argument is framed in favor of **complexity** — the extraordinary richness of structure that actually exists in the full wave field — rather than the reductive "collapse" narrative.

Each test computes something genuinely rich about the full ψ field, then shows what survives after |·|² and/or after individual photon clicks, quantifying the loss in bits, mutual information, or other hard metrics.

## Architecture

### Directory Structure

```
scripts/experiments/detector_information_loss/
├── field_engine.py                      # Shared module (~150 lines)
├── output/                              # Generated figures + JSON summaries
├── 01_phase_entropy.py
├── 02_phase_gradient_field.py
├── 03_phase_singularities.py
├── 04_nodal_line_topology.py
├── 05_spatial_coherence.py
├── 06_cross_slit_correlations.py
├── 07_long_range_phase_locking.py
├── 08_spectral_information.py
├── 09_which_frequency_where.py
├── 10_bits_per_pixel.py
├── 11_fisher_information.py
├── 12_reconstruction_impossibility.py
├── 13_ternary_vs_boolean.py
├── 14_void_is_destructive_interference.py
├── 15_parameter_sensitivity.py
└── 16_cumulative_report.py
```

### Shared Module: `field_engine.py`

Provides the physics computation and degradation pipeline used by all 16 scripts.

**Field computation:**
- `compute_dual_source_field(W, H, lam, separation, phase_offset, t)` → `(psi_re, psi_im)` as `(W×H)` float32 arrays
- Uses Huygens-Fresnel diffraction: `ψ(r) = Σ_s A_s / √r · cos(kr - ωt + φ_s) + i·sin(kr - ωt + φ_s)`
- `c = 1/√3` from `scripts/constants.py`
- Counter-phase sources: φ_A = 0, φ_B = π (matching the HTML simulation)
- Causal wavefront: smoothstep envelope at `r = c·t`

**Degradation pipeline:**
- `born_rule(re, im)` → `re² + im²`
- `sample_detector_clicks(born_field, N_clicks)` → `(N_clicks, 2)` array via rejection sampling
- `phase_field(re, im)` → `arctan2(im, re)`
- `amplitude_field(re, im)` → `√(re² + im²)`

**Information measures:**
- `shannon_entropy(data, bins=256)` → float (bits)
- `mutual_information(field_a, field_b, bins=64)` → float (bits)
- `fisher_information(field, param_derivative, dx)` → float
- `kl_divergence(p, q)` → float (bits)

**Visualization:**
- `make_figure(title, panels, metrics_text, filename)` — consistent 3–4 panel layout
- Dark background (#0a0a0f), FTD color palette (phase→hue, amplitude→luminance)
- Every figure has the degradation spine: full ψ | |ψ|² | detector dots
- Plus one or more analysis-specific panels
- Saves PNG to `output/` and JSON summary (key metrics as machine-readable dict) to `output/`

**Default parameters:**
- Grid: 512×512 (fast enough for all scripts, high enough resolution for topology)
- λ = 32, separation = 80, phase_offset = π, t = 400 (well-developed wavefront)
- K_B = 0.05, N_clicks = 10000 (default detector sample)

### Script Template

Every script follows this structure:

```python
#!/usr/bin/env python3
"""NN_title — one-line description

WHY THIS MATTERS:
2-3 sentences on what this test reveals about the detector's
blindness, framed as complexity the screen destroys.

EPISTEMIC STATUS: [EXPLORATION]
"""

import sys, os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))
from experiments.detector_information_loss.field_engine import (
    compute_dual_source_field, born_rule, sample_detector_clicks,
    phase_field, amplitude_field, shannon_entropy, mutual_information,
    make_figure
)

def main():
    # 1. Compute full field
    psi_re, psi_im = compute_dual_source_field(...)

    # 2. Compute the structure this test targets (on full ψ)
    # ... analysis specific to this test ...

    # 3. Compute the same on |ψ|² and/or detector clicks
    born = born_rule(psi_re, psi_im)
    clicks = sample_detector_clicks(born, N_clicks)

    # 4. Quantify the loss
    # ... bits, mutual information, Fisher information ...

    # 5. Print summary
    print(f"...")

    # 6. Generate figure
    make_figure(...)

    # 7. Save JSON summary
    # { "test": "NN_title", "full_field_metric": X, "detector_metric": Y, "loss_pct": Z }

if __name__ == '__main__':
    main()
```

---

## Test Suite: 16 Scripts in 6 Groups

### Group A: Phase Structure (scripts 01–04)

The phase θ(x,y) = arctan2(Im(ψ), Re(ψ)) is the most direct casualty of |·|².

**01_phase_entropy.py** — *Shannon entropy of the phase field*

WHY THIS MATTERS: The full ψ has a phase θ(x,y) at every pixel — a rich, structured distribution encoding the entire interference geometry. |ψ|² has zero phase information. The gap between H(θ) and H(nothing) is the irreversible cost of the absolute value bars.

- Compute phase field θ(x,y) = arctan2(Im ψ, Re ψ)
- Bin into 256 phase bins, compute Shannon entropy H(θ)
- For |ψ|²: phase is undefined → H = 0
- For detector clicks: phase is unrecoverable → H = 0
- Metric: bits of phase information destroyed per pixel
- Figure: (a) phase field with hue colormap, (b) |ψ|² (gray), (c) detector dots, (d) phase histogram with entropy annotation

**02_phase_gradient_field.py** — *Phase gradients encode momentum flow*

WHY THIS MATTERS: ∇θ is the local wavevector — it tells you which direction energy is flowing at every point. This is a full vector field with curl, divergence, and singularities. The detector screen sees none of it.

- Compute ∇θ via finite differences (with 2π-wrapping handled)
- Compute |∇θ| (gradient magnitude), ∇×(∇θ) (curl — nonzero at vortices)
- Total vector field information: H(∇θ_x) + H(∇θ_y)
- Detector equivalent: no gradient field exists → 0 bits
- Figure: (a) streamline plot of ∇θ, (b) gradient magnitude heatmap, (c) |ψ|² with no flow information, (d) quiver plot overlay showing flow directions

**03_phase_singularities.py** — *Topological defects the detector cannot see*

WHY THIS MATTERS: Phase vortices — points where ψ = 0 with winding number ±1 — carry quantized angular momentum and are topologically protected. The detector sees them as "dark spots" indistinguishable from ordinary low-intensity regions.

- Detect vortices: compute winding number around each 2×2 plaquette via Δθ accumulation
- Count N_+, N_- (positive/negative vortices), verify N_+ - N_- = total charge
- Map positions, overlay on amplitude field
- In |ψ|²: these are just zeros — no winding information survives
- Metric: number of topological defects present vs number recoverable from |ψ|² (zero)
- Figure: (a) phase field with vortex markers (⊕/⊖), (b) amplitude showing the "dark spots", (c) |ψ|² — identical dark spots, no topology, (d) table of vortex census

**04_nodal_line_topology.py** — *The skeleton of interference is invisible*

WHY THIS MATTERS: The zero-contours of Re(ψ) and Im(ψ) form intricate networks whose connectivity and topology encode the source geometry. |ψ|² converts these to "dark fringes" but destroys all topological invariants.

- Extract zero-contours of Re(ψ) and Im(ψ) via marching squares
- Compute graph properties: number of connected components, total arc length, intersection count (= vortex count)
- For |ψ|²: extract dark fringe contours (level set at threshold)
- Compare: dark fringes ≈ |Re(ψ)=0 ∩ geometry| but lose connectivity and Im(ψ)=0 entirely
- Metric: Euler characteristic of nodal network (full field) vs fringe count (detector)
- Figure: (a) Re(ψ)=0 contours (red) + Im(ψ)=0 contours (blue), intersections marked, (b) |ψ|²=threshold contours only, (c) overlay showing what's lost, (d) graph statistics table

### Group B: Correlation Structure (scripts 05–07)

What the field knows about distant points that the detector doesn't.

**05_spatial_coherence.py** — *The mutual coherence function*

WHY THIS MATTERS: Γ(r₁, r₂) = ψ*(r₁)·ψ(r₂) encodes how the field at one point predicts the field at another, including phase relationships across arbitrary distances. The detector's intensity autocorrelation loses all phase-sensitive correlations.

- Compute Γ(Δx, Δy) averaged over the field (Van Cittert-Zernike style)
- Compute g⁽²⁾(Δx, Δy) = ⟨I(r)·I(r+Δr)⟩/⟨I⟩² for the |ψ|² field
- Compute detector g⁽²⁾ from click pair correlations (finite-N statistics)
- Metric: mutual information I(r₁; r₂) at various separations for full field vs detector
- Figure: (a) |Γ(Δx, 0)| coherence profile (oscillating, rich), (b) g⁽²⁾ profile (envelope only), (c) detector pair correlation (noisy envelope), (d) MI vs separation distance plot

**06_cross_slit_correlations.py** — *Correlations between the two slit contributions*

WHY THIS MATTERS: At every point, ψ = ψ_A + ψ_B. The full field lets you decompose this — you can compute the cross-term 2Re(ψ_A*·ψ_B) that IS the interference. The detector gives you |ψ_A + ψ_B|² with no way to separate the three terms.

- Compute ψ_A and ψ_B separately (single-source fields)
- Cross-term: 2Re(ψ_A* · ψ_B) — the interference contribution
- Self-terms: |ψ_A|² + |ψ_B|² — the non-interfering background
- In the detected |ψ|²: all three terms are mixed. The cross-term fraction is computed but irrecoverable from clicks alone
- Metric: fraction of detected intensity due to interference vs background; information needed to separate them (infinite from clicks alone)
- Figure: (a) cross-term heatmap (red/blue for constructive/destructive), (b) self-term sum (smooth envelope), (c) |ψ|² = sum of all three (inseparable), (d) pie chart of energy budget

**07_long_range_phase_locking.py** — *Phase coherence across the entire field*

WHY THIS MATTERS: In the full ψ, points separated by many wavelengths maintain deterministic phase relationships. After |·|², this determinism vanishes: intensity at distant points correlates only through the envelope.

- For pairs of points at separation d, compute phase locking factor ρ(d) = |⟨e^{i(θ₁-θ₂)}⟩|
- Average over all pairs at each separation
- Full field: ρ(d) ≈ 1 everywhere (perfect phase locking from coherent sources)
- |ψ|² field: no phase exists → ρ undefined → set to 0
- Detector dots: no phase information → ρ = 0
- Metric: phase locking factor as function of distance
- Figure: (a) Δθ map between two reference lines, (b) ρ(d) curve (flat at ~1 for full field), (c) "ρ(d) = 0" annotation for detector, (d) comparison plot with both curves

### Group C: Spectral & Frequency Domain (scripts 08–09)

What Fourier analysis reveals about the field vs the detection record.

**08_spectral_information.py** — *Power spectrum of ψ vs |ψ|²*

WHY THIS MATTERS: F[ψ] gives sharp peaks at source k-vectors with full phase. F[|ψ|²] is the autocorrelation spectrum — it contains beat frequencies and self-convolution artifacts but has lost all absolute phase.

- Compute 2D FFT of ψ (complex → complex): sharp peaks at (±k_x, ±k_y) from each source
- Compute 2D FFT of |ψ|² (real → complex): autocorrelation, peaks at difference frequencies
- Count independent spectral components (above noise floor) in each
- Metric: spectral degrees of freedom in F[ψ] vs F[|ψ|²]
- Figure: (a) |F[ψ]|² power spectrum (k-space), (b) |F[|ψ|²]|² power spectrum, (c) overlay with lost components highlighted, (d) radial power spectrum comparison

**09_which_frequency_where.py** — *The spectrogram the detector smears*

WHY THIS MATTERS: Short-time Fourier transform of ψ along the detection axis gives a spectrogram — local frequency content varying with position. |ψ|² spectrogram is degraded. Detector dots have no spectrogram at all until millions are accumulated.

- Compute STFT of ψ along the midline (y = H/2), window size = 4λ
- Compute STFT of |ψ|² along the same line
- Compute STFT from detector click histogram (1D projection)
- Metric: spectral entropy per position bin for each
- Figure: (a) ψ spectrogram (rich, chirped near sources), (b) |ψ|² spectrogram (degraded), (c) detector histogram spectrogram (noisy), (d) spectral entropy comparison curve

### Group D: Information-Theoretic Measures (scripts 10–12)

The hard numbers.

**10_bits_per_pixel.py** — *Total information at each degradation stage*

WHY THIS MATTERS: The spine of the whole suite. Quantifies the total information budget at each stage of the degradation cascade: ψ → |ψ|² → N clicks → 1 click.

- Full ψ: 2 channels (amplitude + phase), each quantized to detector-equivalent precision → bits/pixel
- |ψ|²: 1 channel (amplitude² only) → bits/pixel
- N clicks: position histogram with Poisson noise → effective bits/pixel
- 1 click: single (x,y) from a 512×512 grid → ~18 bits total
- Metric: bits per pixel at each stage, percentage lost at each transition
- Figure: (a) stacked bar chart of information budget, (b) degradation waterfall diagram, (c) the three fields side by side, (d) text summary of losses

**11_fisher_information.py** — *Statistical efficiency for parameter estimation*

WHY THIS MATTERS: Fisher information quantifies the best possible precision for estimating source parameters from data. The full field gives orders of magnitude better estimates than the same "amount" of detector data. The detector is not just lossy — it's statistically inefficient.

- Parameter: slit separation d. Compute ∂ψ/∂d numerically (Δd = 0.1)
- Fisher information for full field: F_ψ = ∫ |∂ψ/∂d|² / |ψ|² dx dy
- Fisher information for |ψ|²: F_I = ∫ (∂I/∂d)² / I dx dy (classical Fisher)
- Fisher information for N clicks: F_N = N · F_I (standard scaling)
- Cramér-Rao bound: Var(d̂) ≥ 1/F
- Metric: ratio F_ψ / F_I = "how many times more informative the full field is"
- Figure: (a) ∂ψ/∂d sensitivity map (rich structure), (b) ∂I/∂d sensitivity (envelope only), (c) Cramér-Rao bounds vs N_clicks, (d) "clicks needed to match 1 field snapshot" annotation

**12_reconstruction_impossibility.py** — *What can you recover from clicks?*

WHY THIS MATTERS: Given N clicks, the best reconstruction recovers |ψ|² to √N resolution. The phase is provably unrecoverable — infinitely many ψ fields produce the same |ψ|². This is fundamental, not practical.

- Generate K = 5 random phase fields that all produce the same |ψ|²: ψ_k = |ψ| · e^{iφ_k(x,y)} where φ_k are random fields with the constraint that |ψ_k|² = |ψ|²
- Show all 5 are wildly different (different phase topology, different momentum flow) yet identical under the detector
- From N clicks, attempt maximum-likelihood reconstruction of |ψ|² — show convergence
- Attempt phase retrieval (Gerchberg-Saxton) — show failure / non-uniqueness
- Metric: reconstruction fidelity for amplitude (converges with N) vs phase (does not converge)
- Figure: (a) 5 different ψ fields (phase coloring), (b) their identical |ψ|², (c) reconstruction from 10k clicks vs 100k clicks, (d) phase retrieval failure

### Group E: The Ternary Alternative (scripts 13–15)

FTD's response: the void is not nothing.

**13_ternary_vs_boolean.py** — *What ternary states preserve that boolean destroys*

WHY THIS MATTERS: FTD's ternary detector has three outcomes (+1, −1, 0) not two (click / no-click). The sign carries phase information: +1 and −1 at the same position mean constructively vs destructively interfering flux that exceeded K_B in opposite orientations.

- Compute ternary state field: s(x,y) = sign(Re(ψ)) where |ψ| > K_B, else 0
- Boolean field: b(x,y) = 1 where |ψ|² > K_B², else 0
- Shannon entropy: H(ternary) vs H(boolean)
- Mutual information with full phase: I(s; θ) vs I(b; θ)
- Metric: information preserved by ternary vs boolean detection (ratio)
- Figure: (a) ternary state map (+1 red, −1 blue, 0 black), (b) boolean map (white/black), (c) phase field for reference, (d) information comparison bar chart

**14_void_is_destructive_interference.py** — *0 ≠ "nothing happened"*

WHY THIS MATTERS: In the boolean detector, a pixel with no click is ambiguous: was the field weak (low energy) or destructively interfering (high energy cancellation)? These are physically opposite situations. The full ψ distinguishes them immediately.

- Identify "dark" pixels: |ψ|² < threshold
- Classify dark pixels into: (a) genuinely low amplitude (|ψ_A| and |ψ_B| both small), (b) destructive interference (|ψ_A| and |ψ_B| both large but opposing)
- Classification method: compute |ψ_A|² + |ψ_B|² at dark pixels — if large, it's cancellation
- Metric: fraction of dark pixels that are high-energy cancellations vs genuine voids
- Figure: (a) |ψ|² with dark regions, (b) dark regions colored by type (red = cancellation, gray = genuine void), (c) |ψ_A|² + |ψ_B|² at dark pixels (histogram), (d) pie chart of dark pixel classification

**15_parameter_sensitivity.py** — *The detector's sluggish response to change*

WHY THIS MATTERS: Sweep slit separation by 1%. The full ψ changes instantly everywhere. The detector dot pattern takes thousands of additional clicks to statistically resolve the same shift. The ratio is the "cost of going boolean."

- Compute field at separation d and d + 0.01·d
- Full field: compute L² difference ‖ψ(d) − ψ(d+Δd)‖²  — immediate, large
- Detector: for increasing N, compute χ² test between click histograms at d vs d+Δd
- Find N* where χ² first exceeds 3σ significance threshold
- Metric: N* = "clicks needed to notice a 1% parameter change"
- Figure: (a) field difference map |ψ(d) − ψ(d+Δd)|, (b) click histogram difference at N=1000, 10000, 100000, (c) χ² vs N curve with 3σ line, (d) annotation of N*

### Group F: The Cumulative Argument (script 16)

**16_cumulative_report.py** — *The prosecution rests*

WHY THIS MATTERS: Aggregates all 15 previous results into a single information budget. Shows the total bits available, where they go, and what the boolean detector recovers.

- Reads JSON summaries from `output/01_*.json` through `output/15_*.json`
- Computes aggregate metrics:
  - Total phase information destroyed (from 01)
  - Total momentum flow information destroyed (from 02)
  - Topological defects invisible (from 03)
  - Nodal topology lost (from 04)
  - Coherence structure lost (from 05)
  - Cross-term irrecoverability (from 06)
  - Phase locking destroyed (from 07)
  - Spectral components lost (from 08, 09)
  - Bits per pixel at each stage (from 10)
  - Fisher information ratio (from 11)
  - Reconstruction impossibility dimension (from 12)
  - Ternary vs boolean ratio (from 13)
  - Dark pixel misclassification rate (from 14)
  - Parameter sensitivity cost (from 15)
- Figure: (a) information budget waterfall/Sankey diagram, (b) table of all 15 metrics, (c) summary text
- Prints plain-English conclusion: "Of the X bits per pixel available in the double-slit field, the boolean detector screen recovers Y (Z%), destroying the remaining W bits which encoded [list]. This is not a measurement limitation. It is a design choice."

---

## Dependencies

- Python 3.9+
- numpy, scipy, matplotlib
- scikit-image (marching squares for nodal lines in 04)
- No FTD engine dependency — pure numerical computation using Huygens-Fresnel

## Output

Each script produces:
- **PNG figure** in `output/NN_name.png` (300 DPI, publication quality)
- **JSON summary** in `output/NN_name.json` (machine-readable metrics)
- **Console output** with key findings and "WHY THIS MATTERS" framing

Script 16 additionally produces:
- `output/cumulative_report.png` — the aggregated information budget
- `output/cumulative_report.json` — all metrics from all scripts

## Running

```bash
# Individual script
python scripts/experiments/detector_information_loss/01_phase_entropy.py

# All scripts
for f in scripts/experiments/detector_information_loss/[0-1]*.py; do python "$f"; done

# Just the cumulative report (requires all others to have run)
python scripts/experiments/detector_information_loss/16_cumulative_report.py
```
