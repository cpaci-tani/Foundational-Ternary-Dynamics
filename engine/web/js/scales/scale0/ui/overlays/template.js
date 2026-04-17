/**
 * Scale 0 Viewport Overlay — Field visualization controls
 *
 * Toggles are organised into four semantic columns so the panel stays
 * scannable as the feature set grows:
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

export function createScale0OverlayTemplate() {
  const container = document.createElement('div');
  container.id = 'viewport-overlay';
  container.className = 'scale0-only s0-overlay-panel';
  container.innerHTML = `
    <header class="s0-overlay-header">
      <span class="s0-overlay-title">Visualization</span>
      <button class="s0-overlay-collapse" type="button"
          aria-label="Collapse visualization overlay"
          aria-expanded="true"
          title="Collapse overlay">
        <span class="s0-overlay-collapse-icon" aria-hidden="true">&#9652;</span>
      </button>
    </header>
    <div class="s0-overlay-body">
    <div class="s0-overlay-col">
      <div class="s0-overlay-col-head">Volume</div>
      <button class="view-toggle active" id="toggle-flux-volume"
          title="Volumetric point cloud of |J| across the whole lattice">Flux Volume</button>
      <button class="view-toggle" id="toggle-flux-slice"
          title="2D slice through the flux field (XZ plane)">Flux Slice</button>
      <button class="view-toggle field-toggle" id="toggle-flux-lines"
          title="Streamlines of the J-field showing flow direction">
        <span class="field-swatch field-swatch-flux-lines"></span>Flux Lines
      </button>
      <button class="view-toggle field-toggle" id="toggle-div-field"
          title="Divergence of J (charge sources and sinks)">
        <span class="field-swatch field-swatch-divj"></span>&nabla;&middot;J
      </button>
    </div>

    <div class="s0-overlay-col">
      <div class="s0-overlay-col-head">Fields</div>
      <button class="view-toggle field-toggle" id="toggle-e-field"
          title="Electric field streamlines (E = -∂J/∂t)">
        <span class="field-swatch field-swatch-e-field"></span>E Field
      </button>
      <button class="view-toggle field-toggle" id="toggle-b-field"
          title="Magnetic field streamlines (B = curl J)">
        <span class="field-swatch field-swatch-b-field"></span>B Field
      </button>
      <button class="view-toggle field-toggle" id="toggle-poynting"
          title="Poynting vector S = E × B (energy flux direction)">
        <span class="field-swatch field-swatch-energy"></span>Poynting S
      </button>
      <button class="view-toggle field-toggle" id="toggle-light"
          title="Photon glow — Poynting magnitude |S| rendered as bloom">
        <span class="field-swatch field-swatch-light"></span>Light
      </button>
    </div>

    <div class="s0-overlay-col">
      <div class="s0-overlay-col-head">Forces</div>
      <div class="force-style-row" id="force-style-row"
          title="Render style for force-field overlays">
        <button class="style-btn active" data-style="arrows" title="Vector arrows">Arrows</button>
        <button class="style-btn" data-style="heatmap" title="Gaussian heatmap">Heatmap</button>
        <button class="style-btn" data-style="flow" title="Animated streamlines">Flow</button>
        <button class="style-btn" data-style="glyphs" title="Oriented glyph field">Glyphs</button>
      </div>
      <button class="view-toggle field-toggle" id="toggle-force-em"
          title="Electromagnetic force vectors (Coulomb + Lorentz)">
        <span class="field-swatch field-swatch-em"></span>EM
      </button>
      <button class="view-toggle field-toggle" id="toggle-force-gravity"
          title="Gravitational force from density gradient (attractive)">
        <span class="field-swatch field-swatch-gravity"></span>Gravity
      </button>
      <button class="view-toggle field-toggle" id="toggle-force-strong"
          title="Strong / color force vectors (SU(3) confinement + color)">
        <span class="field-swatch field-swatch-strong"></span>Strong
      </button>
      <button class="view-toggle field-toggle" id="toggle-force-weak"
          title="Weak transmutation sites (chirality-dependent flavor change)">
        <span class="field-swatch field-swatch-weak"></span>Weak
      </button>
    </div>

    <div class="s0-overlay-col">
      <div class="s0-overlay-col-head">Quantum</div>
      <button class="view-toggle field-toggle" id="toggle-psi-squared"
          title="Born probability density |ψ|² = |J_L|² + |J_R|² (or |J|² if dual substrate off). Where the particle is, probabilistically.">
        <span class="field-swatch field-swatch-psi-squared"></span>|&psi;|&sup2;
      </button>
      <button class="view-toggle field-toggle" id="toggle-phase"
          title="Complex phase φ = arg(J_L + i·J_R). Interference fringes, Aharonov-Bohm loops. Best with Dual Substrate on.">
        <span class="field-swatch field-swatch-phase"></span>Phase &phi;
      </button>
      <button class="view-toggle field-toggle" id="toggle-lagrangian-density"
          title="Lagrangian density ℒ(x) = ½|∂ₜJ|² − ½|∇J|² − V(s,J). Blue = potential-dominated, red = kinetic-dominated.">
        <span class="field-swatch field-swatch-lagrangian"></span>&#8466;(x)
      </button>
      <button class="view-toggle field-toggle" id="toggle-entropy-density"
          title="Local Shannon entropy of state distribution in a 3×3×3 Moore neighborhood. White = disordered, black = crystallized.">
        <span class="field-swatch field-swatch-entropy"></span>Entropy s
      </button>
      <button class="view-toggle field-toggle" id="toggle-grav-potential"
          title="Gravitational potential Φ(x) — the wells generating Gravity force vectors. Deep blue = mass well, yellow = saddle/peak.">
        <span class="field-swatch field-swatch-grav-potential"></span>&Phi; potential
      </button>
    </div>

    <div class="s0-overlay-col">
      <div class="s0-overlay-col-head">Phenomena</div>
      <button class="view-toggle field-toggle" id="toggle-dual-substrate"
          title="Dual substrate split: J_L (warm) and J_R (cool)">
        <span class="field-swatch field-swatch-dualj"></span>Dual J
      </button>
      <button class="view-toggle field-toggle" id="toggle-chirality"
          title="Chirality field: |J_L| − |J_R| (net handedness)">
        <span class="field-swatch field-swatch-chirality"></span>Chirality
      </button>
      <button class="view-toggle field-toggle" id="toggle-dark-halo"
          title="Sub-threshold flux envelope — gravitates but does not manifest (dark matter analogue)">
        <span class="field-swatch field-swatch-dm-halo"></span>DM Halo
      </button>
      <button class="view-toggle field-toggle" id="toggle-genesis-iso"
          title="Isosurface at |J| = K_GENESIS — particles crystallize along this shell">
        <span class="field-swatch field-swatch-genesis"></span>Genesis
      </button>
      <button class="view-toggle field-toggle" id="toggle-damping-zones"
          title="Selective damping zones — 1-hop radii around particles where energy dissipates">
        <span class="field-swatch field-swatch-damping"></span>Damping
      </button>
      <button class="view-toggle field-toggle" id="toggle-confinement"
          title="SU(3) confinement — flux strings connecting color-charged pairs">
        <span class="field-swatch field-swatch-confinement"></span>Confinement
      </button>
    </div>
    </div>
  `;
  return container;
}
