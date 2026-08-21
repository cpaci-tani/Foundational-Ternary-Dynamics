/**
 * Scale 0 Viewport Overlay — Field visualization controls
 *
 * A dense 2-up chip grid: a filter box + an active-overlays strip on top, then the
 * toggles grouped into semantic categories (collapsible per-category, expanded by
 * default so every applicable overlay is visible without clicking through headers —
 * the collapse / filter / active-strip behaviour lives in overlays/panel-shell.js;
 * the toggles themselves are wired in scale0/ui/bindings.js). Each category renders
 * its buttons in a 2-column grid (labels wrap instead of truncating, so nothing is
 * ever cut off); a button that owns its own full-width sub-row (e.g. Flux Volume's
 * Organic/Glow style row, Flux Slice's xy/xz/yz plane row) is wrapped together with
 * that sub-row in a `.s0-overlay-group` so the pair occupies one grid cell instead of
 * leaving an empty half-row next to its own trigger. The categories:
 *
 *   VOLUME    — how the raw flux field is rendered (volume, slice, lines, ∇·J)
 *   FIELDS    — EM-derived vector fields (E, B, Poynting arrows, Poynting glow)
 *   FORCES    — per-particle force vectors, with a render-style selector
 *               (Arrows / Heatmap / Flow / Glyphs) at the top of the column
 *   PHENOMENA — emergent / composite overlays (chirality, DM halo, confinement, …)
 *
 * The same physical quantity may appear twice if the styles are distinct
 * (e.g. Poynting S arrows vs. Light = |S| bloom). Labels come with tooltips.
 */

export function getScale0OverlayTemplate() {
  const container = document.createElement('div');
  container.id = 'viewport-overlay';
  container.className = 'scale0-only s0-overlay-panel';
  container.innerHTML = `
    <header class="s0-overlay-header">
      <span class="s0-overlay-title">Visualization</span>
      <div class="s0-overlay-header-tools">
        <button class="s0-overlay-collapse u-no-baseline" type="button"
            aria-label="Collapse visualization overlay"
            aria-expanded="true"
            title="Collapse overlay">
          <span class="s0-overlay-collapse-icon" aria-hidden="true">&#9652;</span>
        </button>
      </div>
    </header>
    <div class="s0-overlay-search">
      <input type="search" id="s0-overlay-search" class="s0-overlay-search-input"
          placeholder="Filter overlays…" autocomplete="off" spellcheck="false"
          aria-label="Filter visualization overlays" />
    </div>
    <div class="s0-overlay-active" id="s0-overlay-active" aria-label="Active overlays" hidden></div>
    <div class="s0-overlay-body">
    <div class="s0-overlay-col" data-col="volume">
      <div class="s0-overlay-col-head">
        <span class="s0-overlay-col-label">Volume</span>
        <span class="s0-overlay-col-count" data-count-for="volume" aria-hidden="true">0</span>
        <button class="s0-overlay-col-clear u-no-baseline" data-clear-col="volume" type="button"
            title="Turn off every overlay in this column">&#10005;</button>
      </div>
      <div class="s0-overlay-group">
        <button class="view-toggle active" id="toggle-flux-volume"
            title="Volumetric point cloud of |J| (flux magnitude). Every voxel up to L=53; above that an even-stride subsample. Voxels below the |J| threshold slider are hidden.">Flux Volume</button>
        <div class="flux-slice-axis-row" role="group" aria-label="Flux volume style">
          <button class="view-toggle flux-slice-axis-mini active" id="toggle-flux-organic"
              title="Organic scatter jitter (±½ cell), independent of lattice size.">Organic</button>
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
          title="[SELECTION] Divergence ∇·J. The engine's Gauss projection targets ∇·J≈s each tick, so this reads as charge sources (red) and sinks (blue) BY CONSTRUCTION — a selected polarity-to-charge map, not a derivation of charge conservation (see LEDGER FTD-0421/FTD-0426). Same buffer as Charge ρ (Topology column), rendered here as points instead of a rubber sheet.">
        <span class="field-swatch field-swatch-divj"></span>&nabla;&middot;J
      </button>
      <button class="view-toggle field-toggle" id="toggle-state-field"
          title="[AXIOM] Ternary state field s ∈ {−1,0,+1} — the manifestation layer (Postulate 3: J is primary, s is its threshold projection). Manifested voxels render as points (s=−1 blue, s=+1 red); void (s=0) is invisible. Which voxels light up is set by the genesis threshold K_GENESIS (a calibrated value) plus stochastic evaporation.">
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
      <button class="view-toggle field-toggle" id="toggle-poynting"
          title="[DERIVED from E and B; electromagnetic reading SELECTION] Poynting vector S = E × B (energy flux direction). Inherits E's limitation above — the electrostatic near-field contribution is absent, so S is understated wherever a static charge dominates.">
        <span class="field-swatch field-swatch-energy"></span>Poynting S
      </button>
    </div>

    <div class="s0-overlay-col" data-col="forces">
      <div class="s0-overlay-col-head">
        <span class="s0-overlay-col-label">Forces</span>
        <span class="s0-overlay-col-count" data-count-for="forces" aria-hidden="true">0</span>
        <button class="s0-overlay-col-clear u-no-baseline" data-clear-col="forces" type="button"
            title="Turn off every force overlay (preserves the current style selection)">&#10005;</button>
      </div>
      <div class="force-style-row" id="force-style-row"
          title="Render style for force-field overlays">
        <button class="style-btn active" data-style="arrows" title="Vector arrows">Arrows</button>
        <button class="style-btn" data-style="heatmap" title="Gaussian heatmap">Heatmap</button>
        <button class="style-btn" data-style="flow" title="Animated streamlines">Flow</button>
        <button class="style-btn" data-style="glyphs" title="Oriented glyph field">Glyphs</button>
      </div>
      <button class="view-toggle field-toggle" id="toggle-force-em"
          title="[PARAMETRIC] Electrostatic force on a unit test charge: F = (α/4π)·Σ_p s_p·(r-r_p)/(|r-r_p|²+1)^1.5, summed over manifested voxels with periodic minimum-image and 1-voxel softening. Textbook continuum Coulomb law with FTD's α inserted — NOT the lattice Green's function of Phase G, and NOT the force the tick loop actually applies. No Lorentz v×B term is included, even when the lorentz_force toggle is on.">
        <span class="field-swatch field-swatch-em"></span>EM
      </button>
      <button class="view-toggle field-toggle" id="toggle-force-gravity"
          title="[SELECTION — visualization] Density-gradient attraction G_N·∇|J|, with G_N the constant whose identification with physical G was falsified (LEDGER FTD-0131); arrows point up the flux-density gradient. This heuristic is not FTD's substrate→Newton chain of record — that chain is [DERIVED] conditional on the clock-hypothesis axiom and outputs α_G=(m_e/m_P)², not this field.">
        <span class="field-swatch field-swatch-gravity"></span>Gravity
      </button>
      <button class="view-toggle field-toggle" id="toggle-force-strong"
          title="[SELECTION — visualization] Pairwise flux-tube force between all manifested voxels inside a Gaussian tube envelope: Coulomb for r<3, transition for 3≤r<8, harmonic F∝r (V∝r²) for r≥8 — the harmonic long-range arm is NOT the linear potential of area-law confinement. Confinement of record is [THEOREM-within-compact-U(1)-LGT; SELECTION at FTD-substrate level] (LEDGER FTD-0025).">
        <span class="field-swatch field-swatch-strong"></span>Strong
      </button>
      <button class="view-toggle field-toggle" id="toggle-force-weak"
          title="[PROXY — VISUALIZATION ONLY] The curl ∇×J (a parity-even (axial) pseudovector) rendered as arrows, scaled by DUAL_DELTA ≈ 0.957. This is a vector-calculus view of J, NOT the SM weak force — FTD's weak interaction is state transmutation (weak_transmutation toggle). Companion to Vorticity |∇×J| (Topology). Lives in this column only to share the force-style selector. (audit P1-17, 2026-05-27)">
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
          title="[PROXY] Flux energy density |J|², max-normalized — the conserved quadratic density of the discrete wave equation. Computed as |J|² unconditionally; the Dual Substrate toggle does not change this overlay's math. Displayed as a Born-style |ψ|² analogue, but FTD does NOT derive probability from it: the energy-density-to-probability step is [OPEN] (LEDGER FTD-0187) and measured threshold-crossing Born scaling came back [CLOSED NEGATIVE] (FTD-0200).">
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
      <button class="view-toggle field-toggle" id="toggle-grav-potential"
          title="[PROXY] Gravitational potential stand-in — inverted flux density −|J|², box-blurred at render time. No bridge solves a real Φ. A true Φ would obey ∇²Φ=4πGρ and carry a long 1/r tail; this one is purely local, so wells do not add up at a distance. Height is the y-column average at each (x,z), not Φ at a point.">
        <span class="field-swatch field-swatch-grav-potential"></span>&Phi; potential
      </button>
      <button class="view-toggle field-toggle" id="toggle-em-energy"
          title="EM energy density u(x) = ½|E|² + (c²/2)|B|² (C_SPEED weights the magnetic channel — see diagnostics_compute.cpp). E and B are read at their true shared positions (the engine compacts each field's sparse samples independently, so raw loop index does not pair them); peaks where EM fields concentrate, flat in vacuum. Height is peak-hold normalized (see decaying-max).">
        <span class="field-swatch field-swatch-em-energy"></span>EM energy u
      </button>
      <button class="view-toggle field-toggle" id="toggle-charge-density"
          title="[SELECTION] Charge density ρ(x) = ∇·J. The engine's Gauss projection targets ∇·J≈s each tick, so this reads as sources (red hills) and sinks (blue wells) BY CONSTRUCTION — a selected polarity-to-charge map, not a derivation of charge conservation (see LEDGER FTD-0421/FTD-0426). Same ∇·J buffer as the Volume column's ∇·J overlay, rendered here as a signed rubber sheet instead of points.">
        <span class="field-swatch field-swatch-charge"></span>Charge &rho;
      </button>
      <button class="view-toggle field-toggle" id="toggle-vorticity"
          title="Vorticity |ω|(x)=|∇×J| — the swirl magnitude of the flux field. Rises on vortex rings and any circulating flux structure; a purely radial or uniform flow has zero curl and produces no sheet at all. Height is peak-hold normalized (see decaying-max), so read relative structure, not an absolute scale.">
        <span class="field-swatch field-swatch-vorticity"></span>Vorticity &omega;
      </button>
      <button class="view-toggle field-toggle" id="toggle-latency"
          title="[PROXY] Normalized flux density L(x)=√(|J|²/|J|²_max); |J|²_max is the engine's own current-tick global peak, so L arrives already saturated toward its local max whenever any flux exists anywhere. This overlay peak-holds that ratio's own maximum with slow decay instead of an instantaneous per-frame max, so a scene that goes fully quiet fades instead of snapping back to a saturated core — though a field that stays peaked while decaying in absolute terms can still under-report the decline, since the ratio itself is re-normalized upstream every tick. This is a stand-in, NOT the engine's real latency field — that comes from a default-off Poisson solve and actually sets the proper-time budget.">
        <span class="field-swatch field-swatch-latency"></span>Latency L
      </button>
      <button class="view-toggle field-toggle" id="toggle-gauss-residual"
          title="[MEASURED residual of a SELECTION constraint] Gauss-constraint residual r(x)=∇·J−s. div(J)=s is a selected step (SPEC_ENGINE.md), not a theorem, so this measures the miss of that constraint — it is not a charge-conservation violation (see LEDGER FTD-0421/FTD-0426). Red = positive, blue = negative residual.">
        <span class="field-swatch field-swatch-gauss"></span>Gauss resid.
      </button>
    </div>

    <div class="s0-overlay-col" data-col="stress-energy">
      <div class="s0-overlay-col-head" title="Stress-energy components and information-theoretic fields derived from J.">
        <span class="s0-overlay-col-label">Stress-Energy</span>
        <span class="s0-overlay-col-count" data-count-for="stress-energy" aria-hidden="true">0</span>
        <button class="s0-overlay-col-clear u-no-baseline" data-clear-col="stress-energy" type="button"
            title="Turn off every stress-energy overlay">&#10005;</button>
      </div>
      <button class="view-toggle field-toggle" id="toggle-e-pressure"
          title="Electric-channel energy density P_E(x) = ½|E|², E=−∂J/∂t. This is the substrate's wave-KINETIC channel — it peaks on fast-changing flux and falls to ~0 in a settled configuration, even where charge sits. The Poisson-solved electrostatic potential φ_C is a separate field this overlay does not read.">
        <span class="field-swatch field-swatch-e-pressure"></span>P<sub>E</sub> (electric)
      </button>
      <button class="view-toggle field-toggle" id="toggle-b-pressure"
          title="Magnetic-channel energy density P_B(x) = (c²/2)|B|², c=C_SPEED, B=∇×J — this c² factor matches the engine's own Hamiltonian convention (without it P_B was 3x too large and not magnitude-comparable with P_E). Rises where the flux field has spatial curl (shear or twist).">
        <span class="field-swatch field-swatch-b-pressure"></span>P<sub>B</sub> (magnetic)
      </button>
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
        <span class="field-swatch field-swatch-chirality"></span>Chirality
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
          title="Selective damping zones — an octahedral (cross-shaped) wireframe around each particle tracing its true von Neumann 1-hop footprint: the manifested voxel plus its 6 face-adjacent neighbors (7 cells total) where energy actually dissipates (engine: compute_near_particle_mask, neighbors_6). Replaces a prior full 3×3×3 Moore-cube outline that over-drew the damped region 4x (27 cells implied vs 7 actually damped).">
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
  return container;
}
