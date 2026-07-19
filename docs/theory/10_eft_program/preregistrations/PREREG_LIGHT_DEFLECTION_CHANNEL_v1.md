# PREREG — The Gravitational-Optical Channel: does the substrate bend light?

**Tag:** [PRE-REGISTRATION — ENGINE MEASUREMENT] (LOCK-STD v1; git tag `preregister-light-deflection-channel-v1` at the registration commit)
**Question owner:** Gate 2 of the gravity-unification threshold (session record 2026-07-18): FTD's gravity sector of record is g₀₀-only (Newton per `DERIV_NEWTON_FROM_SUBSTRATE.md` with the clock hypothesis [AXIOM]; g_rr RETRACTED per FTD-0361; emergent spin-2 [CLOSED NEGATIVE] in the probed regime per FTD-0193). Light deflection is the classic discriminator: a null geodesic in a g₀₀-only metric bends at HALF the GR value; the other half is g_rr. **This campaign decides the prior question: whether the substrate's wave sector couples to the gravitational (latency) field at all.**
**Engine state at lock:** post-Term-2 electric-coupling sign amendment (2026-07-18) and post-FTD-0388 kinetics (K_GENESIS = 3·W_SC = 1.5163860591519780). The instrument is born after both; no engine-drift ambiguity.

---

## 1 · The question, sharply

Inject a transverse traveling flux packet ("light": the canonical photon-pulse construction) past a static, charge-neutral, locked massive body that sources the latency field ("gravity": the g₀₀ sector of record). Measure the packet's transverse deflection.

**Code-derived expectation, stated as such [not a measurement]:** `phase_read` contains no latency term — the vacuum wave operator is `c²∇²` with constant c; `v.latency` is consumed only by the particle-force sector (`phase_forces.cpp`). Superposition then predicts ZERO deflection of a sub-threshold packet. The expectation is a **structural null**. The campaign exists because code-derived expectations are not measurements: this week the same engine was found enforcing its central constraint at −9.5% while every prior reading assumed it enforced at ~100%. Nonlinear side channels (Gauss-projector warm-start coupling, selective-damping halos, boundary interplay) are exactly the kind of thing that only a measurement excludes.

## 2 · Instrument

`engine/tests/campaign_light_deflection.cpp`, locked at the registration commit. CPU (`force_cpu`), deterministic, no RNG. Single process runs all arms and emits CSV measurement data only; the verdict is applied afterward against §5.

**Common configuration (all arms):**
- L = 96, periodic canonical lattice; `set_sor_iterations(20)` (solver accuracy, mirrors PREREG graviton v2 §8).
- Toggle set: the graviton-v2 canonical eleven (`wave_propagation, coupling, gauss_projection, genesis, forces, gravity, poisson_coulomb, lorentz_force, movement, damping, selective_damping`) **plus `latency_field`** (the gravitational sector of record — `ρ_mass = M_REST·|s|` latency-Poisson, as in `s0-seed-massive-body`). Everything else OFF.
- **Mass:** ball of radius R_m = 3 voxels at the lattice center, **locked** (static: skipped by movement + evaporation), **charge-neutral** (alternating ±1 by site parity — deviation from the all-+1 `s0-seed-massive-body` idiom, declared here: neutrality isolates the gravitational channel from the electrostatic sector; ρ_mass = M_REST·|s| is sign-blind).
- **Packet:** photon-pulse construction (`s0_field.cpp` idiom): z-polarized, x-propagating, J_z = A·g(r)·sin(k(x−x₀)), wave_vel_z = −ω·A·g(r)·cos(k(x−x₀)), σ = 5, λ = 4σ, ω = 2c·sin(k/2), **A = K_B·0.5 ≈ 0.256** (deep sub-threshold: |J| ≤ 0.26 ≪ K_GENESIS = 1.516; genesis inert on the packet by margin > 5×).
- **Geometry:** packet launched centered at (x₀, y_c + b, z_c) = (20, 48+b, 48); measured through transit to x ≈ 76 (~100 ticks at c = 1/√3); impact parameters **b ∈ {10, 14}**.
- **Static baseline subtraction:** after seeding the mass, equilibrate T_eq = 60 ticks, snapshot J_static, verify staticity (10 further ticks; max per-voxel drift recorded — feeds the floor), then inject the packet. Packet field ≡ J(t) − J_static; all packet observables are computed on this difference field inside a moving window |x − x_pack(t)| ≤ 12.

**Arms:**
| Arm | Config | Measures |
|---|---|---|
| W-b10, W-b14 | mass + packet, b = 10, 14 | transverse centroid trajectory y(t), z(t) of the packet difference-field (|J_pkt|² weight); deflection δy = ⟨y⟩_exit − ⟨y⟩_entry; angle θ_w = δv_y/v_x from linear fits |
| C0 | NO mass + packet (b = 10 geometry) | numerical floor: |δy|_C0 |
| P-b10 | mass + test particle (+1, unlocked, v = (0.5, 0, 0)) launched at (30, 48+b, 48) | Newtonian-sector validity: θ_p from the particle's y-velocity change over its transit (fit windows t ∈ [4,16] and [56,68]) |
| Z-null | z-centroid of W arms | in-run symmetry null: mass offset is in y only; δz is a same-run floor replica |
| D-b10 | **diagnostic, non-verdict-bearing:** W-b10 repeated with `damping` + `selective_damping` OFF | separates selective-damping absorption asymmetry (which pushes the centroid AWAY from the mass) from gravitational deflection (toward), if the W arms show signal |

**Particle survival dressing (required by the FTD-0388 evaporation kinetics):** evaporation fires at p = K_EVAP_RATE·exp(−E_local/K_MANIFEST²) with E_local summed over the site + 6 face neighbors (`phase_write.cpp:362-379`); a bare particle (E_local ≈ 0) dies in ~10 ticks. The test particle is therefore dressed at injection with z-polarized flux: 1.45 at its site, 0.55 at each face neighbor (all far below K_GENESIS = 1.516 — no spurious manifestation; E_local ≈ 3.9 → p ≈ 2×10⁻⁸/tick initially, remaining < 10⁻³/tick through the transit under damping decay). Deterministic RNG: survival is reproducible. Death before the exit-fit window ⇒ V2 not evaluable ⇒ run VOID (procedural re-registration with a revised dressing, per LOCK-STD).

**Emitted per arm:** centroid time series, entry/exit fits, packet energy retention, latency profile along the ray path (for §4's θ_γ0), max static-drift, all validity-gate inputs.

## 3 · Validity gates (vacuity firewalls — any failure ⇒ run VOID, not an outcome)

- **V1 — the well exists:** max voxel latency within r ≤ 6 of the mass > 0.01 after equilibration.
- **V2 — the well acts:** the particle arm deflects: |θ_p| > 10 × (floor angle from C0). If the Newtonian sector itself shows nothing, the source is broken and NO statement about the optical channel is licensed.
- **V3 — packet integrity:** ≥ 50% of the packet's difference-field energy remains in the moving window at exit (else dispersion/absorption has destroyed the centroid observable).
- **V4 — staticity:** the baseline drift contribution to δy over the transit is < ½ of the C0 floor.

## 4 · Frozen reference scale

θ_γ0 ≡ the g₀₀-geodesic (optical-metric) deflection computed from the RUN'S OWN measured latency profile by the frozen formula θ_γ0 = ∮ |∇⊥ ln f| dl along the straight ray at impact parameter b, with f the proper-time factor the engine derives from latency (the map of record in `phase_forces.cpp` / SPEC_FTD_LAGRANGIAN §4.3). This is the "half-GR" yardstick: a metric with this g₀₀ and flat spatial part deflects light by θ_γ0; full GR (γ_PPN = 1) deflects by 2·θ_γ0. Computed post-run by script from the emitted latency profile; the formula is frozen here.

## 5 · Pre-declared outcomes

**Floor** ≡ max(3 × |δy|_C0, 3 × |δz|_W, V4 drift bound), expressed as an angle.

| Outcome | Condition (per W arm, both b) | Consequence |
|---|---|---|
| **N — no channel** | \|θ_w\| ≤ Floor **and** Floor < 0.05·θ_γ0 (the floor is small enough that a g₀₀-optical signal would have been seen) | **[MEASURED STRUCTURAL NULL]**: the substrate's wave sector does not couple to the gravitational field. Light does not bend — not at the GR value, not at half. Gate-2 verdict: the optical/spatial-metric sector is **unreachable within the current dynamics**; g_rr requires either a new derivation introducing a wave–latency coupling from existing axioms, or a priced P6C adoption. The "unifies gravity" claim is BOUNDED to the proper-time + force sector. A first-class boundary result (goal face 2). |
| **G-half** | θ_w within ±35% of θ_γ0 (both b, same sign, toward the mass) | An unexpected g₀₀-optical channel EXISTS. Mechanism hunt mandatory before any tag ([OPEN]); the γ_PPN = 0 signature (half-GR) would be a falsifiable substrate prediction *against* observed lensing — a live falsifier, booked as such. |
| **G-full** | θ_w within ±35% of 2·θ_γ0 (both b, same sign, toward the mass) | GR-like optical response WITHOUT an explicit spatial metric — extraordinary; requires independent replication, mechanism isolation, and adversarial review before any claim beyond [MEASURED, UNEXPLAINED]. |
| **Indeterminate** | Anything else (V-gate pass but bands missed, b-inconsistency, floor too large relative to θ_γ0) | Characterize; re-register v2 with the sharpened instrument. Never laundered into N. |

**Sign / absorption disambiguation:** a |θ_w| > Floor signal AWAY from the mass is a candidate selective-damping absorption artifact, not gravity. Adjudication: if the D-arm (damping off) removes the signal, the W-arm away-signal is booked as the absorption artifact and the gravitational verdict is taken from the D-arm against the same bands (with the D-arm's own C0-style floor); if the D-arm retains it, Indeterminate.

## 6 · Anti-gaming

- The arms, amplitudes, impact parameters, window, and gates above are fixed; no post-hoc parameter search.
- A null with failed gates is VOID, not Outcome N.
- Outcome N may not be quoted as "FTD predicts no light bending" — it is a statement about the CURRENT substrate dynamics, and its booked consequence is the boundary/pricing path, not a physics prediction against observation.
- The code-derived expectation in §1 does not soften §5: if the measurement contradicts the code reading, the measurement wins and the mechanism is found.

---

*Registered 2026-07-18, before the instrument's first execution. Author: session 8294fddb, following LOCK-STD v1. Companion lock: `preregister-graviton-substrate-mode-v2-1` (amended-engine TT re-run), registered the same day.*
