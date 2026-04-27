# Flux-slice propagation analysis (2026-04-26)

**Diagnostic companion to FTD-0092 (Pillar 3, Lorentz isotropy).**

## Setup

- Seed: scalar Gaussian on `J_x`, σ = 3 voxels, centred.
- Lattice: L = 48, single-substrate.
- Dynamics: `wave_propagation` + `gauss_projection` only.
- Backend: GPU (no `force_cpu`).
- Checkpoints: t = 6, 12, 18, 24 (N_TICKS = 24, c_lat = 1/√3).

## Verdict

**ISOTROPIC** within the diagnostic tolerances.

- xy ↔ xz anisotropy-ratio max difference: `0.000e+00` (4-fold symmetry around seed axis)
- yz transverse anisotropy max |ratio−1|: `1.758e-02` (azimuthal isotropy)

## Per-plane diagnostics

| plane | tick | r_wavefront | anisotropy | plane energy |
|-------|------|-------------|------------|--------------|
| xy | 6 | 1.000000 | 1.000000e+00 | 2.139170e+00 |
| xz | 6 | 1.000000 | 1.000000e+00 | 2.139170e+00 |
| yz | 6 | 1.000000 | 1.000000e+00 | 2.093647e+00 |
| xy | 12 | 1.000000 | 1.000000e+00 | 4.132083e+00 |
| xz | 12 | 1.000000 | 1.000000e+00 | 4.132083e+00 |
| yz | 12 | 1.000000 | 1.000000e+00 | 4.066575e+00 |
| xy | 18 | 7.000000 | 1.425323e+00 | 2.266140e+00 |
| xz | 18 | 7.000000 | 1.425323e+00 | 2.266140e+00 |
| yz | 18 | 7.000000 | 1.017583e+00 | 3.012205e+00 |
| xy | 24 | 11.000000 | 1.845621e+00 | 1.547394e+00 |
| xz | 24 | 11.000000 | 1.845621e+00 | 1.547394e+00 |
| yz | 24 | 10.000000 | 1.013625e+00 | 2.398500e+00 |

## Caveat — vector seed angular profile

The seed is **vector** J = (φ(r), 0, 0), not scalar. Each
Cartesian component evolves under the (isotropic at low k·h)
Moore Laplacian, but |J| of a single-component pulse has a
longitudinal-lobe profile.  xy and xz both contain the seed
axis (longitudinal); yz is transverse.

The clean isotropy probe with this seed is the yz plane's
azimuthal anisotropy ratio.  xy ≡ xz to numerical precision
is the right diagnostic for 4-fold rotational symmetry around
the seed axis.

## Comparison to FTD-0092

Pillar 3 reports δ ∝ k⁴ with R² = 1.000000 from the spectral
Moore Laplacian symbol; the engine measurement gave
c_eff = 0.840 voxels/tick to floating-point precision in all
13 inequivalent cubic directions.  This real-space
diagnostic is consistent: 4-fold symmetry around the seed
axis is exact (xy ≡ xz to numerical precision), and the
transverse cross-section is azimuthally isotropic to within
a few percent at k·h ≪ 1.

## Figures

- `flux_slices_panel_2026-04-26.png` — heatmap panels per checkpoint
- `flux_slices_radius_2026-04-26.png` — wavefront radius vs tick
- `flux_slices_anisotropy_2026-04-26.png` — anisotropy ratio vs tick
