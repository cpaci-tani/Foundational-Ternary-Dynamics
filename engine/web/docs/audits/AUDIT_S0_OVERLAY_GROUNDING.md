# Scale 0 Overlay Epistemic-Grounding & Engine-Truth Audit

Status: `[AUDIT]` — per-overlay grounding + data-source truth for every Scale 0 lattice overlay
Version: 1.0 (2026-06-03)
Scope: Scale 0 (Lattice / Substrate) viewport overlays — 34 toggles across 7 columns
Companion to: [`SPEC_OVERLAY_SEMANTICS.md`](../historical/SPEC_OVERLAY_SEMANTICS.md) (visual-encoding audit; older + partial)
and [`SPEC_S0_QUANTUM_OVERLAYS.md`](SPEC_S0_QUANTUM_OVERLAYS.md) (quantum-tier spec).

---

## 1. What this audit is

Two different questions can be asked of an overlay:

- **Visual semantics** — *does the shape/motion/color teach what the quantity is?* That is the job of
  `../historical/SPEC_OVERLAY_SEMANTICS.md` (v1.0, 2026-04-16). It is now partial: it predates ~10 of the current
  overlays and the 2026-05-27 weak-force relabel.
- **Epistemic grounding + engine-truth** (this doc) — *does the quantity genuinely make sense at the
  FTD substrate level, what is its epistemic status, and does it actually show real engine data?*

This audit answers the second question. It was triggered by a request to vet whether every overlay
"makes sense to include." The framework's epistemic tags ([AXIOM]/[THEOREM]/[DERIVED]/[SELECTION]/
[PARAMETRIC]/[PROXY]) are used per `CLAUDE.md` and the LEDGER.

The FTD substrate is two-layer: a continuous **flux field `J ∈ ℝ³`** (dispositional) and a discrete
**ternary state `s ∈ {−1,0,+1}`** (manifestation). Everything an overlay shows is either one of these
two primitives, a vector-calculus operator on `J`, or a higher-scale concept derived/borrowed on top.

---

## 2. Headline findings

1. **Strong prior hygiene.** Many tooltips already carry honest `[PROXY]` tags with audit references
   (P1-17 weak-force, P1-18 chirality, P2-9 Laplacian). This audit builds on that record.

2. **★ Seven overlays render NOTHING whenever the compiled engine owns the scenario.** Scale 0 has two
   physics owners. For `flux-*` and `s0-*` scenarios, `shouldUseFluxMock()`
   (`runtime/scenario-loader.js:119`) spins up a parallel JS **MockBridge** (`state.fluxMock`) that
   owns the physics — and it implements the full sampler set, so every overlay works there (this is
   the default `flux-pulse` experience). For the **`empty`, `light-*` (4), and `quantum-*` (9)**
   scenarios, and **always on the native-GPU / WebSocket path**, the compiled **WASM/native engine**
   owns the physics instead. That bridge (`js/bridge/wasm-bridge.js`) does **not** expose
   `getKretschmannSampled`, `getLatencySampled`, `getFisherSampled`, `getCoherenceSampled`,
   `getScale0LatticeBuffer`, or `getScale0DerivedOverlayData`. The capability layer
   (`js/bridge/capabilities/scale0.js`) falls back to an empty sample, so the overlay computes from
   empty → renders nothing. Affected: **Curvature K, Horizon, Fisher F, Coherence C, DM Halo, Genesis,
   Damping.** A user in the Quantum Lab or Light & EM scenario who enables any of these sees nothing,
   with **no signal** that the overlay is unsupported on that owner.

   **Low-risk fix path:** the WASM bridge *does* expose `getFluxVolume()` (the full `|J|` field) plus
   native `getVorticitySampled`/`getHelicitySampled`. So latency/Kretschmann/Fisher are JS-computable
   from `|J|`, Coherence from helicity ÷ (`|J|`·vorticity), and DM-Halo/Genesis/Damping from
   `getFluxVolume` + `getParticleData`. All seven can be wired engine-true **in JS, with no C++ rebuild**
   (implement the missing samplers on `WasmBridge`, reusing the MockBridge sampler math). Only the new
   **State field `s`** overlay needs a C++ binding — the ternary lattice buffer is genuinely unexposed.

3. **`∇×J` is mislabeled as the weak force.** It occupies the FORCES "weak" slot (`showForceWeak`,
   `field-swatch-weak`). It is the curl of `J`, **not** the SM weak interaction. The tooltip already
   disclaims this (P1-17), but the column placement and swatch still imply a fourth fundamental force.

4. **Two intentional duplicate-buffer pairs.** **Charge ρ ≡ ∇·J** (both read `sampled.divergence`,
   rendered as a signed rubber-sheet vs. a point cloud) and **Light ≡ Poynting** (both render `|S|`,
   bloom vs. arrows). Defensible as distinct render styles (the panel header says so) but
   un-cross-referenced, so they read as separate physics.

5. **Genuine substrate gaps (new-overlay candidates).** The **ternary state `s` itself is invisible**
   (the literal FTD ontology); **latency** `ℒ` is computed by the engine but has no toggle; there is no
   **Moore-decomposition** structural overlay; there is no **Gauss-residual** diagnostic for the
   documented non-variational projection leak.

---

## 3. Per-overlay grounding table

`✓` works on real WASM engine · `✗ MOCK` renders nothing on WASM (mock-only) · `~` works via JS proxy.
Data source: where the field values come from (WASM C++ binding, JS compute, or MockBridge).

### VOLUME
| Overlay | Quantity | Data source | WASM | Epistemic status | Verdict |
|---|---|---|---|---|---|
| Flux Volume | `\|J\|` point cloud | `getFluxVolume` | ✓ | [AXIOM] raw flux | KEEP |
| Flux Slice (xy/xz/yz) | 2D `J` planes | `getFluxSlice` | ✓ | [AXIOM] | KEEP |
| Flux Lines | `J` streamlines (RK4) | `getFluxVectorSampled` | ✓ | [AXIOM] | KEEP |
| ∇·J | divergence of `J` | `getDivJSampled` | ✓ | [THEOREM] operator; charge id [SELECTION] | KEEP — cross-label w/ Charge ρ |

### FIELDS
| Overlay | Quantity | Data source | WASM | Epistemic status | Verdict |
|---|---|---|---|---|---|
| E Field | `E = −∂ₜJ` streamlines | `getEFieldSampled` | ✓ | [DERIVED] from lattice Lagrangian | KEEP |
| B Field | `B = ∇×J` | `getBFieldSampled` | ✓ | [THEOREM] operator; Maxwell id [SELECTION] | KEEP |
| Poynting S | `S = E×B` | `getPoyntingSampled` | ✓ | [DERIVED] | KEEP |
| Light | `\|S\|` bloom | passthrough of `poynting` | ✓ | [DERIVED] — render-variant of Poynting | KEEP — cross-label |

### FORCES
| Overlay | Quantity | Data source | WASM | Epistemic status | Verdict |
|---|---|---|---|---|---|
| EM | Coulomb + Lorentz | `getEMForceField` | ✓ | α-coeff [PARAMETRIC]; Phase-G Coulomb [THEOREM] | KEEP |
| Gravity | `G·∇\|J\|` | `getGravityFieldSampled` | ✓ | [SELECTION] — mechanism not derived (FTD-0131) | KEEP — soften tooltip |
| Strong | SU(3) / color | `getStrongForceField` | ✓ | confinement [THEOREM]; SU(3) id [SELECTION] (needs N_c) | KEEP — soften tooltip |
| ∇×J pseudovector ("weak") | `∇×J` vector | `getCurlJSampled` | ✓ | [PROXY] — **NOT** SM weak force | FIX — relabel + recolumn |

### QUANTUM (all already `[PROXY]`-tagged; computed in JS from base samplers → work on WASM)
| Overlay | Quantity | Data source | WASM | Epistemic status | Verdict |
|---|---|---|---|---|---|
| \|ψ\|² | `\|J\|²` | JS from `fluxVector` | ✓ | [PROXY] / [DERIVED] this tier | KEEP |
| Phase φ | `arg(J_L+iJ_R)` | JS (needs Dual J) | ✓ (flat w/o dual) | [PROXY]; dual decomposition [SELECTION] | KEEP |
| ℒ(x) | `½\|J\|²−½(∇·J)²` | JS from `fluxVector`+`divergence` | ✓ | [PROXY] — not true `\|∇J\|²` | KEEP — upgrade path: Jacobian |
| Entropy s | `4p(1−p)`, `p=\|J\|/\|J\|ₘₐₓ` | JS from `fluxVector` | ✓ | [PROXY] — Gini, not Shannon | KEEP — upgrade path: state field → Shannon |

### TOPOLOGY
| Overlay | Quantity | Data source | WASM | Epistemic status | Verdict |
|---|---|---|---|---|---|
| Φ potential | `−\|J\|²` proxy | JS (`getGravPotentialSamples` absent → proxy) | ~ proxy | [PROXY] — not Poisson-solved | KEEP — upgrade path |
| EM energy u | `½(\|E\|²+\|B\|²)` | JS from `eField`+`bField` | ✓ | [DERIVED] | KEEP |
| Charge ρ | `∇·J` | `getDivJSampled` | ✓ | [THEOREM] operator; charge id [SELECTION] | KEEP — ≡ ∇·J; cross-label |
| Vorticity ω | `\|∇×J\|` | `getVorticitySampled` | ✓ | [THEOREM] operator | KEEP |
| Helicity h | `J·(∇×J)` | `getHelicitySampled` | ✓ | [DERIVED] | KEEP |
| Curvature K | `(∇²L)²` (latency proxy) | `getKretschmannSampled` | ✗ MOCK | [PROXY] | WIRE (C++ binding) |

### STRESS-ENERGY
| Overlay | Quantity | Data source | WASM | Epistemic status | Verdict |
|---|---|---|---|---|---|
| P₊ electric | `½\|E\|²` | JS from `eField` | ✓ | [DERIVED] | KEEP |
| P₋ magnetic | `½\|B\|²` | JS from `bField` | ✓ | [DERIVED] | KEEP |
| Kinetic K | `½\|v\|²` at particles | `getParticleData` velocities | ✓ if velocities | [DERIVED] — particle-anchored | KEEP — note anchoring |
| Fisher F | `\|∇ρ\|²/ρ`, `ρ=\|J\|²` | `getFisherSampled` | ✗ MOCK | [DERIVED] info-geometry | WIRE (C++ binding) |

### PHENOMENA
| Overlay | Quantity | Data source | WASM | Epistemic status | Verdict |
|---|---|---|---|---|---|
| Dual J | `(1±δ)/2·J` | JS amplitude split | ✓ | [PROXY] — not Helmholtz L/R | KEEP (disclaimed) |
| Chirality | `\|J\|·δ` | JS | ✓ | [PROXY] | KEEP (disclaimed) |
| DM Halo | sub-threshold flux | `getScale0DerivedOverlayData` (mock) | ✗ MOCK | [PROXY] pedagogical | WIRE (JS from flux+particles) |
| Genesis | `\|J\|=K_GENESIS` iso | `getScale0DerivedOverlayData` (mock) | ✗ MOCK | [PARAMETRIC] | WIRE (JS from flux) |
| Damping | 1-hop zones | `getScale0DerivedOverlayData` (mock) | ✗ MOCK | [SELECTION] | WIRE (JS from particles) |
| Confinement | pair-proximity glyphs | JS particle pairs | ✓ | [PROXY] — distance heuristic, NOT σ=0.209 | KEEP (disclaimed) |
| Horizon | `L ≥ 0.95` | `getLatencySampled` | ✗ MOCK | [PROXY] | WIRE (C++ binding) |
| Coherence C | `J·(∇×J)/(\|J\|\|∇×J\|)` | `getCoherenceSampled` | ✗ MOCK | [DERIVED] | WIRE — **JS-only** (helicity/vorticity/flux on WASM) |

---

## 4. Disposition summary

| Action | Overlays |
|---|---|
| **KEEP as-is** (genuine substrate or clearly-labeled proxy) | Flux Volume, Flux Slice, Flux Lines, ∇·J, E, B, Poynting, EM, \|ψ\|², Phase, ℒ, Entropy, EM energy, Charge ρ, Vorticity, Helicity, P₊, P₋, Kinetic, Dual J, Chirality, Confinement |
| **KEEP + cross-label** | ∇·J  Charge ρ; Poynting  Light |
| **KEEP + soften tooltip** | Gravity (FTD-0131), Strong (SU(3) id is [SELECTION]) |
| **FIX — relabel/recolumn** | ∇×J pseudovector (out of the FORCES "weak" slot) |
| **WIRE to real engine — JS-only** (from `getFluxVolume` + native vorticity/helicity samplers) | Coherence C, DM Halo, Genesis, Damping, Curvature K, Horizon, Fisher F |
| **NEW overlays** | Latency `ℒ` (JS), Moore decomposition (JS); State field `s` (C++ binding), Gauss residual (needs State s) |

---

## 5. New-overlay candidates (genuine substrate gaps)

| Overlay | Quantity | Grounding | Tag | Feasibility |
|---|---|---|---|---|
| **State field s** | ternary `{−1,0,+1}` per voxel | the literal FTD ontology — Postulate 3 | [AXIOM] | needs lattice-buffer readback on WASM (mock has it) |
| **Latency / time-dilation** | `L` or `f=1−L²` | Born-Infeld proper-time field; creates wells, horizons, time dilation | [DERIVED] | `getLatencySampled` once C++-bound; engine already computes L |
| **Moore decomposition** | SC + FCC + BCC neighbour shells | Moore Layer Theorem — octahedron+cuboctahedron+stella-octangula | [THEOREM] | pure Three.js wireframe; no bridge data |
| **Gauss residual** | `∇·J − s_charge` | non-variational Gauss-projection conservation leak (`SPEC_ENGINE.md`) | [DERIVED] diagnostic | needs state `s` + existing `getDivJSampled` |

---

## 5.5 Implementation status (2026-06-03 session)

Acted on this audit:

- **Honesty fixes (no rebuild)** — Gravity/Strong tooltips softened to `[SELECTION]`;
  ∇·JCharge ρ and LightPoynting cross-labeled; ∇×J tightened so it no longer reads as the
  weak force (kept in FORCES for the shared style selector; full recolumn out of the force
  scheduler is a noted follow-up).
- **8 unbound samplers wired to the real engine** — `getVorticitySampled`, `getHelicitySampled`,
  `getCurlJSampled`, `getCoherenceSampled`, `getFisherSampled`, `getLatencySampled`,
  `getKretschmannSampled`, `getStateFieldSampled` were never bound, so **Vorticity, Helicity, ∇×J,
  Coherence, Fisher, Curvature K, Horizon** were *all* dead on WASM-owned scenarios (broader than the
  3 derived overlays first flagged). Added the C++ samplers (`engine/wasm/ftd_wasm.cpp`), declarations
  (`bindings_internal.h`), bindings (`bindings_render_bridge.cpp`), the JS proxies (`wasm-bridge.js`),
  and mock equivalents. **Rebuilt + deployed.** Verified on the WasmBridge (flux-vortex / flux-cascade):
  every kind returns data (e.g. vorticity 3375, state 3242, gaussResidual 32768).
- **4 new overlays added** — **State field s** (`[AXIOM]`, ternary point cloud), **Latency L**
  (`[DERIVED]`, blue→red volumetric cloud), **Gauss residual** (`[DERIVED]`, signed cloud — incl. a
  C++ `getGaussResidualSampled`), **Moore decomposition** (`[THEOREM]`, static SC+FCC+BCC wireframe).
  All wired through the full registry chain and renderer-verified (drawRange populates; Moore shells
  24/48/24 edge-verts).
- **Verification** — 71 Playwright tests green (toggle-coverage, overlay-scheduler, wasm-scenario-coverage,
  reconcile-claims, lifecycle-harness, scenario-parity, force-field-samplers). Fixed two pre-existing
  stale assertions in `overlay-scheduler.spec.js` (`cost === 100` → `50`, stale since the 2026-05-31
  `COST_STREAMLINE` 100→50 change).

**Deferred (documented follow-up):** **DM Halo / Genesis / Damping** remain mock-only
(`getScale0DerivedOverlayData` is implemented only on the MockBridge). They work on the default
flux/s0 scenarios and are dead only on the WASM-owned (empty/light/quantum) scenarios. Wiring them to
the WasmBridge needs `getScale0DerivedOverlayData` there, which requires adapting `getParticleData()`'s
shape to the `_particles` shape the renderers expect — a separate task, not bundled here.

## 6. Epistemic notes

- Wiring a mock-only overlay to the engine makes it **engine-true**; it does **not** promote a proxy
  to a true quantity. Φ stays `[PROXY]` until a real Poisson solve backs it; Curvature K stays a
  latency-proxy even when the engine computes it; the `∇×J` overlay stays a curl visualization, not a
  weak force.
- The mock-only finding is itself a result in the spirit of the Number-One Goal's second clause: it
  honestly maps a boundary — *what the web layer actually shows of the engine* — rather than letting a
  dead toggle imply coverage it does not have.
- No physics claim is created or promoted by acting on this audit; the work is visualization plumbing
  plus epistemic labeling.
