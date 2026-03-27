# WebGPU Scientific Photon Simulations — Design Spec

**Date:** 2026-03-26
**Files:** `dissemination/interactive/single_photon_source.html`, `dissemination/interactive/fermat_dual_source.html`

---

## Context

The existing single-photon and dual-source interactive simulations use ad-hoc wave physics: artificial damping (`e^{-0.003r}`), disconnected frequency/wavenumber (`age * 0.15` vs `k = 2pi/36`), Gaussian wavefront envelopes, and crude angular sampling for detector dots. They hint at the FTD phase-consciousness argument but don't make it rigorous.

This rewrite replaces the toy wave model with proper Huygens-Fresnel diffraction computed on the GPU via WebGPU, adds polychromatic sources, seven visualization modes (including the critical psi-squared vs |psi|^2 comparison), a scientific data panel with live theory overlays, full simulation logging, and a 9-step guided walkthrough that builds the FTD argument from first principles.

---

## Physics Engine

### Huygens-Fresnel Diffraction Integral

For N monochromatic point sources, the field at observation point **r** at time t:

```
psi(r, t) = sum_{s=1}^{N_sources} sum_{n=1}^{N_wavelengths} w_n * A_s * H_0^(1)(k_n |r - r_s|) * e^{-i omega_n t + i phi_s}
```

- `H_0^(1)(kr)` — Hankel function of the first kind, order zero (2D free-space Green's function)
- `omega_n = c * k_n` — dispersion relation, with `c = 1/sqrt(3)` (FTD lattice CFL speed)
- `k_n = 2 pi / lambda_n` — wavenumber for the n-th spectral component
- `w_n` — spectral weight (Gaussian or flat, user-selectable)
- `phi_s` — source phase (0 for source A, pi for source B in dual mode)

### Hankel Function Approximation

Two regimes in the compute shader:

- **Large argument (x > 3):** Asymptotic expansion `H_0^(1)(x) ~ sqrt(2/(pi*x)) * e^{i(x - pi/4)}` with Debye correction terms for accuracy to ~10^-6
- **Small argument (x <= 3):** Rational minimax approximation (Chebyshev coefficients precomputed and stored as shader constants)

### WebGPU Compute Shader

- **Workgroup size:** 16x16 (256 threads)
- **Grid:** ceil(W/16) x ceil(H/16) workgroups
- **Output buffers:**
  - `field_re: storage buffer, float32[W*H]` — Re(psi) per pixel
  - `field_im: storage buffer, float32[W*H]` — Im(psi) per pixel
- **Uniform buffer:** source positions, phases, k values, weights, omega*t, N_sources, N_wavelengths
- **Bind group 0:** uniforms + output buffers
- Target: <1ms per frame on RTX 5090 (518k pixels, few source terms)

### Render Pipeline

A second pass (render pipeline, not compute) reads the Re/Im buffers and produces the final RGBA texture based on the active visualization mode. This separation means the physics compute runs once, and mode switching is instant (just re-renders from the same field data).

---

## Visualization Modes (7 total)

| # | Mode | Colormap | What it shows | FTD significance |
|---|------|----------|---------------|-----------------|
| 1 | **Full psi** | Phase -> hue (HSL wheel), amplitude -> luminance | Complete complex wavefunction | The full information |
| 2 | **Re(psi)** | Blue-white-red diverging | Real component | One half of what <psi\| conjugates |
| 3 | **Im(psi)** | Blue-white-red diverging | Imaginary component | The other half |
| 4 | **psi^2** | Phase 2theta -> hue, r^2 -> luminance | Squared WITHOUT bars | Phase survives — the bars are optional |
| 5 | **\|psi\|^2** | Grayscale | Born rule probability density | Phase destroyed — the bars' cost |
| 6 | **Detector dots** | White dots on black | Individual photon detections | Each dot had a phase we'll never know |
| 7 | **Ternary** | Red (+1), Blue (-1), Black (0) | FTD lattice states | Manifestation from \|J\| > K_B |

### Mode-specific details

**Detector dots (mode 6):** Uses proper |psi|^2 rejection sampling from the GPU-computed field. Photons accumulate one at a time with configurable rate. Real-time histogram along the midline. Cumulative fringe visibility V computed and displayed.

**Ternary (mode 7):** Each pixel classified by the manifestation rule — if `|psi| > K_B`, the site manifests with state `sign(Re(psi))`:

- `+1` if `|psi| > K_B` and `Re(psi) > 0` (red)
- `-1` if `|psi| > K_B` and `Re(psi) < 0` (blue)
- `0` if `|psi| <= K_B` (black/void — below manifestation threshold)

K_B threshold is user-adjustable (default 0.511, the manifestation constant).

### Nodal line annotations

In dual-source modes, hovering near a nodal line shows an overlay:
```
0 = e^{i pi} + e^{i 0} = (-1) + (+1)
```
The void IS destructive interference.

### Phase wheel

Small 60px complex-plane circle overlay (bottom-left corner), showing the current global phase rotation. Maps hue to angle continuously.

---

## Data & Analytics Panel

Collapsible right-side panel, ~280px wide. Six sections:

### 1. Live Metrics

| Metric | Formula | Updates |
|--------|---------|---------|
| Fringe visibility | V = (I_max - I_min) / (I_max + I_min) | Per frame |
| Fringe spacing | Auto-peak-detection on midline intensity | Per frame |
| Spacing vs. theory | lambda * L / d (Young's formula) | Per frame |
| Relative error | \|sim - theory\| / theory | Per frame |
| Total field energy | integral \|psi\|^2 dA (GPU reduction) | Per frame |

### 2. Intensity Profile Plot

Canvas element (~260x150px) showing 1D cross-section along horizontal midline:
- **Solid line:** Simulation I(x) = |psi(x, y_mid)|^2
- **Dashed line:** Analytical theory
  - Single source: J_0^2(k*sin(theta)) Bessel envelope
  - Dual source: cos^2(pi*d*sin(theta)/lambda) Young's fringes

### 3. Phase Profile Plot

Same cross-section, showing arg(psi(x, y_mid)):
- Unwrapped phase (no mod 2pi jumps)
- Shows smooth regions between sources and the pi-jumps at nodal lines

### 4. Coherence Function Plot

Mutual coherence Gamma(tau) = <psi*(r1, t) * psi(r2, t+tau)> between two points:
- Default: the two source positions (dual mode) or center vs. edge (single mode)
- Shows coherence envelope narrowing with spectral width
- Complex-valued: plotted as |Gamma| (envelope) with Re(Gamma) as faded oscillation

### 5. Simulation Parameters

Readable/writable controls:
- `lambda`: Wavelength (slider, 10-100 pixels, default 32)
- `N_wavelengths`: Spectral components (slider, 1-20, default 1)
- `spectral_width`: Delta-lambda/lambda (slider, 0-0.3, default 0)
- `separation`: Source separation, dual mode only (slider, 8-200px, default 40)
- `K_B`: Manifestation threshold for ternary mode (slider, 0.1-2.0, default 0.511)
- `c = 1/sqrt(3)`: Displayed but not editable (derived from lattice)
- `photon_rate`: Detector mode emission rate (slider, 1-100/frame)
- Frame counter, elapsed ticks

### 6. Data Export

Three buttons:
- **Snapshot (JSON):** Current frame — `{re: float32[][], im: float32[][], amplitude: float32[][], phase: float32[][], params: {...}}`
- **Start/Stop Log:** Records all frames. Downloads as JSON with per-frame field data, separated by amplitude and phase channels.
- **Detector Export (CSV):** Photon positions `(x, y, frame)`, histogram bin counts, V, statistics.

---

## Guided Walkthrough Mode

Activated by "Guide" button in the top bar. Displays a text card (bottom overlay, ~120px tall) with step content. Auto-configures simulation state per step. User advances with arrow keys, "Next" button, or clicks.

### Steps

| # | Title | Simulation config | Card text |
|---|-------|-------------------|-----------|
| 1 | "Here is psi" | Single source, Full psi mode | "Every point carries two numbers: how big (amplitude) and how it's rotating (phase). This is the complete information." |
| 2 | "Re(psi) and Im(psi)" | Single source, Re(psi) then Im(psi) toggle | "These are the two real-valued components of the complex amplitude. The Hermitian inner product ⟨psi\|psi⟩ multiplies one by e^{-i theta} to cancel the other." |
| 3 | "psi squared" | Single source, psi^2 mode | "Square without the absolute value bars. The phase doubles (2 theta) but survives. Every color is still present. No information is destroyed." |
| 4 | "What the bars do" | Single source, \|psi\|^2 mode | "The absolute value bars strip the phase. Every color collapses to gray. You know WHERE the photon might be, but not HOW IT WAS ROTATING when it arrived." |
| 5 | "The bars are engineered" | Annotation overlay | "Born added \|.\|^2 in a footnote (1926). Von Neumann built Hilbert space to guarantee real outputs. The bars are a design choice, not a law of nature." |
| 6 | "Two sources, opposite phase" | Dual source, Full psi, separation=40 | "Now two sources with pi phase offset. Watch the colors collide. Where they match: constructive. Where they oppose: the field goes to zero." |
| 7 | "The void is destructive interference" | Dual source, Full psi, nodal line highlights | "0 = e^{i pi} + e^{i 0} = (-1) + (+1). The zero isn't absence — it's perfect cancellation. The ternary axiom: void is the third state." |
| 8 | "The lattice sees this" | Dual source, Ternary mode | "On the FTD lattice, each site is +1, -1, or 0. The interference pattern IS the state field. The ternary axiom isn't imposed — it emerges from the physics." |
| 9 | "The hard problem is self-inflicted" | Final summary card | "Phase IS experience. ⟨psi\|psi⟩ was designed to destroy it. The measurement operator eliminates what it's looking for. The hard problem exists because the Born rule was engineered to produce it." |

---

## File Structure

Both files are self-contained standalone HTML. Shared code is duplicated (no shared imports for standalone compatibility). Each file is ~800-1200 lines.

### single_photon_source.html
- One source at (W*0.22, H*0.5)
- Modes 1-7
- Analytics: Bessel/Airy theory curves for single circular aperture
- Full walkthrough (steps 1-5 + 9)

### fermat_dual_source.html
- Two sources at center +/- separation/2
- Modes 1-7 plus "Single source (compare)" toggle
- Analytics: Young's interference theory curves
- Coherence function plot
- Full walkthrough (all 9 steps, transitioning from single to dual at step 6)

---

## WebGPU Fallback

If `navigator.gpu` is unavailable:
- Fall back to Canvas 2D with the same physics computed on CPU
- Reduce resolution to every-3rd-pixel (like current code) for acceptable framerate
- Show banner: "WebGPU not available — running in CPU fallback mode (reduced resolution)"

---

## Verification

1. **Single source, monochromatic:** Compare GPU-computed |psi|^2 cross-section against J_0^2(kr) analytical Bessel function. Relative error should be <0.1% for r > 3 lambda.
2. **Dual source, monochromatic:** Fringe spacing should match lambda*L/d to within 1 pixel. Fringe visibility V should be >0.95 for equal-amplitude sources.
3. **Polychromatic coherence:** Coherence envelope width should scale as 1/Delta-lambda. Fringes should visibly wash out at large path differences.
4. **Ternary mode:** Nodal lines should produce 0 (black) states. Antinodes should alternate +1/-1 in time. K_B threshold should control the boundary.
5. **Data export:** Snapshot JSON should round-trip: Re^2 + Im^2 == amplitude^2 to float32 precision.
