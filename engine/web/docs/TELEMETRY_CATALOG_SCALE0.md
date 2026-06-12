# Scale 0 Telemetry Catalog

> Quick reference for every Scale 0 telemetry and chart in the web dashboard.
> Source of truth for wiring: the three descriptor files listed under
> **Producers** below. If you add a metric, add it to the descriptor
> and update this catalog in the same commit.

**Scale 0** is the flux-lattice engine. Its telemetry flows through a single
hub (`engine/web/js/telemetry-hub.js`) that every consumer reads from.
Future per-scale catalogs should follow this same shape.

---

## 1. Architecture

```
                 ┌──────────────────────────────────────────┐
                 │  Scale 0 controller + energy-audit       │
                 │  (every tick, per-frame)                 │
                 └─────────────────┬────────────────────────┘
                                   │ writes
                                   ▼
                 ┌──────────────────────────────────────────┐
                 │ telemetry-hub.js — TelemetryHub          │
                 │                                           │
                 │  hub.s0.diag        { scalar snapshots } │
                 │  hub.s0.audit       { energy-audit      }│
                 │  hub.s0.lagrangian  { 7 terms + totals  }│
                 │                                           │
                 │  Ring buffers (time series):             │
                 │    hub.flux / energy / entropy / …       │
                 │    hub.aud.*   (14 buffers, 500 samples) │
                 │    hub.sp.*    (5 buffers, 80 samples)   │
                 │    hub.lag.*   (10 buffers, 400 samples) │
                 └─────────────────┬────────────────────────┘
                                   │ reads
                 ┌─────────────────┴────────────────────────┐
                 ▼                                          ▼
     ┌─────────────────────┐                ┌──────────────────────┐
     │ Diagnostics panel   │                │ Charts panel         │
     │ 5 sections ×        │                │ 6 line/area charts   │
     │ 25 scalar rows      │                │ × 10 total series    │
     │ + inline sparklines │                │                      │
     └─────────────────────┘                └──────────────────────┘
                 ▼
     ┌──────────────────────────────────────────────────────┐
     │ Lagrangian panel                                      │
     │ 7 stacked terms + 8 action rows + 8 constant rows    │
     └──────────────────────────────────────────────────────┘
```

### Descriptor files (source of truth)

| Panel | Descriptor |
|---|---|
| Diagnostics | `engine/web/js/ui/panels/diagnostics-panel/descriptors/scale0.js` |
| Charts | `engine/web/js/ui/panels/charts-panel/descriptors/scale0.js` |
| Lagrangian | `engine/web/js/ui/panels/lagrangian-panel/descriptors/scale0.js` |

### Unit conventions

| Unit | Meaning |
|---|---|
| `E*` | Lattice energy unit (flux² · dx³) |
| `E*²` | Energy squared (for constraint violations) |
| `|J|` | Flux magnitude |
| `|S|` | Poynting magnitude (E* · c / dx) |
| `\|w\|²` | Wave-substrate energy |
| `ℏ` | Action / angular-momentum unit |
| `nat` | Natural-log entropy |
| `ct` | Pure count |
| `''` | Dimensionless ratio (em-dash rendered in UI) |

---

## 2. Ring buffers (hub globals)

The 500- / 400- / 80-sample ring buffers in `TelemetryHub`. Any consumer
can read any of these; they are the only mutable state in the hub.

### 2.1 Headline trends (500 samples)

| Buffer | Units | What it tracks | Used by |
|---|---|---|---|
| `hub.flux` | \|J\| | `s0.diag.totalFlux` | Diagnostics (Energy Budget), Charts (Flux & Energy) |
| `hub.energy` | E* | `s0.diag.totalEnergy` | Diagnostics (Energy Budget), Charts (Flux & Energy) |
| `hub.manifested` | ct | particle count | Diagnostics (Particle State), Charts (Particle Count) |
| `hub.entropy` | nat | `s0.diag.entropy` | Diagnostics, Charts (Entropy) |
| `hub.charges` | ct | `pos − neg` | Diagnostics (Charge net), Charts (Charge Balance) |
| `hub.positive` | ct | `s0.diag.positive` | Diagnostics, Charts (Particle Count) |
| `hub.negative` | ct | `s0.diag.negative` | Diagnostics, Charts (Particle Count) |
| `hub.ebDiff` | E* | `E-energy − B-energy` | Charts (E vs B Field Energy) |
| `hub.gauss` | E*² | `s0.audit.gaussViolation` | Diagnostics (Constraints), Charts (Gauss Violation) |

### 2.2 Audit detail (`hub.aud.*`, 500 samples)

| Buffer | Units | Source snapshot |
|---|---|---|
| `aud.fieldEnergy` | E* | `s0.audit.fieldEnergy` |
| `aud.waveEnergy` | E* | `s0.audit.waveEnergy` |
| `aud.particleKE` | E* | `s0.audit.particleKE` |
| `aud.coulombPE` | E* | `s0.audit.coulombPE` |
| `aud.eFieldEnergy` | E* | `s0.audit.EFieldEnergy` / `eFieldEnergy` |
| `aud.bFieldEnergy` | E* | `s0.audit.BFieldEnergy` / `bFieldEnergy` |
| `aud.poyntingMag` | \|S\| | `|s0.audit.totalPoynting|` |
| `aud.maxGaussError` | E* | `s0.audit.maxGaussError` |
| `aud.selfFieldInjection` | E* | `s0.audit.selfFieldInjection` |
| `aud.eLeftEnergy` | E* | `s0.audit.ELTotal` / `eLTotal` |
| `aud.eRightEnergy` | E* | `s0.audit.ERTotal` / `eRTotal` |
| `aud.chirality` | '' | `s0.audit.chiralityTotal` |
| `aud.waveLeft` | \|w\|² | `s0.audit.wvLTotal` |
| `aud.waveRight` | \|w\|² | `s0.audit.wvRTotal` |

### 2.3 Sparkline buffers (`hub.sp.*`, 80 samples)

Smaller, faster-wrapping buffers for inline sparklines in the diagnostics table.

| Buffer | Tracks |
|---|---|
| `sp.manifested` | particle count |
| `sp.charges` | net charge |
| `sp.flux` | total flux |
| `sp.energy` | total energy |
| `sp.entropy` | entropy |

### 2.4 Lagrangian term buffers (`hub.lag.*`, 400 samples)

| Buffer | Units | Role |
|---|---|---|
| `lag.fieldKinetic` | E* | kinetic term |
| `lag.fieldGradient` | E* | gradient term |
| `lag.bornInfeld` | E* | Born-Infeld term |
| `lag.coupling` | E* | source-flux coupling term |
| `lag.velocity` | E* | velocity term |
| `lag.gauss` | E*² | Gauss penalty term |
| `lag.dissipation` | E* | dissipation term |
| `lag.total` | E* | `ℒ` total (sum of the 7 terms above) |
| `lag.hamiltonian` | E* | Legendre-transformed total |
| `lag.action` | ℏ | running `∫ ℒ dt` |

---

## 3. Diagnostics panel (25 rows across 5 sections)

Rendered by `engine/web/js/ui/panels/diagnostics-panel/descriptors/scale0.js`.

### 3.1 Particle State (7 rows)

| Metric | ID | Unit | Hub source | Trend buffer | Format |
|---|---|---|---|---|---|
| Manifested | `manifested` | ct | `s0.diag.manifested` | `manifested` | scalar |
| Positive | `positive` | ct | `s0.diag.positive` | `positive` | scalar (variant: positive) |
| Negative | `negative` | ct | `s0.diag.negative` | `negative` | scalar (variant: negative) |
| Charge (net) | `charge` | ct | `s0.diag.chargeBalance` | `charges` | scalar |
| Spin Up/Down | `spin` | ct | `s0.diag.spinUp`, `s0.diag.spinDown` | — | pair |
| Color R/G/B | `color` | ct | `s0.diag.colorRed`, `.colorGreen`, `.colorBlue` | — | triple |
| Colorless | `colorless` | ct | `s0.diag.colorless` | — | scalar |

### 3.2 Energy Budget (7 rows)

| Metric | ID | Unit | Hub source | Trend buffer | Format |
|---|---|---|---|---|---|
| Total Energy | `energy` | E* | `s0.diag.totalEnergy` | `energy` | scalar |
| Field \|J\|² | `field-energy` | E* | `s0.audit.fieldEnergy` | `aud.fieldEnergy` | scalar |
| Wave \|w\|² | `wave-energy` | E* | `s0.audit.waveEnergy` | `aud.waveEnergy` | scalar |
| Particle KE | `particle-ke` | E* | `s0.audit.particleKE` | `aud.particleKE` | scalar |
| Coulomb PE | `coulomb-pe` | E* | `s0.audit.coulombPE` | `aud.coulombPE` | scalar |
| Total Flux | `flux` | \|J\| | `s0.diag.totalFlux` | `flux` | scalar |
| Entropy | `entropy` | nat | `s0.diag.entropy` | `entropy` | scalar |

### 3.3 Electromagnetic (4 rows)

| Metric | ID | Unit | Hub source | Trend buffer | Format |
|---|---|---|---|---|---|
| E-Field \|E\|²/2 | `e-field` | E* | `s0.audit.EFieldEnergy` (fallback `eFieldEnergy`) | `aud.eFieldEnergy` | computed |
| B-Field \|B\|²/2 | `b-field` | E* | `s0.audit.BFieldEnergy` (fallback `bFieldEnergy`) | `aud.bFieldEnergy` | computed |
| Poynting \|S\| | `poynting` | \|S\| | `\|s0.audit.totalPoynting\|` | `aud.poyntingMag` | computed |
| Angular Mom | `ang-mom` | ℏ | `s0.diag.angMomX/Y/Z` | — | vector |

### 3.4 Constraints (3 rows)

| Metric | ID | Unit | Hub source | Trend buffer | Format |
|---|---|---|---|---|---|
| Gauss Σ(div J−s)² | `gauss` | E*² | `s0.audit.gaussViolation` | `gauss` | scalar |
| Max Gauss err | `max-gauss` | E* | `s0.audit.maxGaussError` | `aud.maxGaussError` | scalar |
| Self-field inj | `self-inj` | E* | `s0.audit.selfFieldInjection` | `aud.selfFieldInjection` | scalar |

### 3.5 Dual Substrate (4 rows)

| Metric | ID | Unit | Hub source | Trend buffer | Format |
|---|---|---|---|---|---|
| E_L (left) | `e-left` | E* | `s0.audit.ELTotal` (fallback `eLTotal`) | `aud.eLeftEnergy` | computed |
| E_R (right) | `e-right` | E* | `s0.audit.ERTotal` (fallback `eRTotal`) | `aud.eRightEnergy` | computed |
| Chirality | `chirality` | '' | `s0.audit.chiralityTotal` | `aud.chirality` | scalar |
| Wave L / R | `wave-lr` | \|w\|² | `s0.audit.wvLTotal`, `.wvRTotal` | — | pair |

---

## 4. Charts panel (6 charts)

Rendered by `engine/web/js/ui/panels/charts-panel/descriptors/scale0.js`.
Default-active charts render on tab open; inactive are one click away.

| Chart | ID | X | Y | Series | Buffer | Color var | Default |
|---|---|---|---|---|---|---|---|
| **Flux & Energy** | `flux-energy` | tick | E* | Flux | `flux` | `--chart-flux` | ✓ |
| | | | | Energy | `energy` | `--chart-energy` | |
| **Particle Count** | `particles` | tick | ct | Total | `manifested` | `--chart-total` | ✓ |
| | | | | Positive | `positive` | `--chart-positive` | |
| | | | | Negative | `negative` | `--chart-negative` | |
| **Charge Balance** | `charge` | tick | pos−neg | Charge | `charges` | `--chart-charge` | ✓ |
| **E vs B Field Energy** | `eb-energy` | tick | E* (E−B) | E−B | `ebDiff` | `--chart-eb` | |
| **Gauss Violation** | `gauss` | tick | E*² | Violation | `gauss` | `--chart-gauss` | |
| **Entropy** | `entropy` | tick | nat | Entropy | `entropy` | `--chart-entropy` | ✓ |

Total: 6 charts × 10 series.

---

## 5. Lagrangian panel (7 terms + 8 action rows + 8 constants)

Rendered by `engine/web/js/ui/panels/lagrangian-panel/descriptors/scale0.js`.

### 5.1 Stacked-area terms

Stacked area chart; each term is a legend-toggleable series pulled from a `hub.lag.*` buffer. All default-on.

| Key | Label | Buffer | Color var |
|---|---|---|---|
| `fieldKinetic` | Field KE | `lag.fieldKinetic` | `--legend-field-kinetic` |
| `fieldGradient` | Gradient | `lag.fieldGradient` | `--legend-field-gradient` |
| `bornInfeld` | Born-Infeld | `lag.bornInfeld` | `--legend-bi` |
| `coupling` | Coupling | `lag.coupling` | `--legend-coupling` |
| `velocity` | Velocity | `lag.velocity` | `--legend-velocity` |
| `gauss` | Gauss | `lag.gauss` | `--legend-gauss` |
| `dissipation` | Dissipation | `lag.dissipation` | `--legend-dissipation` |

### 5.2 Action / Lagrangian side table (8 rows)

| Metric | ID | Unit | Hub source | Trend buffer |
|---|---|---|---|---|
| Action `S` | `action` | ℏ | `s0.lagrangian.totalAction` | `lag.action` |
| `ℒ` total | `total` | E* | `s0.lagrangian.total` | `lag.total` |
| Hamiltonian `H` | `hamiltonian` | E* | `s0.lagrangian.hamiltonian` | `lag.hamiltonian` |
| Gauss `‖div J−s‖` | `gauss` | E*² | `s0.lagrangian.gauss` | `lag.gauss` |
| Max Gauss err | `max-gauss` | E* | `s0.audit.maxGaussError` | `aud.maxGaussError` |
| Total \|J\| | `flux-mag` | \|J\| | `s0.diag.totalFlux` | `flux` |
| Wave KE | `wave-ke` | E* | `s0.audit.waveEnergy` | `aud.waveEnergy` |
| Manifested | `manifested` | ct | `s0.diag.manifested` | `manifested` |

### 5.3 Constants side table (8 rows, static — no hub source)

| Metric | ID | Unit | Constant module path |
|---|---|---|---|
| `G*` | `gstar` | '' | `consts.G_STAR` |
| `1/α` | `alpha-inv` | '' | `consts.X_PLUS` |
| `α` | `alpha` | '' | `consts.ALPHA` |
| `K_B` | `kb` | MeV | `consts.K_B` |
| `G_N` | `gn` | '' | `consts.G_N` |
| `g_c` | `gc` | '' | `consts.G_C` |
| `N_c` | `nc` | ct | `consts.N_C` |
| `N_eff` | `neff` | ct | `consts.N_EFF` |

---

## 6. Wiring-verification checklist

Quick visual sweep when adding / renaming a Scale 0 metric:

- [ ] Producer writes to `hub.s0.diag`, `hub.s0.audit`, or `hub.s0.lagrangian` every tick.
- [ ] If you want a sparkline or full chart, the producer also records into the matching `hub.*` ring buffer (or `hub.aud.*`, `hub.sp.*`, `hub.lag.*`).
- [ ] A row is added to the relevant descriptor file (section 3 / 4 / 5 above).
- [ ] This catalog is updated in the same commit.
- [ ] Reload the page; the new row shows a number (not `—`) and the sparkline animates (not flat) under a non-trivial scenario like `flux-pulse` or `electron-positron-pair`.
- [ ] `engine/web/tests/scales.spec.js` still green (`npx playwright test scales.spec.js`) — the shell-init test exercises all registered panels and will surface producer-wiring regressions.

Red flags that indicate a broken wiring:

- Value shows as literal `—` while simulation is running → producer never wrote the field, OR the descriptor's `source` path is misspelled.
- Sparkline stays flat (orange horizontal line) → the `trend` buffer name doesn't match a buffer declared in `TelemetryHub.constructor()`.
- Value shows as `NaN`, `Infinity`, or `0.0000` stuck forever → the ring buffer is getting `record(undefined)` calls (upstream producer returning no value).
- Row renders `undefined` literal as text → the `compute()` callback is throwing or returning `undefined` for a missing sub-field (wrap with `??`).

---

## 7. Sibling catalogs (per scale)

The per-scale split ensures each scale can evolve its telemetry independently. Expected companion files as those scales get documented:

- `TELEMETRY_CATALOG_SCALE1.md` — Particle Engine (PE telemetry ring buffers, PE telemetry panel)
- `TELEMETRY_CATALOG_SCALE2.md` — Atom / Molecule Engine (AE)
- `TELEMETRY_CATALOG_SCALE4.md` — Planetary
- `TELEMETRY_CATALOG_SCALE5.md` — Cosmic
- `TELEMETRY_CATALOG_SCALE11.md` — Reference frame context

When a new scale is documented, add it here with a one-line summary and a link.
