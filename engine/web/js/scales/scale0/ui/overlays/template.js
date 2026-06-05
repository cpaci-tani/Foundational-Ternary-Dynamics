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

export function getScale0OverlayTemplate() {
  const container = document.createElement('div');
  container.id = 'viewport-overlay';
  container.className = 'scale0-only s0-overlay-panel';
  container.innerHTML = `
    <header class="s0-overlay-header">
      <span class="s0-overlay-title">Visualization</span>
      <div class="s0-overlay-header-tools">
        <button class="s0-overlay-collapse" type="button"
            aria-label="Collapse visualization overlay"
            aria-expanded="true"
            title="Collapse overlay">
          <span class="s0-overlay-collapse-icon" aria-hidden="true">&#9652;</span>
        </button>
      </div>
    </header>
    <div class="s0-overlay-body">
    <div class="s0-overlay-col" data-col="volume">
      <div class="s0-overlay-col-head">
        <span class="s0-overlay-col-label">Volume</span>
        <span class="s0-overlay-col-count" data-count-for="volume" aria-hidden="true">0</span>
        <button class="s0-overlay-col-clear" data-clear-col="volume" type="button"
            title="Turn off every overlay in this column">&#10005;</button>
      </div>
      <button class="view-toggle active" id="toggle-flux-volume"
          title="Volumetric point cloud of |J| across the whole lattice">Flux Volume</button>
      <div class="flux-slice-axis-row" role="group" aria-label="Flux volume style">
        <button class="view-toggle flux-slice-axis-mini active" id="toggle-flux-organic"
            title="Organic scatter (cloud) vs regular lattice grid — applies at L>53">Organic</button>
        <button class="view-toggle flux-slice-axis-mini active" id="toggle-flux-glow"
            title="Additive glow bloom on the flux volume">Glow</button>
      </div>
      <button class="view-toggle" id="toggle-flux-slice"
          title="2D slice through the flux field — xy, xz, yz mid-planes">Flux Slice</button>
      <div class="flux-slice-axis-row" role="group" aria-label="Flux slice planes">
        <button class="view-toggle flux-slice-axis-mini active" id="flux-slice-axis-xy"
            title="Toggle the xy mid-plane (z = L/2)">xy</button>
        <button class="view-toggle flux-slice-axis-mini active" id="flux-slice-axis-xz"
            title="Toggle the xz mid-plane (y = L/2)">xz</button>
        <button class="view-toggle flux-slice-axis-mini active" id="flux-slice-axis-yz"
            title="Toggle the yz mid-plane (x = L/2)">yz</button>
      </div>
      <button class="view-toggle field-toggle" id="toggle-flux-lines"
          title="Streamlines of the J-field showing flow direction">
        <span class="field-swatch field-swatch-flux-lines"></span>Flux Lines
      </button>
      <button class="view-toggle field-toggle" id="toggle-div-field"
          title="Divergence of J (charge sources and sinks). Same quantity as Charge ρ (Topology column) — rendered here as a point cloud, there as a signed rubber sheet.">
        <span class="field-swatch field-swatch-divj"></span>&nabla;&middot;J
      </button>
      <button class="view-toggle field-toggle" id="toggle-state-field"
          title="[AXIOM] Ternary state field s ∈ {−1,0,+1} — the literal FTD manifestation layer (Postulate 3). Manifested voxels render as points (s=−1 blue, s=+1 red); the void (s=0) is the invisible background. The substrate's actual matter content, normally only seen indirectly via particles.">
        <span class="field-swatch field-swatch-state"></span>State s
      </button>
      <button class="view-toggle field-toggle" id="toggle-moore-decomp"
          title="[THEOREM] Moore-neighbourhood decomposition (Moore Layer Theorem). The 26 neighbours split into three polyhedral shells: SC octahedron (6 faces, red) + FCC cuboctahedron (12 edges, green) + BCC stella octangula (8 corners = two tetrahedra, blue). A static structural glyph centred on the lattice — the geometric heart of FTD, not a sampled field.">
        <span class="field-swatch field-swatch-moore"></span>Moore cell
      </button>
    </div>

    <div class="s0-overlay-col" data-col="fields">
      <div class="s0-overlay-col-head">
        <span class="s0-overlay-col-label">Fields</span>
        <span class="s0-overlay-col-count" data-count-for="fields" aria-hidden="true">0</span>
        <button class="s0-overlay-col-clear" data-clear-col="fields" type="button"
            title="Turn off every overlay in this column">&#10005;</button>
      </div>
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
          title="Photon glow — Poynting magnitude |S| rendered as additive bloom. Render-variant of Poynting S (same E×B energy flux), not independent physics.">
        <span class="field-swatch field-swatch-light"></span>Light
      </button>
    </div>

    <div class="s0-overlay-col" data-col="forces">
      <div class="s0-overlay-col-head">
        <span class="s0-overlay-col-label">Forces</span>
        <span class="s0-overlay-col-count" data-count-for="forces" aria-hidden="true">0</span>
        <button class="s0-overlay-col-clear" data-clear-col="forces" type="button"
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
          title="Electromagnetic force vectors (Coulomb + Lorentz)">
        <span class="field-swatch field-swatch-em"></span>EM
      </button>
      <button class="view-toggle field-toggle" id="toggle-force-gravity"
          title="[SELECTION] Gravitational force modeled as G·∇|J| (density-gradient attraction). The substrate→gravity mechanism is NOT derived — the G_N identification was falsified (FTD-0131); shown as a pedagogical attractive field.">
        <span class="field-swatch field-swatch-gravity"></span>Gravity
      </button>
      <button class="view-toggle field-toggle" id="toggle-force-strong"
          title="[SELECTION] Strong / color force vectors. Area-law confinement is a lattice-gauge [THEOREM], but the SU(3) identification needs an independent N_c = 3 source (DERIV_NC_FROM_TOPOLOGY); shown as a confining color-force field.">
        <span class="field-swatch field-swatch-strong"></span>Strong
      </button>
      <button class="view-toggle field-toggle" id="toggle-force-weak"
          title="[PROXY — VISUALIZATION ONLY] The curl ∇×J (a parity-odd pseudovector) rendered as arrows, scaled by DUAL_DELTA ≈ 0.957. This is a vector-calculus view of J, NOT the SM weak force — FTD's weak interaction is state transmutation (weak_transmutation toggle). Companion to Vorticity |∇×J| (Topology). Lives in this column only to share the force-style selector. (audit P1-17, 2026-05-27)">
        <span class="field-swatch field-swatch-weak"></span>&nabla;&times;J pseudovector
      </button>
    </div>

    <div class="s0-overlay-col" data-col="quantum">
      <div class="s0-overlay-col-head">
        <span class="s0-overlay-col-label">Quantum</span>
        <span class="s0-overlay-col-count" data-count-for="quantum" aria-hidden="true">0</span>
        <button class="s0-overlay-col-clear" data-clear-col="quantum" type="button"
            title="Turn off every quantum overlay">&#10005;</button>
      </div>
      <button class="view-toggle field-toggle" id="toggle-psi-squared"
          title="[PROXY] Born probability density |ψ|² = |J_L|² + |J_R|² (or |J|² if dual substrate off). Where the particle is, probabilistically.">
        <span class="field-swatch field-swatch-psi-squared"></span>|&psi;|&sup2;
      </button>
      <button class="view-toggle field-toggle" id="toggle-phase"
          title="[PROXY] Complex phase φ = arg(J_L + i·J_R). Requires Dual Substrate toggle to be on (otherwise renders flat). Interference fringes, Aharonov-Bohm loops.">
        <span class="field-swatch field-swatch-phase"></span>Phase &phi;
      </button>
      <button class="view-toggle field-toggle" id="toggle-lagrangian-density"
          title="[PROXY] Lagrangian density ℒ(x) ≈ ½|∂ₜJ|² − ½(∇·J)² − V(s,J). Uses (∇·J)² as a stand-in for |∇J|² (Frobenius norm of full Jacobian not exposed).">
        <span class="field-swatch field-swatch-lagrangian"></span>&#8466;(x)
      </button>
      <button class="view-toggle field-toggle" id="toggle-entropy-density"
          title="[PROXY] Local entropy approximated as 4p(1−p) with p = |J|/|J|_max (Gini-style impurity). True neighborhood Shannon estimator lands when state-field access is exposed.">
        <span class="field-swatch field-swatch-entropy"></span>Entropy s
      </button>
    </div>

    <div class="s0-overlay-col" data-col="topology">
      <div class="s0-overlay-col-head" title="Rubber-sheet height fields that go flat in stillness and deform as physical structure develops.">
        <span class="s0-overlay-col-label">Topology</span>
        <span class="s0-overlay-col-count" data-count-for="topology" aria-hidden="true">0</span>
        <button class="s0-overlay-col-clear" data-clear-col="topology" type="button"
            title="Turn off every topology overlay (rubber sheets have non-trivial perf cost — useful for a quick reset)">&#10005;</button>
      </div>
      <button class="view-toggle field-toggle" id="toggle-grav-potential"
          title="[PROXY] Gravitational potential Φ(x) — approximated as smoothed −|J|² mass density when the bridge doesn't expose a Poisson-solved Φ directly. Deep blue = mass well, yellow = saddle/peak.">
        <span class="field-swatch field-swatch-grav-potential"></span>&Phi; potential
      </button>
      <button class="view-toggle field-toggle" id="toggle-em-energy"
          title="EM energy density u(x) = ½(|E|² + |B|²). Maxwell energy density; peaks where EM fields concentrate, flat in vacuum.">
        <span class="field-swatch field-swatch-em-energy"></span>EM energy u
      </button>
      <button class="view-toggle field-toggle" id="toggle-charge-density"
          title="Charge density ρ(x) = ∇·J. FTD-native charge via Gauss. Red hills = sources, blue wells = sinks, flat where divergence-free. Same ∇·J buffer as the Volume column's ∇·J overlay — here as a signed rubber sheet.">
        <span class="field-swatch field-swatch-charge"></span>Charge &rho;
      </button>
      <button class="view-toggle field-toggle" id="toggle-vorticity"
          title="Vorticity |ω|(x) = |∇×J|. Flux-field swirl magnitude; lights up around vortex rings and rotational solitons, flat for curl-free flow.">
        <span class="field-swatch field-swatch-vorticity"></span>Vorticity &omega;
      </button>
      <button class="view-toggle field-toggle" id="toggle-helicity"
          title="Helicity density h(x) = J·(∇×J). Signed scalar — field-line linking number density. Positive = right-handed, negative = left-handed.">
        <span class="field-swatch field-swatch-helicity"></span>Helicity h
      </button>
      <button class="view-toggle field-toggle" id="toggle-kretschmann"
          title="[PROXY] Kretschmann-like curvature K(x) = (∇²L)² with L = √(|J|²/|J|²_max) as mass-density proxy (MockBridge doesn't run the Poisson solver). Log-compressed so the horizon spike doesn't flatten the background. Spikes at event horizons.">
        <span class="field-swatch field-swatch-kretschmann"></span>Curvature K
      </button>
      <button class="view-toggle field-toggle" id="toggle-latency"
          title="[DERIVED] Latency / time-dilation field L(x) = √(|J|²/|J|²max) ∈ [0, 0.998]. The Born-Infeld proper-time field — f = 1 − L² is the local speed limit, so high-L regions are gravity wells where time slows; L spikes toward 1 at event horizons. Volumetric point cloud, blue (L≈0) → red (L→1).">
        <span class="field-swatch field-swatch-latency"></span>Latency L
      </button>
      <button class="view-toggle field-toggle" id="toggle-gauss-residual"
          title="[DERIVED] Gauss-constraint residual r(x) = ∇·J − s_charge. FTD-native charge is the ternary state, so a clean substrate has r ≈ 0; non-zero r maps the non-variational Gauss-projection conservation leak (SPEC_ENGINE.md). Red = positive, blue = negative residual.">
        <span class="field-swatch field-swatch-gauss"></span>Gauss resid.
      </button>
    </div>

    <div class="s0-overlay-col" data-col="stress-energy">
      <div class="s0-overlay-col-head" title="Stress-energy components and information-theoretic fields derived from J.">
        <span class="s0-overlay-col-label">Stress-Energy</span>
        <span class="s0-overlay-col-count" data-count-for="stress-energy" aria-hidden="true">0</span>
        <button class="s0-overlay-col-clear" data-clear-col="stress-energy" type="button"
            title="Turn off every stress-energy overlay">&#10005;</button>
      </div>
      <button class="view-toggle field-toggle" id="toggle-e-pressure"
          title="Electric pressure P_E(x) = ½|E|². Half of EM energy; rises on charge concentrations.">
        <span class="field-swatch field-swatch-e-pressure"></span>P&#8324; (electric)
      </button>
      <button class="view-toggle field-toggle" id="toggle-b-pressure"
          title="Magnetic pressure P_B(x) = ½|B|². Sister field to P_E; rises on current loops.">
        <span class="field-swatch field-swatch-b-pressure"></span>P&#8331; (magnetic)
      </button>
      <button class="view-toggle field-toggle" id="toggle-kinetic-energy"
          title="Kinetic energy density K(x) = ½|v|² at particle sites. Highlights moving particles.">
        <span class="field-swatch field-swatch-kinetic"></span>Kinetic K
      </button>
      <button class="view-toggle field-toggle" id="toggle-fisher"
          title="Fisher information F(x) = |∇ρ|²/ρ with ρ = |J|². Log-compressed so localized-mode edges remain readable. Brightens the sharp edges of soliton shells and wave-packet envelopes.">
        <span class="field-swatch field-swatch-fisher"></span>Fisher F
      </button>
    </div>

    <div class="s0-overlay-col" data-col="phenomena">
      <div class="s0-overlay-col-head">
        <span class="s0-overlay-col-label">Phenomena</span>
        <span class="s0-overlay-col-count" data-count-for="phenomena" aria-hidden="true">0</span>
        <button class="s0-overlay-col-clear" data-clear-col="phenomena" type="button"
            title="Turn off every phenomena overlay">&#10005;</button>
      </div>
      <button class="view-toggle field-toggle" id="toggle-dual-substrate"
          title="Dual substrate split: J_L (warm) and J_R (cool)">
        <span class="field-swatch field-swatch-dualj"></span>Dual J
      </button>
      <button class="view-toggle field-toggle" id="toggle-chirality"
          title="[PROXY] Chirality magnitude proxy: |J| · δ where δ = DUAL_DELTA ≈ 0.957. The true (E_L − E_R)/(E_L + E_R) handedness diagnostic lives in the energy budget; this overlay renders the scalar magnitude of the chiral channel, not the L−R difference (audit P1-18 correction, 2026-05-27).">
        <span class="field-swatch field-swatch-chirality"></span>Chirality
      </button>
      <button class="view-toggle field-toggle" id="toggle-dark-halo"
          title="[PROXY] Sub-threshold flux envelope: voxels where |J| &lt; K_GENESIS rendered as a halo of points. Pedagogical analogue for &quot;dark&quot; (un-manifested) matter; NOT the 17/27 Moore-shell DM_FRACTION derivation — that constant lives in the cosmology card, not this overlay.">
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
          title="[PROXY] Pair-proximity glyphs: lines drawn between every particle pair within a fixed L² radius (≈ √120 voxels). Pedagogically suggests color-flux strings but is NOT the area-law / Wilson-loop σ = 0.209 confinement claim — implementation is a distance heuristic only.">
        <span class="field-swatch field-swatch-confinement"></span>Confinement
      </button>
      <button class="view-toggle field-toggle" id="toggle-horizon"
          title="[PROXY] Event-horizon isosurface — voxels where the latency proxy L ≥ 0.95. Rendered as a voxel-centred point cloud (marching-cubes isosurface not yet implemented). The point of no escape for gravitationally trapped flux.">
        <span class="field-swatch field-swatch-horizon"></span>Horizon
      </button>
      <button class="view-toggle field-toggle" id="toggle-coherence"
          title="[PROXY] Dual-substrate coherence C(x) = (J·∇×J)/(|J|·|∇×J|) in [-1,1]. Cosine of the angle between flow and curl — chirality sign density (a scale-free stand-in for a true Helmholtz L/R decomposition).">
        <span class="field-swatch field-swatch-coherence"></span>Coherence C
      </button>
    </div>
    </div>
  `;
  return container;
}
