# Pre-Registration - Helium Lattice Spectrum Protocol (v1)

> **STATUS: DRAFT - NOT LOCKED, NOT RUN.** This protocol prepares a substrate-native
> measurement campaign for the Scale-0 helium seed. It is not hash-locked, tagged,
> or executed. No result document may cite this file as evidence until the owner
> explicitly locks the preregistration before the first campaign run.

**Date:** 2026-06-11
**Branch:** `codex/helium-spectrum-lattice`
**Proposed tag:** `preregister-helium-lattice-spectrum-v1`
**Primary scenario:** `s0-seed-helium`
**Primary instrument:** `engine/web/js/scales/scale0/analysis/lattice-spectrum.js`
**Campaign helper:** `engine/web/js/scales/scale0/analysis/helium-spectrum-protocol.js`

---

## 0. Honest Scope

This protocol asks whether the existing Scale-0 helium seed has a reproducible,
substrate-native spectral fingerprint on the lattice.

It does **not** claim to derive the laboratory helium emission spectrum. The
first admissible output is a lattice-field measurement in simulation units:
spatial wave number `k` in radians per voxel, temporal frequency in cycles per
tick, and dimensionless shape metrics. Any later comparison to physical helium
lines requires a separate calibration preregistration and must be labeled as a
comparison or parametric insertion unless the calibration itself is derived.

The matter-substrate derivation ladder is therefore:

1. **Instrument check:** the FFT and readouts pass Parseval and determinism gates.
2. **Substrate fingerprint:** He-4 is reproducibly distinguishable from frozen
   controls under predeclared metrics.
3. **Line-candidate test:** a time-domain FTD-native observable has stable peaks
   across duplicate runs, lattice sizes, and perturbation axes.
4. **Physical-spectrum bridge:** only after a separate unit/calibration protocol.

Only steps 1-3 are in scope here.

---

## 1. Question

**Q-HE-SPEC-v1:** Under the existing Scale-0 rules and the current `s0-seed-helium`
construction, does the helium seed generate a reproducible lattice spectral
signature that is distinguishable from predeclared controls without fitting to
external helium data?

---

## 2. Frozen Definitions

**D1 - Helium seed.** `s0-seed-helium`: 2 proton triads, 2 neutron triads, and 2
electron seeds in a 1s-like shell, as implemented in both JS and C++ scenario
paths.

**D2 - Spatial substrate spectrum.** `E_J(k)` is the shell-binned FFT power of
the flux field `J`, computed by `energySpectrum()`. The Parseval ratio is
`sum_k E_J(k) / sum_x |J(x)|^2`.

**D3 - Spatial fingerprint.** The frozen fingerprint vector is:

`[log1p(totalPower), kPeak, centroidK, bandwidthK, spectralEntropy, irFraction,
midFraction, uvFraction, slope, parsevalRatio, chargeDipoleMagnitude]`.

All entries are computed in simulation units. `ir/mid/uv` are fractions of the
normalized spectral power in the lower 25%, middle 50%, and upper 25% of the
resolved `k` band.

**D4 - Time-domain line candidate.** The primary line-candidate observable is
the charged-particle dipole

`D_q(t) = sum_i q_i * (r_i(t) - r0)`

using `getScale0ParticleList()` charges/states and `r0 = (L/2,L/2,L/2)`. The
radiative proxy is the second difference `A_q(t) = D_q(t)-2D_q(t-1)+D_q(t-2)`.
The line-candidate spectrum is the Hann-windowed FFT power of `|A_q(t)|`.

**D5 - Distinguishability metric.** Spectrum-to-spectrum distance is reported as
L1 distance, Jensen-Shannon divergence, Hellinger distance, and cosine
similarity between normalized, common-grid spectral weights. No one metric may
be replaced after looking at the helium results.

**D6 - Reproducibility radius.** A duplicate run is admissible only when the
same scenario/config produces a fingerprint distance less than the predeclared
within-run ceiling in Section 6.

---

## 3. Apparatus

Backend:

- Primary: browser WASM bridge.
- Optional parity: JS MockBridge only as a debug/instrument comparison, not as
  the verdict backend.

Scenarios:

- Subject: `s0-seed-helium`.
- Controls: `empty`, `s0-seed-hydrogen`, `s0-seed-h2-bond-formation`,
  `s0-seed-moore-cell`, `s0-seed-octahedron`.

Configuration:

- Lattice sizes: L=64 primary, L=97 blind extension.
- Scenario default toggles are retained and recorded after load. For helium this
  means genesis off and the scenario's force/confinement defaults on.
- The app is paused; advancement uses explicit bridge ticks.
- Random or stochastic toggles must be off unless the scenario default requires
  them and the seed is recorded.

Readouts:

- `getScale0FieldSamples({ kind: 'fluxVector', stride: 1 })`
- `getScale0ParticleList()`
- `getScale0Diagnostics()`
- `getScale0EnergyAudit()`

---

## 4. Spatial Measurement

For each scenario and lattice size:

1. Load scenario fresh.
2. Record diagnostics/audit/toggles at tick 0.
3. Advance to ticks `{0, 20, 80, 160}`.
4. At each checkpoint compute `E_J(k)` with `M=64`. This is full-band for L=64
   and explicitly band-limited for L=97. A later protocol may add a corrected
   higher-resolution L=97 FFT, but v1 does not treat interpolation above the
   lattice Nyquist as new physical bandwidth.
5. Record the fingerprint vector D3 and the Parseval ratio D2.

The spatial verdict is based on tick 80. Other checkpoints are stability and
drift diagnostics, not a post-hoc opportunity to choose a better tick.

---

## 5. Time-Domain Line-Candidate Measurement

For each of the three perturbation axes `{x,y,z}`:

1. Load `s0-seed-helium` fresh at L=64.
2. Advance 80 settle ticks.
3. Apply a weak antisymmetric flux kick:
   - `epsilon = 0.05 * K_B`
   - at `r0 + 2*axis`: `+epsilon * axis`
   - at `r0 - 2*axis`: `-epsilon * axis`
4. Record `D_q(t)` for 512 ticks.
5. Compute `A_q(t)` by second difference.
6. Compute the Hann-windowed FFT power of `|A_q(t)|`.
7. Report the top five local maxima after excluding DC and the first two bins.

The same run must be repeated once with a fresh scenario load. Peak agreement is
assessed by predeclared frequency windows; peak amplitudes are descriptive only.

---

## 6. Predictions, Outcomes, and Falsifiers

### P0 - Instrument Validity

Prediction: every spatial spectrum has Parseval ratio in `[0.95, 1.05]`.

Falsifier: any subject/control spectrum outside this range is inadmissible until
the instrument or sampler mismatch is fixed. No helium claim may be made.

### P1 - Deterministic Reproducibility

Prediction: duplicate fingerprints for the same scenario/config have
`JS <= 0.02` and `L1 <= 0.15`.

Falsifier: duplicate distances exceed either ceiling. Verdict:
UNDERDETERMINED-instrument or CLOSED-NEGATIVE for the current seed/config,
depending on whether the failing cause is technical or physical.

### P2 - Helium Spatial Distinguishability

Prediction: the helium tick-80 fingerprint is farther from every listed control
than from its duplicate by a margin of at least `3x` in JS divergence and `2x`
in L1 distance.

Falsifier: any control lies inside those margins. Verdict: no resolved helium
substrate fingerprint under v1.

### P3 - Line-Candidate Stability

Prediction: for each perturbation axis, at least three of the top five non-DC
frequency peaks repeat across duplicate runs within `+/- 1` FFT bin.

Falsifier: fewer than three peaks repeat on any axis. Verdict: no stable
line-candidate spectrum under v1.

### P4 - Lattice-Size Sanity

Prediction: the rank ordering of helium-vs-control spatial distances at L=97
matches L=64 for the top three farthest controls.

Falsifier: rank ordering changes completely or the helium/control separation
collapses below the P2 margin. Verdict: finite-size artifact unless a follow-up
explains the scaling.

---

## 7. Banned Moves

- Do not compare peak positions to NIST/laboratory helium lines in this protocol.
- Do not rescale tick frequency, voxel length, or energy units to make a peak
  match a physical spectrum.
- Do not choose a checkpoint, perturbation strength, window, FFT length, or
  distance metric after seeing helium results.
- Do not add controls after seeing the subject result and still call them
  predeclared controls.
- Do not promote a spatial substrate fingerprint to an atomic emission spectrum.
- Do not call a standard-physics formula with FTD-measured inputs a derivation.
- Do not run near-miss or coincidence searches.

---

## 8. Lock-and-Run Procedure

Before any campaign run:

1. Finalize this file.
2. Record SHA256 of this preregistration and the campaign helper.
3. Add the campaign row to `REF_PREREGISTER_MANIFEST.md`.
4. Commit and tag `preregister-helium-lattice-spectrum-v1`.
5. Run only from the tagged commit.
6. Write results to a separate analysis document. Do not edit this prereg after
   the run except to mark it superseded by a later version.

---

## 9. One-Line Summary

v1 measures whether the Scale-0 helium seed has a reproducible lattice-native
spatial fingerprint and a stable internal line-candidate spectrum; it explicitly
does not derive or fit the laboratory helium spectrum.
