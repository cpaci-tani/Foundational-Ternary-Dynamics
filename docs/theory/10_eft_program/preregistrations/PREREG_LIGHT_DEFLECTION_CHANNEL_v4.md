# PREREG — The Gravitational-Optical Channel v4 (final gate repair + unseen-b verdict arm)

**Tag:** [PRE-REGISTRATION — ENGINE MEASUREMENT] (LOCK-STD v1; git tag `preregister-light-deflection-channel-v4` at the registration commit)
**Parents:** v1 (Indeterminate: wrap artifact + capture) → v2 (VOID: uncalibrated retention gate) → v3 (Indeterminate: two gate-formula defects, physics stable). **Question, geometry, toggle set, constructions, fit windows, θ_γ0 formula, exclusions: verbatim from the chain.** This lock repairs the two v3 gate defects and adds genuinely fresh verdict-bearing content so the reformed gates are not applied to already-seen data.

## 1 · The two v3 gate defects (on record)

1. **V4 was algebraically unsatisfiable:** Floor included 2·|θ_S| while V4 demanded |θ_S| < ½·Floor — when the S-term dominates, the gate reads |θ_S| < |θ_S|. Defect of formula, not of physics.
2. **The raw θ_S scale over-penalizes:** the S-arm (mass-only) centroid is weighted by the *contamination alone* (window energy Ē_S ≈ 10⁻²–10⁻¹), while in a W arm the same contamination is diluted by the packet's energy (Ē_W ≈ 7 at the exit fit). The contamination's influence on a W-arm angle scales as θ_S · (Ē_S/Ē_W), not as θ_S.

## 2 · Changes (v3 → v4)

1. **V4 (repaired):** contamination influence θ_S^eff ≡ |θ_S| · (Ē_S/Ē_W), with Ē the mean window energy over the exit-fit ticks. Gate: θ_S^eff < ½ · Floor. **Floor (repaired):** max(3·|θ_z| over all W arms, 3·|θ_y(C0)|, 3·θ_S^eff).
2. **V2 (repaired):** the well-acts gate accepts EITHER transit (exit v_x > 0.25 and |θ_p| > 10·Floor) OR **capture** (exit v_x < 0.1 with the particle alive and displaced toward the mass) — capture is the stronger demonstration that the gravitational sector grips matter. Fresh arm **P-b26** (particle at (30, 48, 48), v = (0.5,0,0), mass at (48, 48−26, 48)) is the V2-bearing arm.
3. **Fresh verdict-bearing arm W-b18** (mass at (48, 48−18, 48)) — never measured in any prior cycle. **Frozen prediction (the code-derived null, stated as such):** |θ_diff(b18)| < 0.05 · θ_γ0(18), same-floor-class as b10/b14, no consistent toward-mass sign across the three b's.
4. The verdict applies the v1 §5 outcome map to **all three** W arms (b ∈ {10, 14, 18}) with the repaired Floor; Outcome N requires all three inside the N-band and Floor < 0.05·θ_γ0(b) for at least b=10 (the deepest yardstick). G outcomes require consistent toward-mass sign at all three b with magnitudes tracking θ_γ0(b) within ±35%.
5. Analysis script updated to compute θ_S^eff, Ē ratios, and the three-b verdict inputs. No other instrument change (arms C0/W-b10/W-b14/D-b10/S-b10 re-run identically; the engine is deterministic, so their values re-verify the v3 characterization rather than constituting new looks).

## 3 · Stopping rule

This is the final instrument cycle of the v1 chain. If v4's gates fail again, the campaign is closed as **[INSTRUMENT-LIMITED — Indeterminate]** with the characterization of record (optical response < 5% of the g₀₀ yardstick at demonstrated sensitivity; matter captured by the same well) and any further attempt requires a fresh campaign design under a new name — not a v5.

---

## OUTCOME (2026-07-18, adjudicated after the run) — VOID on V2; stopping rule fired; campaign CLOSED [INSTRUMENT-LIMITED — Indeterminate]

**Gate results.** V1 ✓ (latency_max 0.540). V3 ✓ (|θ_C0| = 4.9×10⁻¹⁴; retention 0.349–0.351 ≥ 0.25). V4 ✓ (θ_S_eff = 1.29×10⁻⁴ < Floor/2 = 9.04×10⁻⁴; Floor = 1.81×10⁻³ from the z-channels). **V2 ✗ — and the failure identified a four-cycle instrument defect:** the P-b26 PROW trajectory shows the particle at (30, 48, 48) with velocity ≈ 0 for all 72 ticks — `inject_particle`'s velocity argument is not applied as kinematic velocity (the engine's scenario helpers set `voxel.velocity` separately via `SET_VEL` for exactly this reason). Every particle arm in v1–v4 launched a particle AT REST; the v1/v3 "capture" readings (exit v_x ≈ 0) were this defect, not gravitational capture. Neither the transit clause nor the capture clause (which requires toward-mass displacement; the particle is undisplaced) is met. Per §3, no v5: **closed.**

**Characterization of record (instrument-characterization grade; books no outcome-map verdict):**
- The wave-side instrument is strongly validated: the mass-free control reads machine zero (4.9×10⁻¹⁴); the contamination arm, energy-ratio-scaled, is sub-floor; the floor is 2.6% of θ_γ0 at b=10.
- Across b ∈ {10, 14, 18}: θ_diff = −0.80 / +1.64 / +1.59 ×10⁻³ — all inside the Floor band, sign-inconsistent (no toward-mass trend), 1.1–7.9% of the shrinking θ_γ0 yardstick. The v4 frozen prediction for the unseen b=18 (< 0.05·θ_γ0) failed narrowly by its letter (0.079·θ_γ0) — the away-signed floor-scale residual does not shrink with b as a gravitational signal would; it tracks the artifact floor.
- This is *consistent with* the code-derived structural null (the wave operator carries no latency term) at the few-percent-of-θ_γ0 level, but the formal Outcome N is **not booked**: V2's purpose — demonstrating the well dynamically grips matter — was never achieved, so "no optical response" is not yet discriminated from "no dynamical response to this source at all."

**Successor path (new campaign, new name, per the stopping rule):** identical wave-side instrument + a particle arm using the `SET_VEL` idiom (and a verified transit at feasible speed), which would make the V2 gate decidable and, if the present numbers reproduce, book the formal [MEASURED STRUCTURAL NULL] for the gravitational-optical channel.
