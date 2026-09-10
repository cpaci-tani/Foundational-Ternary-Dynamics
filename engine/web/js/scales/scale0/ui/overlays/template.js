/**
 * Scale 0 Viewport Overlay — Field visualization controls
 *
 * A compact layer inspector: command header, filter, active-layer rail, shared
 * presentation controls, then semantic accordion cards. The collapse / filter /
 * active-rail behavior lives in overlays/panel-shell.js; the physics toggles remain
 * wired in scale0/ui/bindings.js. Context controls (volume treatment, slice planes,
 * sheet height) stay adjacent to their owning layer and are revealed only while that
 * layer is active. The categories:
 *
 *   STANDARD MODEL — contextual catalog reference, only on elementary-particle scenarios
 *   VOLUME    — how the raw flux field is rendered (volume, slice, lines, ∇·J)
 *   FIELDS    — EM-derived vector fields (E, B)
 *   FIELD ENERGY & FLOW — field energy channels, Poynting flow, and flux curl
 *   FORCES    — per-particle force vectors; its presentation selector is shared
 *               with the scalar selector in the panel-level Render card
 *   PHENOMENA — emergent / composite overlays (chirality, DM halo, confinement, …)
 *
 * Rendering choices share each layer's existing control. Labels and tooltips
 * identify the sampled field quantity and its interpretation limits.
 */

export function getScale0OverlayTemplate() {
  const container = document.createElement('div');
  container.id = 'viewport-overlay';
  container.className = 'scale0-only s0-overlay-panel';
  container.innerHTML = `
    <header class="s0-overlay-header">
      <div class="s0-overlay-identity">
        <span class="s0-overlay-mark" aria-hidden="true">
          <span></span><span></span><span></span>
        </span>
        <span class="s0-overlay-title-group">
          <span class="s0-overlay-title">Visualization</span>
          <span class="s0-overlay-subtitle">Scale 0 · reference field layers</span>
        </span>
      </div>
      <div class="s0-overlay-header-tools">
        <span class="s0-overlay-summary" id="s0-overlay-summary" aria-live="polite">0 active</span>
        <button class="s0-overlay-collapse u-no-baseline" type="button"
            aria-label="Collapse visualization overlay"
            aria-expanded="true"
            title="Collapse overlay">
          <span class="s0-overlay-collapse-icon" aria-hidden="true"></span>
        </button>
      </div>
    </header>
    <div class="s0-overlay-command">
      <label class="s0-overlay-search" for="s0-overlay-search">
        <span class="s0-overlay-search-icon" aria-hidden="true"></span>
        <input type="search" id="s0-overlay-search" class="s0-overlay-search-input"
            placeholder="Find a field layer" autocomplete="off" spellcheck="false"
            aria-label="Filter visualization overlays" />
        <button class="s0-overlay-search-clear u-no-baseline" id="s0-overlay-search-clear"
            type="button" aria-label="Clear overlay filter" title="Clear filter" hidden></button>
      </label>
    </div>
    <div class="s0-overlay-active" id="s0-overlay-active" aria-label="Active overlays" hidden></div>
    <section class="s0-overlay-render-deck" aria-labelledby="s0-overlay-render-title">
      <div class="s0-overlay-render-head">
        <span id="s0-overlay-render-title">Rendering</span>
        <span>presentation only</span>
      </div>
      <div class="s0-overlay-render-row">
        <span class="s0-overlay-render-label">Scalar</span>
        <div class="force-style-row" id="scalar-render-row" role="group"
            aria-label="Scalar layer rendering"
            title="Render scalar overlays as their native surface/cloud, a glow heat map, or a smooth volume. Volume interpolation and opacity affect presentation only.">
          <button class="style-btn active" type="button" data-scalar-mode="default"
              aria-pressed="true" title="Native rubber-sheet or scalar-cloud rendering">Surface</button>
          <button class="style-btn" type="button" data-scalar-mode="heatmap"
              aria-pressed="false" title="Volumetric thermal glow heat map">Heat map</button>
          <button class="style-btn" type="button" data-scalar-mode="volume"
              aria-pressed="false" title="Smooth volume heatmap of the selected lattice quantity; interpolation is visual only">Volume</button>
        </div>
      </div>
      <label id="scalar-volume-controls" for="scalar-volume-opacity" style="display:none; align-items:center; gap:8px; margin:6px 0">
        Opacity <input id="scalar-volume-opacity" type="range" min="0" max="1" step="0.05" value="0.75" style="min-width:0; flex:1">
        <output id="scalar-volume-opacity-value" for="scalar-volume-opacity">75%</output>
      </label>
      <div class="s0-overlay-render-row">
        <span class="s0-overlay-render-label">Vector</span>
        <div class="force-style-row" id="force-style-row" role="group"
            aria-label="Vector force rendering" title="Render style for force-field overlays">
          <button class="style-btn active" type="button" data-style="arrows" aria-pressed="true" title="Vector arrows">Arrows</button>
          <button class="style-btn" type="button" data-style="heatmap" aria-pressed="false" title="Gaussian heatmap">Heat</button>
          <button class="style-btn" type="button" data-style="glyphs" aria-pressed="false" title="Oriented glyph field">Glyphs</button>
        </div>
      </div>
    </section>
    <div class="s0-overlay-body">
    <div class="s0-overlay-col" data-col="standard-model">
      <div class="s0-overlay-col-head">
        <span class="s0-overlay-col-label">Standard Model</span>
        <span class="s0-overlay-col-count" data-count-for="standard-model" aria-hidden="true">0</span>
        <button class="s0-overlay-col-clear u-no-baseline" data-clear-col="standard-model" type="button"
            title="Hide the Standard Model reference overlay">&#10005;</button>
      </div>
      <div class="s0-sm-context-card" id="s0-sm-context-card">
        <div class="s0-sm-context-head">
          <span><strong data-sm-field="symbol">—</strong> <span data-sm-field="name">Standard Model particle</span></span>
          <span class="s0-sm-reference-tag">Reference</span>
        </div>
        <div class="s0-sm-context-values">
          <span>spin <b data-sm-field="spin">—</b></span>
          <span>Q <b data-sm-field="charge">—</b></span>
          <span>chirality <b data-sm-field="chirality">—</b></span>
          <span>gen <b data-sm-field="generation">—</b></span>
          <span>color <b data-sm-field="color">—</b></span>
        </div>
        <p>Catalog quantum numbers only. They are not measured or derived by this Scale 0 template.</p>
      </div>
      <button class="view-toggle field-toggle" id="toggle-sm-reference"
          title="Show a compact viewport card of Standard Model reference quantum numbers: spin, electric charge, chiral sector, generation, and color representation. This is static catalog context, not live Scale 0 data; the scenario's audited identity status is preserved.">
        <span class="field-swatch field-swatch-sm-reference"></span>Quantum numbers
      </button>
    </div>

    <div class="s0-overlay-col" data-col="volume">
      <div class="s0-overlay-col-head">
        <span class="s0-overlay-col-label">Volume</span>
        <span class="s0-overlay-col-count" data-count-for="volume" aria-hidden="true">0</span>
        <button class="s0-overlay-col-clear u-no-baseline" data-clear-col="volume" type="button"
            title="Turn off every overlay in this column">&#10005;</button>
      </div>
      <div class="s0-overlay-group">
        <button class="view-toggle active" id="toggle-flux-volume"
            title="[PROXY — visualization] Volumetric activation cloud. Each voxel combines its own 1/2|J|², the mean energy of its surrounding 26 Moore neighbours, and |s|E_REST. Every available source voxel is evaluated at a fixed coordinate; threshold only hides voxels below the selected relative energy cutoff. Stronger energy makes the point larger and advances its blue→cyan→yellow→red colour phase. At threshold 0 every available voxel is shown. Native large-L frames may provide an even-stride source grid while the engine continues computing the full lattice.">Flux Volume</button>
        <div class="flux-slice-axis-row" role="group" aria-label="Flux volume style">
          <button class="view-toggle flux-slice-axis-mini" id="toggle-flux-organic"
              aria-pressed="false" title="Organic scatter jitter (±½ cell), independent of lattice size. Off by default so the exact lattice remains visible.">Organic</button>
          <button class="view-toggle flux-slice-axis-mini active" id="toggle-flux-glow"
              title="Additive glow bloom on the flux volume">Glow</button>
        </div>
      </div>
      <div class="s0-overlay-group">
        <button class="view-toggle" id="toggle-flux-slice"
            title="2D slice of |J| (flux magnitude, not the vector J) through the lattice mid-planes — xy @ z=L/2, xz @ y=L/2, yz @ x=L/2. Magnitude only, so it is sign-blind; use ∇·J for sources and sinks.">Flux Slice</button>
        <div class="flux-slice-axis-row" role="group" aria-label="Flux slice planes">
          <button class="view-toggle flux-slice-axis-mini active" id="flux-slice-axis-xy"
              title="Toggle the xy mid-plane (z = L/2)">xy</button>
          <button class="view-toggle flux-slice-axis-mini active" id="flux-slice-axis-xz"
              title="Toggle the xz mid-plane (y = L/2)">xz</button>
          <button class="view-toggle flux-slice-axis-mini active" id="flux-slice-axis-yz"
              title="Toggle the yz mid-plane (x = L/2)">yz</button>
        </div>
      </div>
      <button class="view-toggle field-toggle" id="toggle-flux-lines"
          title="Streamlines of the J-field showing flow direction. Color encodes the LOCAL |J| magnitude at each point along the line — same ramp as Flux Volume (blue=weak, red=strong), not the vertex's position along the line.">
        <span class="field-swatch field-swatch-flux-lines"></span>Flux Lines
      </button>
      <button class="view-toggle field-toggle" id="toggle-div-field"
          title="[SELECTION] Divergence ∇·J. The correction targets a coupled, mean-neutralized source and normally acts only on vacuum sites. This raw divergence is not that solver residual or a derivation of charge conservation. Same buffer as Charge ρ (Topology column), rendered here as points instead of a rubber sheet.">
        <span class="field-swatch field-swatch-divj"></span>&nabla;&middot;J
      </button>
      <button class="view-toggle field-toggle" id="toggle-state-field"
          title="[REFERENCE ENGINE] Ternary state field s ∈ {−1,0,+1}, the continuous-J engine manifestation layer. The v3 strict law instead has a complete finite record; this overlay is not its microscopic state. Manifested voxels render as points (s=−1 blue, s=+1 red); void (s=0) is invisible. Which voxels light up is set by the genesis threshold K_GENESIS (a calibrated value) plus stochastic evaporation.">
        <span class="field-swatch field-swatch-state"></span>State s
      </button>
    </div>

    <div class="s0-overlay-col" data-col="fields">
      <div class="s0-overlay-col-head">
        <span class="s0-overlay-col-label">Fields</span>
        <span class="s0-overlay-col-count" data-count-for="fields" aria-hidden="true">0</span>
        <button class="s0-overlay-col-clear u-no-baseline" data-clear-col="fields" type="button"
            title="Turn off every overlay in this column">&#10005;</button>
      </div>
      <button class="view-toggle field-toggle" id="toggle-e-field"
          title="[SELECTION] Radiative (inductive) electric field ONLY: E = -∂J/∂t (A≡J), drawn as streamlines. This is the temporal-gauge part only — the longitudinal Coulomb near-field -∇φ_C, which actually drives electrostatic force, is NOT included here, so a settled static charge renders almost nothing. The Coulomb field is available separately via the EM force overlay (Forces column — getEMForceField, real α/4π coupling).">
        <span class="field-swatch field-swatch-e-field"></span>Radiative E (&minus;&part;J/&part;t)
      </button>
      <button class="view-toggle field-toggle" id="toggle-b-field"
          title="[SELECTION] Magnetic field streamlines (B = ∇×J). Same curl operator as the &nabla;&times;J pseudovector in the Forces column, which is explicitly disclaimed there as a [PROXY] parity-even (axial) pseudovector, not the SM weak force — this overlay's Maxwell identification carries the same honesty.">
        <span class="field-swatch field-swatch-b-field"></span>B Field
      </button>
    </div>

    <div class="s0-overlay-col" data-col="forces">
      <div class="s0-overlay-col-head">
        <span class="s0-overlay-col-label">Forces</span>
        <span class="s0-overlay-col-count" data-count-for="forces" aria-hidden="true">0</span>
        <button class="s0-overlay-col-clear u-no-baseline" data-clear-col="forces" type="button"
            title="Turn off every force overlay (preserves the current style selection)">&#10005;</button>
      </div>
      <button class="view-toggle field-toggle" id="toggle-force-em"
          title="[PARAMETRIC] Electrostatic force on a unit test charge: F = (α/4π)·Σ_p s_p·(r-r_p)/(|r-r_p|²+1)^1.5, summed over manifested voxels with periodic minimum-image and 1-voxel softening. Textbook continuum Coulomb law with FTD's α inserted — NOT the lattice Green's function of Phase G, and NOT the force the tick loop actually applies. No Lorentz v×B term is included, even when the lorentz_force toggle is on.">
        <span class="field-swatch field-swatch-em"></span>EM
      </button>
      <button class="view-toggle field-toggle" id="toggle-force-gravity"
          title="[SELECTION — visualization] Exact finite Scale-0 gravity-law field. Default: G_N·δ₂|J| with δ₂f=[f(x+2)−f(x−2)]/4 on the periodic computational quotient. With Geometric Gravity selected: M·c²·L·δ₂L. The tick applies this field only at manifested sites; the overlay samples it throughout the lattice. G_N's identification with physical G is falsified (LEDGER FTD-0131), so this is not presented as derived Newtonian gravity.">
        <span class="field-swatch field-swatch-gravity"></span>Gravity
      </button>
      <button class="view-toggle field-toggle" id="toggle-force-strong"
          title="[SELECTION — visualization] Pairwise flux-tube force between all manifested voxels inside a Gaussian tube envelope: Coulomb for r<3, transition for 3≤r<8, harmonic F∝r (V∝r²) for r≥8 — the harmonic long-range arm is NOT the linear potential of area-law confinement. Confinement of record is [THEOREM-within-compact-U(1)-LGT; SELECTION at FTD-substrate level] (LEDGER FTD-0025).">
        <span class="field-swatch field-swatch-strong"></span>Strong
      </button>
      <button class="view-toggle field-toggle" id="toggle-force-weak"
          title="[PROXY — VISUALIZATION ONLY] The curl ∇×J (a parity-even (axial) pseudovector) rendered as arrows, scaled by DUAL_DELTA ≈ 0.957. This is a vector-calculus view of J, NOT the SM weak force — FTD's weak interaction is state transmutation (weak_transmutation toggle). Companion to Flux curl |∇×J| (Field energy &amp; flow). Lives in this column only to share the force-style selector. (audit P1-17, 2026-05-27)">
        <span class="field-swatch field-swatch-weak"></span>&nabla;&times;J pseudovector
      </button>
    </div>

    <div class="s0-overlay-col" data-col="quantum">
      <div class="s0-overlay-col-head">
        <span class="s0-overlay-col-label">Quantum</span>
        <span class="s0-overlay-col-count" data-count-for="quantum" aria-hidden="true">0</span>
        <button class="s0-overlay-col-clear u-no-baseline" data-clear-col="quantum" type="button"
            title="Turn off every quantum overlay">&#10005;</button>
      </div>
      <button class="view-toggle field-toggle" id="toggle-psi-squared"
          title="[PROXY] Flux amplitude squared |J|², max-normalized. This quantity alone is not the conserved wave Hamiltonian. Computed as |J|² unconditionally; the Dual Substrate toggle does not change this overlay's math. Displayed as a Born-style |ψ|² analogue, but FTD does NOT derive probability from it: the energy-density-to-probability step is [OPEN] (LEDGER FTD-0187) and measured threshold-crossing Born scaling came back [CLOSED NEGATIVE] (FTD-0200).">
        <span class="field-swatch field-swatch-psi-squared"></span>|&psi;|&sup2;
      </button>
      <button class="view-toggle field-toggle" id="toggle-phase"
          title="[PROXY] Complex phase φ = arg(J_L + i·J_R), requires Dual Substrate on. Under the current scalar (1±δ)/2 split, J_L and J_R are both scalar multiples of the same J, so φ collapses to one fixed constant everywhere — not a spatially-varying field. Retained as a placeholder pending a real chiral (curl-free/divergence-free) decomposition.">
        <span class="field-swatch field-swatch-phase"></span>Phase &phi;
      </button>
      <button class="view-toggle field-toggle" id="toggle-lagrangian-density"
          title="[PROXY] Kinetic-vs-gradient balance ½|E|² − ½(∇·J)² — NOT the engine's true Lagrangian. Substitutes two available field terms for a pedagogical stand-in; there is no V(s,J) term (state s is not sampled here).">
        <span class="field-swatch field-swatch-lagrangian"></span>&#8466;(x)
      </button>
      <button class="view-toggle field-toggle" id="toggle-entropy-density"
          title="[PROXY] Disorder proxy 4p(1−p) with p=|J|/|J|_max (Gini-style impurity). Pointwise in |J| with a GLOBAL normalizer, not neighborhood-local — a true local Shannon entropy of the ternary state over a Moore neighborhood would need a separate overlay.">
        <span class="field-swatch field-swatch-entropy"></span>Entropy s
      </button>
    </div>

    <div class="s0-overlay-col" data-col="topology">
      <div class="s0-overlay-col-head" title="Rubber-sheet height fields that go flat in stillness and deform as physical structure develops.">
        <span class="s0-overlay-col-label">Topology</span>
        <span class="s0-overlay-col-count" data-count-for="topology" aria-hidden="true">0</span>
        <button class="s0-overlay-col-clear u-no-baseline" data-clear-col="topology" type="button"
            title="Turn off every topology overlay (rubber sheets have non-trivial perf cost — useful for a quick reset)">&#10005;</button>
      </div>
      <div class="s0-overlay-group">
      <button class="view-toggle field-toggle" id="toggle-grav-potential"
          title="[SELECTION — visualization] Finite gravity scalar. When the native latency-Poisson term is active, this shows its clamped well potential Φ_L=−L² from the engine's 18-point finite Poisson solve. Otherwise it shows Φ_local=−G_N|J|, whose matching radius-2 difference gives the default selected gravity field exactly. The rubber-sheet interpolation and box blur are presentation only, not primitive continuum geometry. The y slider selects a thin lattice slab.">
        <span class="field-swatch field-swatch-grav-potential"></span>&Phi; potential
      </button>
      <div class="s0-sheet-height-row" data-sheet-height="gravPotential">
        <span class="s0-sheet-height-cap" title="Slice height — slide to read the field at different levels (y)">y</span>
        <input type="range" class="pe-slider s0-sheet-height-slider" id="sheet-height-grav-potential"
            min="0" max="0.999" step="0.01" value="0.50" aria-label="Φ potential slice height" />
        <span class="pe-ctrl-value s0-sheet-height-val" id="sheet-height-grav-potential-val">0.50</span>
      </div>
      </div>
      <div class="s0-overlay-group">
      <button class="view-toggle field-toggle" id="toggle-charge-density"
          title="[SELECTION] Charge density ρ(x) = ∇·J. The correction targets a coupled, mean-neutralized source and normally acts only on vacuum sites. This raw divergence is not that solver residual or a derivation of charge conservation. Same ∇·J buffer as the Volume column's ∇·J overlay, rendered here as a signed rubber sheet. The y slider slides the sheet up/down and samples ρ in a thin slab at that height.">
        <span class="field-swatch field-swatch-charge"></span>Charge &rho;
      </button>
      <div class="s0-sheet-height-row" data-sheet-height="chargeDensity">
        <span class="s0-sheet-height-cap" title="Slice height — slide to read the field at different levels (y)">y</span>
        <input type="range" class="pe-slider s0-sheet-height-slider" id="sheet-height-charge-density"
            min="0" max="0.999" step="0.01" value="0.62" aria-label="Charge density slice height" />
        <span class="pe-ctrl-value s0-sheet-height-val" id="sheet-height-charge-density-val">0.62</span>
      </div>
      </div>
      <button class="view-toggle field-toggle" id="toggle-latency"
          title="[PROXY] Normalized flux density L(x)=√(|J|²/|J|²_max); |J|²_max is the engine's own current-tick global peak, so L arrives already saturated toward its local max whenever any flux exists anywhere. This overlay peak-holds that ratio's own maximum with slow decay instead of an instantaneous per-frame max, so a scene that goes fully quiet fades instead of snapping back to a saturated core — though a field that stays peaked while decaying in absolute terms can still under-report the decline, since the ratio itself is re-normalized upstream every tick. This is distinct from the default-off, Poisson-derived [IMPOSED] engine latency mapping that contributes to the engine's mapped proper-time budget; neither readout is a recovered physical metric or clock measurement.">
        <span class="field-swatch field-swatch-latency"></span>Latency L
      </button>
      <button class="view-toggle field-toggle" id="toggle-gauss-residual"
          title="[PROXY residual] Raw r(x)=∇·J−s. The engine audit instead uses a coupled, mean-neutralized source and normally vacuum-only correction. This display is not the solver residual or a charge-conservation violation. Red = positive, blue = negative residual.">
        <span class="field-swatch field-swatch-gauss"></span>Gauss resid.
      </button>
      <button class="view-toggle field-toggle" id="toggle-proper-time"
          title="[MEASURED, IMPOSED clock axiom] Accumulated proper time τ(x)=Σ√max(1−u²/C_SPEED²−L²,0), the FTD-0402 causal-budget contract integrated per manifested voxel by accumulate_proper_time. Accumulation requires latency_field or de_broglie_clock; disabling them does not erase previously retained values. Not a substrate derivation of physical proper time.">
        <span class="field-swatch field-swatch-proper-time"></span>Proper time &tau;
      </button>
      <button class="view-toggle field-toggle" id="toggle-lapse"
          title="[MEASURED, IMPOSED clock axiom] Instantaneous lapse dτ/dt(x)=√max(1−u²/C_SPEED²−L²,0) — causal_kinematics.h's proper_time_rate evaluated live from the voxel's latency and velocity. Red=zero mapped rate (selected budget saturated), green=full mapped rate. This does not identify a physical horizon. Distinct from accumulated τ at left.">
        <span class="field-swatch field-swatch-lapse"></span>Lapse d&tau;/dt
      </button>
      <button class="view-toggle field-toggle" id="toggle-db-phase"
          title="[IMPOSED clock axiom, FTD-0271] de Broglie internal clock phase φ(x), advanced as dφ=ω₀·dτ only while the de_broglie_clock toggle is ON. Wrapped to [0,2π) and rendered as a cyclic hue wheel (the winding clock hand), not a one-directional ramp. ω₀ is IMPOSED and tied to K_B; this is implementation telemetry, not evidence of physical covariance.">
        <span class="field-swatch field-swatch-db-phase"></span>dB phase &phi;
      </button>
    </div>

    <div class="s0-overlay-col" data-col="stress-energy">
      <div class="s0-overlay-col-head" title="Energy channels and flow derived from the active lattice fields. These views do not establish a fluid velocity, gas pressure, or the full lattice Hamiltonian.">
        <span class="s0-overlay-col-label">Field energy &amp; flow</span>
        <span class="s0-overlay-col-count" data-count-for="stress-energy" aria-hidden="true">0</span>
        <button class="s0-overlay-col-clear u-no-baseline" data-clear-col="stress-energy" type="button"
            title="Turn off every field energy and flow overlay">&#10005;</button>
      </div>
      <div class="s0-overlay-group">
      <button class="view-toggle field-toggle" id="toggle-em-energy" data-search="fluid heatmap gas"
          title="Field energy density = ½|E|² + (c²/2)|B|², with E=−∂J/∂t, B=∇×J and c=C_SPEED. This combines the sampled electric and magnetic channels; it is not gas pressure or the full lattice Hamiltonian. The y slider selects a thin slab in Surface mode; heat map and volume interpolation are presentation only.">
        <span class="field-swatch field-swatch-em-energy"></span>Field energy
      </button>
      <div class="s0-sheet-height-row" data-sheet-height="emEnergy">
        <span class="s0-sheet-height-cap" title="Slice height — slide to read the field at different levels (y)">y</span>
        <input type="range" class="pe-slider s0-sheet-height-slider" id="sheet-height-em-energy"
            min="0" max="0.999" step="0.01" value="0.56" aria-label="Field energy slice height" />
        <span class="pe-ctrl-value s0-sheet-height-val" id="sheet-height-em-energy-val">0.56</span>
      </div>
      </div>
      <button class="view-toggle field-toggle" id="toggle-poynting" data-search="fluid poynting"
          title="[DERIVED from E and B; electromagnetic reading SELECTION] Poynting vector S = C_SPEED²(E × B), using the reference field-energy convention. E=−∂J/∂t omits the electrostatic near-field contribution. This is field energy flow, not a fluid velocity or the full lattice Hamiltonian current.">
        <span class="field-swatch field-swatch-energy"></span>Energy flow S
      </button>
      <div class="s0-overlay-group">
      <button class="view-toggle field-toggle" id="toggle-vorticity" data-search="fluid vorticity curl"
          title="Flux curl magnitude |∇×J| measures spatial curl of the active flux field J. It is not curl of a fluid velocity and does not establish gas vorticity or viscosity. The y slider selects a thin slab in Surface mode; the display uses relative normalization.">
        <span class="field-swatch field-swatch-vorticity"></span>Flux curl |&nabla;&times;J|
      </button>
      <div class="s0-sheet-height-row" data-sheet-height="vorticity">
        <span class="s0-sheet-height-cap" title="Slice height — slide to read the field at different levels (y)">y</span>
        <input type="range" class="pe-slider s0-sheet-height-slider" id="sheet-height-vorticity"
            min="0" max="0.999" step="0.01" value="0.68" aria-label="Flux curl slice height" />
        <span class="pe-ctrl-value s0-sheet-height-val" id="sheet-height-vorticity-val">0.68</span>
      </div>
      </div>
      <div class="s0-overlay-group">
      <button class="view-toggle field-toggle" id="toggle-e-pressure" data-search="fluid pressure energy"
          title="Electric-channel energy density = ½|E|², E=−∂J/∂t. This is the wave-kinetic channel; the electrostatic near-field is not included. It measures field energy, not gas pressure or the full lattice Hamiltonian. The y slider selects a thin slab in Surface mode.">
        <span class="field-swatch field-swatch-e-pressure"></span>Electric energy
      </button>
      <div class="s0-sheet-height-row" data-sheet-height="ePressure">
        <span class="s0-sheet-height-cap" title="Slice height — slide to read the field at different levels (y)">y</span>
        <input type="range" class="pe-slider s0-sheet-height-slider" id="sheet-height-e-pressure"
            min="0" max="0.999" step="0.01" value="0.44" aria-label="Electric energy slice height" />
        <span class="pe-ctrl-value s0-sheet-height-val" id="sheet-height-e-pressure-val">0.44</span>
      </div>
      </div>
      <div class="s0-overlay-group">
      <button class="view-toggle field-toggle" id="toggle-b-pressure" data-search="fluid pressure energy"
          title="Magnetic-channel energy density = (c²/2)|B|², c=C_SPEED and B=∇×J. The c² factor follows the reference field-energy convention. This is energy in field curl, not gas pressure or the full lattice Hamiltonian. The y slider selects a thin slab in Surface mode.">
        <span class="field-swatch field-swatch-b-pressure"></span>Magnetic energy
      </button>
      <div class="s0-sheet-height-row" data-sheet-height="bPressure">
        <span class="s0-sheet-height-cap" title="Slice height — slide to read the field at different levels (y)">y</span>
        <input type="range" class="pe-slider s0-sheet-height-slider" id="sheet-height-b-pressure"
            min="0" max="0.999" step="0.01" value="0.38" aria-label="Magnetic energy slice height" />
        <span class="pe-ctrl-value s0-sheet-height-val" id="sheet-height-b-pressure-val">0.38</span>
      </div>
      </div>
    </div>

    <div class="s0-overlay-col" data-col="phenomena">
      <div class="s0-overlay-col-head">
        <span class="s0-overlay-col-label">Phenomena</span>
        <span class="s0-overlay-col-count" data-count-for="phenomena" aria-hidden="true">0</span>
        <button class="s0-overlay-col-clear u-no-baseline" data-clear-col="phenomena" type="button"
            title="Turn off every phenomena overlay">&#10005;</button>
      </div>
      <button class="view-toggle field-toggle" id="toggle-dual-substrate"
          title="[PROXY] Amplitude split J_L=J·(1+δ)/2, J_R=J·(1−δ)/2, δ=DUAL_DELTA. Both halves are COLLINEAR with J — they carry no independent direction — so this is an amplitude-asymmetry demonstration, NOT the substrate's left/right decomposition. The engine's real per-voxel flux_L/flux_R exist under the dual_substrate toggle but are not sampled here.">
        <span class="field-swatch field-swatch-dualj"></span>Dual J
      </button>
      <button class="view-toggle field-toggle" id="toggle-chirality"
          title="[PROXY] Chiral amplitude |J|·δ, δ=DUAL_DELTA — a NON-NEGATIVE magnitude, so it cannot show which handedness dominates. The signed handedness diagnostic (per-voxel chirality density) is not sampled to JS here.">
        <span class="field-swatch field-swatch-chirality"></span>Chiral amplitude
      </button>
      <button class="view-toggle field-toggle" id="toggle-dark-halo"
          title="[PROXY] Sub-threshold flux envelope: voxels with 0.003 &lt; |J| &lt; K_GENESIS. Pedagogical analogue for un-manifested flux. Unrelated to the 17/27 Moore-shell figure, which is a separate [SELECTION]-tagged dark-state count, not a derivation and not a density prediction.">
        <span class="field-swatch field-swatch-dm-halo"></span>DM Halo
      </button>
      <button class="view-toggle field-toggle" id="toggle-genesis-iso"
          title="[SELECTION] Genesis frontier: the shell |J|≈K_GENESIS. This is the BOUNDARY of the region where manifestation is possible, not where it happens — genesis fires stochastically INSIDE the shell with probability rising with excess flux, so the rate is ~0 on the shell itself.">
        <span class="field-swatch field-swatch-genesis"></span>Genesis
      </button>
      <button class="view-toggle field-toggle" id="toggle-color-charge"
          title="Color particles by their real genesis-assigned color charge (argmax|J_axis| in {red,green,blue}) instead of charge sign. FTD 'color' is a C3-symmetric discrete axis label, NOT SU(3) gauge charge (LEDGER FTD-0077).">
        <span class="field-swatch field-swatch-color-charge"></span>Color charge
      </button>
      <button class="view-toggle field-toggle" id="toggle-damping-zones"
          title="[PROXY] Bounding-box glyph around each particle: a 3×3×3 box encloses the 7-cell von Neumann damping footprint (center plus 6 face neighbors). The box does not mark all enclosed cells as damped; it is not an octahedral cell mask. At most 100 particle boxes are shown.">
        <span class="field-swatch field-swatch-damping"></span>Damping
      </button>
      <button class="view-toggle field-toggle" id="toggle-confinement"
          title="[PROXY] Pair-proximity glyphs: a line between every particle pair whose separation r satisfies 1&lt;r&lt;√120≈10.95 voxels — a fixed voxel distance that does NOT scale with lattice size, and which excludes face-adjacent pairs at r=1. Line colour encodes separation direction, not colour charge. The epistemic disclaimer about confinement (LEDGER FTD-0025) still applies — this is not a Wilson-loop measurement.">
        <span class="field-swatch field-swatch-confinement"></span>Confinement
      </button>
      <button class="view-toggle field-toggle" id="toggle-horizon"
          title="[PROXY] Voxels where the latency proxy L≥0.95. L is peak-hold normalized against a recent (not instantaneous) maximum, so this marks flux within ~90% of a recent peak — a RELATIVE marker, not an absolute gravitational threshold, and not evidence anything is actually trapped.">
        <span class="field-swatch field-swatch-horizon"></span>Horizon
      </button>
    </div>
    </div>
  `;
  // Keep every layer row visually consistent without hand-wrapping dozens of
  // semantic labels. The wrapper preserves each existing swatch/icon and lets
  // CSS center and subtly reduce the content while retaining the full button
  // hit target and the project-wide 16 px computed font floor.
  for (const button of container.querySelectorAll('.s0-overlay-body .view-toggle')) {
    const content = document.createElement('span');
    content.className = 's0-toggle-content';
    while (button.firstChild) content.appendChild(button.firstChild);
    button.appendChild(content);
  }
  return container;
}
