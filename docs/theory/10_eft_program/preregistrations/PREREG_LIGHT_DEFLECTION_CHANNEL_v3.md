# PREREG — The Gravitational-Optical Channel v3 (calibrated gates + contamination arm)

**Tag:** [PRE-REGISTRATION — ENGINE MEASUREMENT] (LOCK-STD v1; git tag `preregister-light-deflection-channel-v3` at the registration commit)
**Parents:** v1 (Indeterminate: boundary-wrap artifact + particle capture) → v2 (VOID on its own V3 gate: the 50% window-retention threshold was uncalibrated for lattice dispersion — retention is 0.35 in EVERY arm including the mass-free control, while the same control demonstrates the centroid observable intact at machine precision, |θ_C0| ≈ 5×10⁻¹⁴). **Question, physics scope, geometry, toggle set, packet/mass construction, fit windows, θ_γ0 formula, outcome philosophy and anti-gaming clauses: v2/v1 verbatim.** This lock registers gate recalibration + one new arm.

## 1 · Changes (v2 → v3)

1. **V3 recalibrated (packet integrity).** Two-part gate replaces the bare 50% retention: (a) direct observable-integrity: |θ_y(C0)| ≤ 10⁻⁶ (the mass-free twin must read zero — the strongest possible demonstration that dispersion has not destroyed the centroid); (b) retention ≥ 0.25 at the exit-fit midpoint (calibrated from the v2 void-run characterization: symmetric lattice dispersion of the σ=5 packet gives 0.35; below ~0.25 the exit fit is statistics-starved). Justification on record: in the v2 void run, retention 0.35 coexisted with a 10⁻¹⁴-level control null — window retention was measuring dispersion, not observable validity.
2. **New S-arm (static-contamination instrument, feeds V4).** Mass at (48, 48−10, 48), NO packet: after the identical equilibration + baseline snapshot, evolve the same 110 ticks and compute the identical windowed difference-centroid time series. Its fitted "angle" θ_S is the pure baseline-evolution contamination signal. **V4 (replaces v1's per-voxel drift bound): |θ_S| < ½ · Floor.** The v2 void run showed a small away-signed W-arm signal (≈ +5–7×10⁻³) that grows with b and survives damping-off — the S-arm decides whether it is baseline contamination.
3. **Equilibration extended:** T_eq 60 → 200 ticks (shrinks the settling drift the S-arm measures).
4. **Analysis-script keys fixed** (P-b20 arm name; θ_diff printed explicitly; S-arm handling). No physics content.

## 2 · Outcome map

v1 §5 verbatim on θ_diff(b) = θ_w(W-b) − θ_w(C0), with Floor = max(3·|θ_z| over W arms, 3·|θ_y(C0)|, 2·|θ_S|). Sign/absorption clause unchanged. If the W-arm signal is away-signed, exceeds Floor, and the S-arm accounts for it (|θ_S| comparable), the away-signal is booked as baseline contamination and the gravitational verdict is taken from |θ_diff| vs Floor with the S-contribution subtracted; if the S-arm does NOT account for it, Indeterminate with the anomaly characterized (candidate: Gauss-projector/matter constraint-texture scattering — a non-gravitational contact channel; any such booking is [MEASURED, UNEXPLAINED], no tag).

---

*Registered 2026-07-18, before the v3 instrument's first execution. Author: session 8294fddb, following LOCK-STD v1.*
