# AUDIT — The Functional Census

**Status:** `[AUDIT / SYNTHESIS]` — enumerates and classifies; introduces no
claim, moves no tag.
**Date:** 2026-08-15
**Programme:** Universality Programme P3
(`../../10_eft_program/temporal_interior_programme/SCOPE_UNIVERSALITY_PROGRAMME_v1.md`, DRAFT).
**Question:** how many distinct energy functionals does the production
engine actually run — and therefore where must universality (universal
dilation, frame hiding) break, per the programme's thesis?

The measured background: carriers whose kinetics and binding came from two
energy categories shattered dilation universality (p ∈ [−2.70, −0.94]);
one-functional carriers restored it (p = −1.000 ± 0.002, FTD-0814;
two-body signal FTD-1009). The census below is the engine-side ledger of
that distinction. Sources of record: `MAP_LAGRANGIAN_TO_ENGINE.md` §1 (the
six-term decomposition), `engine/include/ftd/lagrangian.h`,
`engine/include/ftd/render_bridge_diagnostics.h` (EnergyAudit, lines
84–137), `engine/src/energy_ledger_compute.cpp` (lines 17–32: what the
ledger actually sums), `engine/include/ftd/term_toggles.h`,
`engine/SPEC_ENGINE.md` §4 (tick cycle) and line 251 (genesis energetics).
⚠ Naming hazard inherited from `AUDIT_TELEMETRY_ORGANIZATION.md` §4:
`Diagnostics.total_energy` (a Born–Infeld flux proxy) and
`EnergyAudit.total_energy` (field+wave+particle KE) are different
quantities — never compare them numerically.

## 1. The census

**Class CF (common-functional): the wave island.** Terms that are, or
extend, the single quadratic functional of the flux sector — the sector
for which the free-sector Lorentz results (FTD-0815: unique O_h stencil,
dim-4 LV vanishing by symmetry, anisotropy (ka)⁴/3240) actually hold.

| # | term | definition | source | toggle | note |
|---|------|-----------|--------|--------|------|
| CF1 | field kinetic | ½\|Δ_t J\|² | `lagrangian.h:77-79` | `wave_propagation` | the p-half of the wave functional |
| CF2 | field gradient | −½c²Σ_links w_l\|ΔJ\|², w_face=1/3, w_edge=1/6 | `field_gradient_term()`, `lagrangian.h` | `wave_propagation` | the unique O_h-symmetric 18-point pair (FTD-0815 route A) |
| CF3 | Klein–Gordon mass | −ω₀²J term | `term_toggles.h:103` (`de_broglie_clock`) | `de_broglie_clock` | isotropic mass term; hyperbolic dispersion preserved — CF-compatible extension |

**Class SC (second-category): everything else.** Each line names the
universality-breaking channel it opens — the observable that would deviate
if a composite's energy has support on the term.

| # | term | definition | source | toggle | breaking channel |
|---|------|-----------|--------|--------|------------------|
| SC1 | latency / gravity sector | elliptic Poisson solve, −(1/8πG)\|∇𝓛\|² | `render_bridge.cpp:280` (`solve_latency_poisson`) | `latency_field`, `gravity`, `field_energy_gravity` | **instantaneous action in exactly one frame** — the maximal channel; gravitational influence propagates at infinite speed in the substrate frame only |
| SC2 | strong potential | Σ_{i<j} U_ij static pair law | `render_bridge_diagnostics.h:130-136` (FTD-0406) | `strong_stress_energy`, `strong_force` | same elliptic/instantaneous class as SC1 |
| SC3 | Rayleigh dissipation | (α/2)\|v_wave\|² | `lagrangian.h:64-66` | `damping`, `selective_damping` | **the bath has a rest frame** — a moving oscillator sees a direction-dependent quality factor; Q-anisotropy is an internal substrate-frame detector |
| SC4 | Langevin thermostat | OU noise on wave_vel | `render_bridge.cpp` (thermostat), `term_toggles.h:70` | `langevin` | bath frame, as SC3, plus a preferred-frame fluctuation spectrum |
| SC5 | genesis / evaporation thresholds | 7-site E_local vs K_GENESIS/K_MANIFEST; dτ/dt = √(1−B) | `SPEC_ENGINE.md:251` | `genesis`, `evaporation` | **amplitude-gated state change**: a boosted flux profile presents different \|J\| maxima, so manifestation fires frame-dependently; composite clocks near threshold lose universal dilation (the carrier programme's recorded drain) |
| SC6 | pair production / annihilation burst | threshold consumption/burst of local wave/flux energy | `SPEC_ENGINE.md` §4.1 (`pair_production_cpu`, `phase_movement`) | `pair_production` | threshold class, as SC5 |
| SC7 | state–flux coupling (electric) | −g_c s(∇_L·J) | `lagrangian.h:36-38` | `coupling` | couples to the s-sector, which has **no kinetic functional at all** — see SC9 |
| SC8 | velocity coupling (magnetic) | −g_c s(v·J) | `lagrangian.h:43-48` | `lorentz_force` | as SC7; v is the lattice-frame hop velocity |
| SC9 | s-sector movement rules | discrete hop dynamics of manifested states | `phase_movement` (tick phase 7) | `forces` et al. | the matter-state sector is **rule-dynamics, not functional-dynamics**: its "kinetics" are lattice-frame update rules with no action, hence no inherited cone — the deepest structural gap the census exposes |
| SC10 | particle kinetic energy | Σ(γ₀−1)·E_REST, E_REST = K_B/3 | `render_bridge_diagnostics.h:89`, `SPEC_ENGINE.md:614` | `cluster_inertia` | the relativistic *form* is **imposed by hand**, not emergent from a functional — an import wearing the answer's clothes; calibration-conditional (M_INERTIAL = K_B, FTD-0402 role conflation) |
| SC11 | Gauss constraint | −λ_G(∇_L·J − ρ)², λ_G = 100 | `lagrangian.h:55-60` | `gauss_projection` | [SELECTION] (FTD-0421/0426); as a stiff constraint it is CF-adjacent (constraint surfaces preserve the wave cone), but the projection is solved instantaneously per tick — elliptic-class in implementation |
| SC12 | Born–Infeld core | −K_B√((f²−v²)/f) | `lagrangian.h:29-31`, `phase_forces.cpp:225-253` | (diagnostic-only per the engine-mass facts of record) | nonlinear in \|J\|; if ever promoted beyond diagnostic, it deforms the cone amplitude-dependently |
| SC13 | compact matter law | A_ab·V(q_ab), σ ternary per FTD-1007 | `DERIV_MINIMAL_MANY_BODY_MATTER_NETWORK_v1.md` §2 | **not implemented** | paper-only; a second category by construction (contact potential with its own scale ε) — the FTD-0812 Galilean result is its recorded signature |

**The count.** With every toggle off except the wave island, the engine
runs **one** functional (CF1+CF2) — and that island is exactly where the
recorded Lorentz results live. The logic-first default set adds the
coupling, Gauss, genesis, movement, and damping machinery: **at least five
second-category channels active by default**. The full toggle surface
exposes ~13 distinct energy categories. By the programme's thesis, the
engine is not "approximately Lorentzian with corrections" — it is exactly
as Lorentzian as its active functional set is unified, sector by sector.

## 2. Where universality must break — the target register

Ranked by expected severity of the breaking channel; each line is a
standing measurement target for the programme (P1-style event-map
campaigns apply directly):

1. **SC1/SC2 (instantaneous elliptic sectors).** Gravitational/strong
   influences propagate at infinite speed in one frame. Sharpest possible
   internal frame detector; any composite bound by these sectors should
   show *order-one* dilation non-universality. Measurement: dilation
   exponent of a latency-bound vs wave-bound composite.
2. **SC3/SC4 (bath frames).** Q-factor anisotropy of a moving damped
   oscillator. Clean, small-amplitude, engine-testable.
3. **SC5/SC6 (thresholds).** Amplitude-gated universality loss near
   K_GENESIS; predicted signature: dilation exponent drifting with
   proximity to threshold (the FTD-0794/0804 finite-amplitude language).
4. **SC9 (rule-dynamics matter).** No action ⇒ no cone inheritance ⇒ the
   s-sector's effective "clock" is the raw tick. Any composite whose
   period depends on state hops cannot dilate correctly. This is the
   census's restatement of why the carrier programme kept failing —
   and the structural reason the φ⁴ surrogate (all-functional) succeeds
   where engine matter does not.
5. **SC10 (imposed relativistic form).** Not a dynamical break but an
   accounting one: where dilation *appears* correct because γ₀ was typed
   in, the appearance is an import; flag any claimed engine Lorentz result
   that routes through particle_ke.

## 3. What the census licenses

Nothing beyond bookkeeping. It does not claim the wave island *is*
Lorentzian (that claim lives at FTD-0815's tags, with its (ka)⁴ remainder
and open common-cone item); it does not claim the SC channels *do* break
universality at the stated signatures (each is a measurement target, not a
result). Its single synthesis-grade output is the count: **one common
functional, ~a dozen second categories, each priced with a named
observable** — the Universality Programme's standing work list.
