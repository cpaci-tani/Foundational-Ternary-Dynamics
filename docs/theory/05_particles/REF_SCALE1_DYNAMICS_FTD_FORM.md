# REF — Scale-1 Particle-Engine Dynamics, in FTD-Native Form

**Tag:** `[REFERENCE]`
**Date:** 2026-06-15
**Status:** `[REFERENCE]` — a code-grounded cross-walk of every dynamic the web dashboard's **Scale 1 (Particle Engine)** computes, with each formula re-expressed in FTD's own constants where one genuinely exists.
**Scope:** `engine/web/js/` Scale-1 modules (the particle engine) only. Scale 0 (lattice/substrate), Scale 2/3 (atoms/molecules), and the C++ engine are out of scope.

> **Epistemic banner (read first).** Re-expressing a textbook formula in FTD constants (`α → G_C²`, `m_e → K_B`, `c → 1/√3`, …) is **notation, not derivation.** The Scale-1 *dynamical laws* are imported physics (Coulomb, Newton, Velocity-Verlet, Fermi, Bohr/Dirac, Klein–Nishina, Gamow); only the **constants plugged into them** are FTD quantities. The epistemic tags below (`[PARAMETRIC]`, `[IMPOSED]`, `[SELECTION]`, `[DERIVED]`) describe the *law*, and are **unchanged** by the FTD-form rewrite. Constants with no FTD derivation (ħc, V_ud, g_A, f_π, f_n, branching ratios, PDG quark/hadron/boson masses, amu) are left as external inputs — this doc does **not** manufacture FTD forms for them (per the project's epistemic discipline against substitution identities). Conflict precedence: LEDGER > this doc.

---

## §0 · One-line summary

After substitution, the **only** FTD-native quantities driving Scale 1 are: `G_C²` (α), `K_B` (m_e), `1/√3` (c), the framework integers `{N_c=3, N_base=4, b₃=7, N_eff=13}` (mass ratios), `1/(b₃+N_c)²` (G_N, `[IMPOSED]`; physical identification falsified per FTD-0131), and `sin²θ_W = 3/13`. Everything else is an external input, and every dynamical *law* is imported textbook physics.

## §1 · FTD constant substitution key (engine-defined)

| Textbook symbol | FTD-native form | Engine source | Epistemic status |
|---|---|---|---|
| α (fine structure) | **G_C²** (`ALPHA_EFT = G_C·G_C`) = **1/x₊**, x₊ = 8G\*² + 4G\*^{3/2}√(4G\*−1) ≈ 137.036 (master-quadratic root) | `constants.js:71,77` | physical ID `[STRONGLY MOTIVATED CONJECTURE]` (FTD-0013) |
| electron mass m_e | **K_B** = 0.511 (mass anchor); dimensional origin m_P·√(2π)·(16/3)·α¹¹ | `constants.js` | `[DERIVED]` anchor (FTD-0015) |
| speed of light c | **1/√3** (`C_SPEED`, CFL stability on the cubic lattice) | `constants.js:117` | `[DERIVED]` |
| Newton G_N | **1/(b₃+N_c)² = 1/100 = 0.01** | `constants.js:80–87` | `[IMPOSED]`; physical ID **falsified** (FTD-0131) |
| Coulomb prefactor | **G_C²/(4π)** (`COULOMB_K_FORCE`) — used for both force and potential energy (the PE is the exact integral of the force; force-consistent, verified by the 2026-06-15 fidelity audit) | `mock-particle-engine.js:133,146` | — |
| m_μ | **K_B·[3·b₃(b₃+N_c) − N_c] = 207·K_B** | `constants.js:125` | `[DERIVED]` (0.11%) |
| m_τ | **K_B·[(N_eff+N_base)·207 − 2·N_c·b₃] = 3477·K_B** | `constants.js:126` | `[DERIVED]` (0.007%) |
| m_p | **K_B·[N_eff/α + N_base·N_eff + N_c] = K_B·(N_eff·x₊ + 55) ≈ 1836.47·K_B** | `constants.js:130` | `[STRONGLY MOTIVATED CONJECTURE]` (FTD-0016) |
| pair / genesis threshold | **2·K_B** (1.022 MeV); genesis scale K_GENESIS = **N_c·K_B** | `cross-sections.js:94`; `constants.js` | — |
| weak mixing | **sin²θ_W = 3/13** | `constants.js:322` | `[PARAMETRIC]` (FTD-0018) |
| velocity damping γ | **G_C²** (= α; convention, default OFF) | `constants.js:121` | `[IMPOSED]` |
| framework integers | N_c=3, N_base=4, b₃=7, N_eff=13 | `constants.js` | `[THEOREM]`/`[SELECTION]` |

## §2 · Core live N-body dynamics (the actual simulation)

The default Scale-1 backend is the JS N-body integrator in `bridge/mock-particle-engine.js` (a native WASM integrator is used when present).

| Process | FTD-native law | Status | Source |
|---|---|---|---|
| Coulomb force | `F = −(G_C²/4π)·q₁q₂/r²` (softened `r²+soft²`) | `[PARAMETRIC]` | `mock-particle-engine.js:133,146` |
| Gravity | `F = (1/(b₃+N_c)²)·m₁m₂/r² = (1/100)·m₁m₂/r²` | `[IMPOSED]` | `mock-particle-engine.js:147` |
| Integrator | Velocity-Verlet (half-kick / drift / half-kick) | `[IMPOSED]` (numerics) | `mock-particle-engine.js:161–212` |
| Light-speed cap | clamp `\|v\| ≤ 1/√3` | `[DERIVED]` | `mock-particle-engine.js:223` |
| Velocity damping | `v *= 1 − G_C²·dt` (default OFF) | `[IMPOSED]` | `mock-particle-engine.js:215` |
| Pair annihilation | geometric contact (opposite charge, radii touch) → remove | `[SELECTION]` (not a QED cross-section) | `mock-particle-engine.js:233–252` |
| Boundary reflection | reflect mobile particles into sphere r=35 | `[IMPOSED]` | `mock-particle-engine.js:191` |
| Locked particles | infinite-mass nucleus surrogate (no integration) | `[IMPOSED]` | `mock-particle-engine.js:93–103` |

> **Scope note.** The JS `_peComputeForces` computes **only Coulomb + gravity**. The UI toggles `lorentz, strong, exchange, magnetic_dipole, spin_orbit, radiation, relativistic` are forwarded to a possible native `peSetToggle` but have **no JS dynamics behind them** (`wasm-bridge.js:944–958`). The 3-regime strong-force constants (`STRONG_*`, `constants.js:355–367`) are consumed by the **C++** engine (`render_bridge.cpp::phase_forces`), not by any JS Scale-1 path.

## §3 · Scenario seeding & black-hole / Hawking demo

| Process | FTD-native law | Status | Source |
|---|---|---|---|
| Circular-orbit velocity (1 center) | `v = √(G_C²·Q·r² / (4π·m·(r²+soft²)^{3/2}))` | `[PARAMETRIC]` | `controller.js:466` |
| Two-body mutual orbit | same family with `G_C²` (sep = 2r) | `[PARAMETRIC]` | `scenarios.js:166…` |
| BH orbital velocity | `v = √((1/100)·M_BH·r² / (r²+soft²)^{3/2})` | `[PARAMETRIC]` | `scenarios.js:386` |
| 25 `pe-*` scenarios | atoms, exotic atoms, leptonia, hadron pairs, scatterings, 3-body, micro-BH | `[SELECTION]` (pedagogical ICs) | `scenarios.js:138–432` |
| BH accretion | Newtonian gravity, M_BH = 5000 MeV; inspiral emerges from the `1/√3` cap | `[SELECTION]`/`[EMERGENT]` | `scenarios.js:380–423` |
| Hawking-analogue emission | every 300 ticks spawn test mass **K_B**: e⁻ (out, `0.6·(1/√3)`) + e⁺ (in) at r=3.5 | `[SELECTION]` toy — no temperature/spectrum/greybody | `controller.js:69,280–298` |

## §4 · Cross-sections (`cross-sections.js`) — analysis cards, not in the integrator

All `[PARAMETRIC]` (textbook QED with FTD α, K_B; ħc external). Engine *displays* with PDG m_e; the FTD-native form below uses K_B.

| Process | FTD-native law |
|---|---|
| Classical electron radius | `r_e = G_C²·ħc/K_B` |
| Thomson | `σ_T = (8π/3)·r_e²` |
| Rutherford | `dσ/dΩ = (Z₁Z₂·G_C²·ħc/4E)² / sin⁴(θ/2)` |
| Mott | Rutherford·`(1 − β²sin²(θ/2))` |
| Pair-production threshold | `E = 2·K_B` |
| Bethe–Heitler | `σ = (7/9)·G_C²·r_e²·Z²·(28/3·ln(2E/K_B) − 218/27)` |
| Klein–Nishina | `dσ/dΩ = ½r_e²·P²(P + 1/P − 1 + cos²θ)`, `x = E/K_B` |

(Source: `cross-sections.js:16–153`.)

## §5 · Masses & decays (`particle-catalog.js`, `decay-rates.js`)

| Process | FTD-native law | Status / external input |
|---|---|---|
| Lepton masses | `m_μ = 207·K_B`, `m_τ = 3477·K_B` (framework integers) | `[DERIVED]` |
| Proton mass | `m_p = K_B·(N_eff·x₊ + N_base·N_eff + N_c)` | `[SMC]` (FTD-0016) |
| Muon lifetime | `τ = 192π³ħ / (G_F²·(207·K_B)⁵)`, G_F from FTD α & sin²θ_W=3/13 | `[PARAMETRIC]` |
| Tau lifetime | `τ_μ·(207/3477)⁵·BR` | `[PARAMETRIC]`; BR external |
| Neutron β-decay | `τ = 2π³ħ / (G_F²·K_B⁵·\|V_ud\|²·f_n·(1+3g_A²))` | `[PARAMETRIC]`; V_ud, g_A, f_n external |
| Charged-pion decay | `Γ = G_F²·f_π²·(207·K_B)²·m_π(1−(207·K_B)²/m_π²)²/8π` | `[PARAMETRIC]`; f_π, m_π external |
| α-decay (Gamow) | `T = exp(−2π·Z·2·G_C²·√(m_red/2Q))` | `[PARAMETRIC]`; amu external |
| 80+ SM catalog | per-entry mass/charge/spin/formula | leptons `[DERIVED]`; quarks/Higgs/proton `[SELECTION]`; W/Z/hadrons `[PARAMETRIC PDG]`; γ/gluon `[AXIOM]` |
| Decay channels / BR | tabulated SM channels (display strings) | `[SELECTION]` — particles never actually decay in-sim |

(Source: `particle-catalog.js:38–595`, `decay-rates.js:21–202`.)

## §6 · Spectroscopy (`spectroscopy.js`) — display cards

All `[PARAMETRIC]` (Bohr/Dirac with FTD α; ħc external; engine uses PDG m_e — FTD form uses K_B).

| Process | FTD-native law |
|---|---|
| Hydrogenic energy levels | `E_n = −K_B·Z²·G_C⁴ / (2n²)` |
| Fine-structure correction | `ΔE = E_n·G_C⁴/n·(1/(j+½) − 3/4n)` |
| Bohr radius | `a₀ = ħc/(K_B·G_C²)` |
| Compton wavelength | `λ_C = 2π·ħc/K_B` |
| Spectral series | `λ = 2π·ħc/\|E_i − E_f\|` (Lyman…Pfund) |
| Rydberg | `R_∞ = K_B·G_C⁴/2` |

(Source: `spectroscopy.js:18–87`.)

## §7 · Field overlays & telemetry

**Fields** (`fields.js`, 25×25 grid, visualization): Coulomb `φ = Σ(G_C²/4π)·q/r`; gravity `φ = −Σ(1/100)·m/r`; RK4 E-field streamlines. `[PARAMETRIC]`/`[IMPOSED]`.

**Telemetry** (`pe-telemetry.js`, `mock-particle-engine.js`) — measured each tick (conservation laws are *measured*, not imposed, validating the integrator): KE `= Σ½mv²`; Coulomb PE `= Σ (G_C²/4π)·q_iq_j/r` (force-consistent — the exact integral of the Coulomb force, *not* a separate "bare G_C²" convention; confirmed by the 2026-06-15 fidelity audit); gravity PE `= −Σ(1/100)·m_im_j/r`; total energy + drift%; linear & angular momentum; virial `2K/|U|`; temperature `T = (2/3)·KE/N`; RMS velocity / CoM / radius; per-particle force table; full two-body Kepler analysis (μ, e, a, period, vis-viva); radial-velocity phase-space plot. (Source: `pe-telemetry.js:355–611`.)

## §8 · External inputs with no FTD form (left as-is)

ħc (= 197.327 MeV·fm, unit bridge), V_ud, g_A, f_n, f_π, leptonic branching ratios (e.g. 0.1785), PDG masses for quarks / hadrons / W / Z / neutron, amu. FTD provides no derivation for these; they are calibration/PDG inputs.

## §9 · What Scale 1 does **not** compute (so the table isn't over-read)

- No relativistic equations of motion (only the kinematic `|v| ≤ 1/√3` clamp); no Lorentz/magnetic force, spin-orbit, or radiation reaction in JS.
- **No actual decay events** — `decay-rates.js` is display tables; sim particles only annihilate by contact.
- **No actual scattering events** — `cross-sections.js` is analysis cards; scattering scenarios just launch particles under the Coulomb force.
- Hawking emission has no temperature/spectrum/greybody — fixed 300-tick cadence.
- Genesis / cluster-mass / lattice-wave dynamics are **Scale 0**, not Scale 1.

## §10 · Source modules & cross-references

**Scale-1 modules** (under `engine/web/js/`): `scales/scale1/{controller,scenarios,pe-cloud-expander}.js`, `bridge/{mock-particle-engine,wasm-bridge}.js`, `cross-sections.js`, `decay-rates.js`, `spectroscopy.js`, `particle-catalog.js`, `pe-telemetry.js`, `fields.js`, `constants.js`.

**Canonical FTD references:** `scripts/constants.py` + `engine/include/ftd/ontic.h` + `engine/web/js/constants.js` (the canonical constant triple); [`../07_assessment/core_ledgers/LEDGER.md`](../07_assessment/core_ledgers/LEDGER.md) (per-claim status: FTD-0013 α, FTD-0015 m_e, FTD-0016 m_p, FTD-0018 sin²θ_W, FTD-0131 G_N falsification); [`../07_assessment/AUDIT_ATOMIC_DYNAMICS_STATUS.md`](../07_assessment/AUDIT_ATOMIC_DYNAMICS_STATUS.md) (why hydrogen/Lamb-shift are `[PARAMETRIC]`, not substrate derivations); [`REF_PHYSICS_REFERENCE.md`](REF_PHYSICS_REFERENCE.md) (integer-encoding catalog).
