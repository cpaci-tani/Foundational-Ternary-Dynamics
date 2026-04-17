# Verification Lab Specification

Status: `[SELECTION]` design spec for the FTD dashboard's self-validation panel
Version: 1.0 (2026-04-16)
Replaces: "Quantum Lab" (8-experiment Monte Carlo tab)

---

## 1. Purpose

The browser dashboard should **prove** the FTD claims, not just illustrate them. Every theoretical prediction in `docs/theory/` and every benchmark in `engine/tests/benchmark_*.cpp` that can run in WebAssembly should be reproducible from the browser, with a pass/fail badge against the canonical theory value.

This elevates validation from "open DevTools, type commands" to a first-class, discoverable UI surface.

Epistemic contract (from `CLAUDE.md`): every experiment here either

- **[THEOREM]** — reproduces a proven prediction; must pass within tolerance
- **[SELECTION]** — reproduces a consistency-argued prediction; expected to pass but softer tolerance
- **[EMERGENT]** — observes a behavior arising from dynamics; displays the measured value, no hard pass/fail
- **[CONJECTURE]** — proposed; reports measured vs. proposed with explicit "unverified" badge

Each experiment's catalog entry carries its tag so the UI can color/badge accordingly.

---

## 2. Architecture

### 2.1 Module layout

```
engine/web/
├── js/
│   ├── verification/
│   │   ├── runner.js              # MeasurementAccumulator + trial orchestration
│   │   ├── registry.js            # EXPERIMENTS catalog (id → definition)
│   │   ├── export.js              # CSV / JSON / clipboard
│   │   ├── categories/
│   │   │   ├── quantum.js         # 8 existing experiments, refactored
│   │   │   ├── conservation.js    # energy / momentum / angular / charge
│   │   │   ├── electromagnetic.js # Coulomb 1/r², field energy
│   │   │   ├── strong.js          # Wilson loops, confinement area law
│   │   │   ├── gravity.js         # time dilation, Kepler, BH thermo
│   │   │   └── emergence.js       # hydrogen 1/n², Bell S=2√2
│   │   └── badge.js               # computePass(measured, predicted, tolerance)
│   ├── ui/panels/verification-lab-panel/
│   │   ├── component.js           # mount + wiring
│   │   ├── template.js            # DOM markup
│   │   └── live-viz.js            # auto-enable Quantum overlays during runs
│   └── quantum-lab.js             # [DEPRECATED] → thin re-export to verification/
│
├── css/ui/panels/
│   └── verification-lab-panel.css # renames quantum-lab-panel.css
│
└── docs/
    └── SPEC_VERIFICATION_LAB.md   # this file
```

### 2.2 Data model — Experiment

```ts
{
  id: string;                  // "quantum-born-rule"
  name: string;                // "Born rule"
  category: Category;          // "quantum" | "conservation" | "em" | "strong" | "gravity" | "emergence"
  epistemicTag: EpistemicTag;  // "THEOREM" | "SELECTION" | "EMERGENT" | "CONJECTURE"
  description: string;         // one-line human summary
  theoryRef?: string;          // docs/theory/.../FILE.md path for deep-dive
  scenarioId: string;          // FTD scenario to load
  overlays?: string[];         // overlay toggle ids to auto-enable during run
  defaultTrials: number;
  defaultTicksPerTrial: number;
  resetFn:   (bridge) => void;
  measureFn: (bridge, trialIndex) => MeasurementResult;
  aggregateFn?: (results) => AggregateResult;     // default: mean + stddev
  theoryFn:  () => { value: number; units: string; };
  tolerance: { relative?: number; absolute?: number };
  formatter: (value: number) => string;           // for display
}
```

`MeasurementResult` can be a scalar or a structured object (double-slit's detector-screen intensity array, e.g.). `AggregateResult` always reduces to:

```ts
{ mean: number; stddev: number; extra?: any }
```

`extra` carries per-experiment display data (fringe positions, FFT peaks, histogram bins).

### 2.3 Pass/fail logic

```js
passes(measured, predicted, tolerance) {
  if (tolerance.absolute != null) return Math.abs(measured - predicted) <= tolerance.absolute;
  return Math.abs(measured - predicted) / Math.max(Math.abs(predicted), 1e-12) <= tolerance.relative;
}
```

Three badge states:
- `✓ PASS` — within tolerance, theoretical match
- `~ CLOSE` — within 2× tolerance, not quite passing
- `✗ FAIL` — further than 2× tolerance
- `—` — not yet run

EMERGENT experiments skip pass/fail; they display measurement only.

### 2.4 Live visualization integration

When an experiment starts running:
1. Load its `scenarioId` via `Scale0Controller.loadScenario(ctx, scenarioId)`.
2. For every overlay id in `experiment.overlays`, activate the corresponding toggle (respecting the §11 "preserve user's overlays" restore logic — we layer on top).
3. After the run completes, the overlays stay on so the user can inspect the final state.

Example: `quantum-born-rule` auto-enables `toggle-psi-squared`. `quantum-double-slit` auto-enables `toggle-phase` and `toggle-psi-squared`. `classical-coulomb-r2` enables `toggle-e-field`. `gravity-time-dilation` enables `toggle-grav-potential`.

---

## 3. Experiment catalog (initial)

### 3.1 Quantum (8 — ported from existing Quantum Lab)

| Id | Name | Theory | Tolerance | Overlays |
|---|---|---|---|---|
| `quantum-born-rule` | Born rule | P(x) ∝ \|J(x)\|² | 5% rel | psi-squared |
| `quantum-double-slit` | Double-slit fringes | Visibility V ≥ 0.5 | 10% rel | psi-squared, phase |
| `quantum-tunnel` | Transmission coefficient | T(W) matches WKB | 15% rel | psi-squared |
| `quantum-well` | Energy-level spacing | ΔE = π²ℏ²n/(2ma²) | 10% rel | psi-squared |
| `quantum-entangle` | Spatial correlation | C(d) singlet form | 10% rel | phase, chirality |
| `quantum-aharonov-bohm` | Phase shift vs enclosed flux | Δφ = 2π·Φ/Φ₀ | 10% rel | phase |
| `quantum-casimir` | Vacuum pressure vs separation | P ∝ 1/d⁴ | 15% rel | light |
| `quantum-zeno` | Decay rate vs measurement interval | Γ(τ) ∝ τ for small τ | 20% rel | psi-squared |

All tagged **[EMERGENT]** (the model doesn't pre-impose QM; we're testing whether QM emerges from the lattice dynamics).

### 3.2 Conservation (4 new)

| Id | Name | Theory | Tolerance | Epistemic |
|---|---|---|---|---|
| `conservation-energy` | Total energy drift over 1000 ticks | |ΔE/E| ≤ 0.01 | 1% abs | THEOREM |
| `conservation-momentum` | \|Σp\| drift | constant under translations | 1% abs | THEOREM |
| `conservation-angular` | \|ΣL\| drift | constant under rotations | 2% abs | THEOREM |
| `conservation-charge` | Σq constant | strict zero drift | 0 abs | THEOREM |

Uses existing bridge diagnostics (`getDiagnostics()`, `getEnergyAudit()`).

### 3.3 Electromagnetic (2 new)

| Id | Name | Theory | Tolerance | Epistemic | Overlays |
|---|---|---|---|---|---|
| `em-coulomb-r2` | F vs r for two charges | F = αq₁q₂/r² | 2% rel | THEOREM | e-field |
| `em-field-energy` | ½∫(E²+B²) matches total EM energy | Poynting + field balance | 3% rel | THEOREM | e-field, b-field |

### 3.4 Strong / Color (2 new)

| Id | Name | Theory | Tolerance | Epistemic | Overlays |
|---|---|---|---|---|---|
| `strong-wilson-loop` | Log ⟨W(R,T)⟩ vs area | Area law: ∝ σ·R·T | 15% rel | SELECTION | confinement |
| `strong-flux-tube` | E(r) linear for color pairs | E(r) = σ·r + const | 10% rel | THEOREM | confinement |

### 3.5 Gravity (3 new)

| Id | Name | Theory | Tolerance | Epistemic | Overlays |
|---|---|---|---|---|---|
| `gravity-kepler` | T² vs a³ for orbits | Kepler's 3rd | 5% rel | THEOREM | grav-potential |
| `gravity-time-dilation` | dt_local vs Φ | Schwarzschild weak-field | 5% rel | EMERGENT | grav-potential |
| `gravity-bh-luminosity` | L peak vs mass | L_peak ≈ 0.62 (unitless) | 10% rel | SELECTION | grav-potential |

### 3.6 Emergence (2 new)

| Id | Name | Theory | Tolerance | Epistemic | Overlays |
|---|---|---|---|---|---|
| `emergence-hydrogen` | 1/n² energy ladder | −13.6/n² eV | 0.1% rel | THEOREM | psi-squared |
| `emergence-bell` | Bell S value at violation peak | S = 2√2 | 2% rel | SELECTION | phase, chirality |

**Total initial: 21 experiments** across 6 categories. Further C++-benchmark ports (continuum→QED limit, budget equation, ontic constants sweep) can follow as Tier-2 additions.

---

## 4. UI design

### 4.1 Panel layout

```
┌─────────────────────────────────────────────────────────────────┐
│ VERIFICATION LAB                            [Export all JSON]    │
├─────────────────────────────────────────────────────────────────┤
│ [Quantum] [Conservation] [EM] [Strong] [Gravity] [Emergence]    │  ← category pills
├──────────────────────┬──────────────────────────────────────────┤
│ ● Born rule      ✓   │ ▶ Born rule                              │
│ ● Double-slit    ✓   │                                          │
│ ● Tunneling      ~   │ [EMERGENT] P(x) ∝ |J(x)|² should emerge  │
│ ● Energy wells   —   │ from the lattice dynamics.               │
│ ● Entanglement   —   │                                          │
│ ● Aharonov-B     ✗   │ Scenario: quantum-born-rule              │
│ ● Casimir        —   │ Trials: [100    ] Ticks: [200]           │
│ ● Zeno           —   │                                          │
│                      │ [▶ Run]      [Stop]      [Export CSV]    │
│                      │                                          │
│                      │ Progress: ████████▏░░░ 85% (85/100)      │
│                      │                                          │
│                      │ MEASUREMENT          THEORY      BADGE    │
│                      │ 0.953 ± 0.021        1.000       ✓ PASS  │
│                      │                                          │
│                      │ ┌── histogram / fringe / sparkline ──┐   │
│                      │ │                                      │   │
│                      │ └──────────────────────────────────────┘   │
└──────────────────────┴──────────────────────────────────────────┘
```

### 4.2 Badges

- `✓ PASS` — green (uses `--positive`)
- `~ CLOSE` — amber (uses `--warning`)
- `✗ FAIL` — red (uses `--negative`)
- `—` — muted
- `[EMERGENT]` — accent-tinted label, no pass/fail

### 4.3 Tab name

Rename `Quantum Lab` → `Verify`. Shorter, universal, fits the tab bar.

### 4.4 Discoverability

- Every FTD scenario that matches an experiment shows a small "Verify" chip in the scenario-meta drawer: clicking jumps to the corresponding experiment with the scenario preloaded.
- An "Overview" sub-tab shows all categories' pass counts: `Quantum 7/8 · Conservation 4/4 · …` — glanceable health of the engine.

---

## 5. Implementation plan

**This session (MVP):**

1. Scaffold `verification/` module tree (runner, registry, export, badge).
2. Port the 8 existing quantum experiments to the new catalog format with theoryFn / tolerance / epistemicTag.
3. Build the new `verification-lab-panel` (rename + restructure).
4. Rename the tab `Quantum Lab` → `Verify`.
5. Wire pass/fail badges, category pills, experiment list.
6. Implement live-viz integration (auto-enable overlays on run).
7. Add 4 **Conservation** experiments (easy wins — diagnostic data already collected).

Deferred to follow-ups:

- EM, Strong, Gravity, Emergence categories (require new bridge probes for some)
- Overview sub-tab
- Scenario-meta "Verify" chip jump
- Histogram/fringe visualizations beyond the default sparkline

**Exit criteria (this session):**

- [x] Spec written
- [ ] New module tree exists
- [ ] 12 experiments catalogued (8 quantum + 4 conservation)
- [ ] Category pills render + filter list
- [ ] Experiment list shows status per entry
- [ ] Run → progress → pass/fail badge flow works end-to-end for at least one experiment
- [ ] Old `quantum-lab.js` replaced (re-export only) or removed
- [ ] Tab renamed
- [ ] Live overlay auto-enable works for Born rule (psi-squared) at minimum
- [ ] CSV export of a completed run
