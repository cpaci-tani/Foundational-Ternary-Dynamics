import { G_N } from '../../../constants.js';

const G_N_LABEL = G_N.toFixed(2);

export function getScaleControlsBlocksTemplate() {
    return `
        <!-- Planetary Controls (visible only in planetary mode) -->
        <div class="scale4-only scale-controls-block">
            <div class="panel-grid panel-grid-2">
                <div class="card">
                    <div class="card-title">FTD Sandbox Physics</div>
                    <div class="scale-info-mono">
                        <div>Gravity ($G_N$): <span id="planetary-ctrl-gravity">${G_N_LABEL}</span></div>
                        <div>Verlet $\\Delta t$: <span>0.0001 (N-body)</span></div>
                    </div>
                </div>
                <div class="card">
                    <div class="card-title">Planet Genesis</div>
                    <div class="scale-info-mono">
                        <div>Renderer: <span>Procedural GLSL FBM</span></div>
                        <div>Source: <span>NASA Exoplanet Archive</span></div>
                    </div>
                </div>
            </div>
        </div>
        <!-- Cosmic Controls (visible only in cosmic mode) -->
        <div class="scale5-only scale-controls-block">
            <div class="panel-grid panel-grid-3">
                <div class="card">
                    <div class="card-title">Simulation</div>
                    <div class="scale-info-mono" id="cosmic-ctrl-sim">
                        <div>Bodies: <span id="cosmic-n-bodies">--</span></div>
                        <div>Tick: <span id="cosmic-tick">--</span></div>
                        <div>H(t): <span id="cosmic-hubble">--</span></div>
                        <div>a(t): <span id="cosmic-scale-factor">--</span></div>
                        <div>z(t): <span id="cosmic-redshift">--</span></div>
                    </div>
                </div>
                <div class="card">
                    <div class="card-title">Cosmology (FTD)</div>
                    <div class="scale-info-copy">
                        <div title="Engine [CONJECTURE], not a derived dark-energy density: Ω_Λ = 2/3 does NOT match the observed Ω_Λ ≈ 0.685. FTD natively predicts Λ = 0 (FC-1 declines ℏ); any nonzero value is a [BOUNDARY] needing a horizon length L_H. See FAQ 'dark-energy'.">&Omega;<sub>m</sub> = 1/3 &nbsp; &Omega;<sub>&Lambda;</sub> = 2/3 <span style="opacity:0.6">[CONJECTURE]</span></div>
                        <div title="Moore-shell [SELECTION]; does NOT match Planck 2018 observed Ω_DM/Ω_m ≈ 84%. See FAQ 'dark-matter'.">DM frac = 17/27 &asymp; 63% <span style="opacity:0.6">[SELECTION]</span></div>
                        <div title="G_N = ${G_N_LABEL} is the lattice-natural simulation constant [IMPOSED]. The 1/(b₃+N_c)² identification with the physical Newton constant is RETIRED — FTD-0131 [CLOSED NEGATIVE].">G<sub>N</sub> = ${G_N_LABEL} <span style="opacity:0.6">[IMPOSED]</span></div>
                        <div>&gamma; = (D+2)/D = 5/3</div>
                        <div>c = 1/&radic;3</div>
                    </div>
                </div>
                <div class="card">
                    <div class="card-title">Body Count</div>
                    <div class="scale-info-mono" id="cosmic-ctrl-bodies">
                        <div>DM: <span id="cosmic-n-dm">--</span></div>
                        <div>Gas: <span id="cosmic-n-gas">--</span></div>
                        <div>Stars: <span id="cosmic-n-stars">--</span></div>
                        <div>BH: <span id="cosmic-n-bh">--</span></div>
                        <div>KE: <span id="cosmic-ke">--</span></div>
                    </div>
                </div>
            </div>
        </div>
    `;
}

export function getZooPanelTemplate() {
    return `
        <div class="panel" id="panel-zoo">
            <div class="panel-resource-shell">
                <div class="panel-resource-toolbar">
                    <input
                        type="text"
                        class="ctrl-input panel-resource-input"
                        id="zoo-search"
                        placeholder="Search particles..."
                    >
                    <select class="tb-select panel-resource-select" id="zoo-filter" title="Filter particle zoo by category">
                        <option value="all">All Categories</option>
                        <option value="leptons">Leptons</option>
                        <option value="quarks">Quarks</option>
                        <option value="gauge_bosons">Gauge Bosons</option>
                        <option value="scalar">Scalar Boson</option>
                        <option value="baryons">Baryons</option>
                        <option value="mesons">Mesons</option>
                    </select>
                    <select class="tb-select panel-resource-select" id="zoo-group-by" title="Group particles by category (default) or by fermion generation (Moore-layer 3-generation structure)">
                        <option value="category">Group: Category</option>
                        <option value="generation">Group: Generation</option>
                    </select>
                </div>
                <div id="zoo-table-container" class="panel-resource-table-wrap"></div>
            </div>
        </div>
    `;
}

export function getInspectorPanelTemplate() {
    return `
        <div class="panel" id="panel-inspector">
            <div class="panel-resource-shell panel-resource-shell-padded">
                <div class="card panel-inspector-hero">
                    <div class="panel-inspector-hero-copy">
                        <div class="card-title">Selection Inspector</div>
                        <div id="insp-mode-label" class="panel-inspector-mode">Lattice</div>
                        <div id="insp-selection-summary" class="panel-inspector-summary">
                            Single-click a visible object in the viewport to inspect it. Camera drags will not trigger a selection.
                        </div>
                    </div>
                    <div class="panel-inspector-hero-actions">
                        <button id="btn-clear-inspector" class="ctrl-btn" type="button">Clear</button>
                        <button id="btn-focus-voxel" class="ctrl-btn scale0-only" type="button" title="Focus camera on the selected voxel">Focus</button>
                    </div>
                </div>
                <div class="card scale0-only panel-resource-card panel-inspector-hit-card">
                    <div class="toggle-row">
                        <span>Hit Radius</span>
                        <div class="slider-group">
                            <input type="range" id="raycast-threshold" min="0.1" max="10.0" step="0.1" value="2.0">
                            <span id="raycast-threshold-val" class="slider-val">2.0</span>
                        </div>
                    </div>
                    <div class="panel-inspector-hit-help">Increase this when lattice selections feel too sparse or hard to grab.</div>
                </div>
                <div class="inspector-empty scale0-only" id="inspector-empty">
                    Select a voxel or manifested particle to inspect its local state.
                </div>
                <div id="inspector-content" class="scale0-only panel-resource-hidden">
                    <div class="panel-resource-grid panel-resource-grid-2">
                        <div class="card panel-resource-card">
                            <div class="card-title">Identity</div>
                            <dl class="inspector-grid">
                                <dt>Particle ID</dt>
                                <dd id="insp-id">--</dd>
                                <dt>State</dt>
                                <dd id="insp-state">--</dd>
                                <dt>Position</dt>
                                <dd id="insp-pos">--</dd>
                                <dt>Spin</dt>
                                <dd id="insp-spin">--</dd>
                                <dt>Color</dt>
                                <dd id="insp-color">--</dd>
                                <dt>Pair ID</dt>
                                <dd id="insp-pair">--</dd>
                                <dt>Locked</dt>
                                <dd id="insp-locked">--</dd>
                            </dl>
                        </div>
                        <div class="card panel-resource-card">
                            <div class="card-title">Field State</div>
                            <dl class="inspector-grid">
                                <dt>Flux</dt>
                                <dd id="insp-flux">--</dd>
                                <dt>Density</dt>
                                <dd id="insp-density">--</dd>
                                <dt>div(J)</dt>
                                <dd id="insp-divj">--</dd>
                                <dt>curl(J)</dt>
                                <dd id="insp-curl">--</dd>
                                <dt>Velocity</dt>
                                <dd id="insp-vel">--</dd>
                                <dt>Speed</dt>
                                <dd id="insp-speed">--</dd>
                                <dt>Accel</dt>
                                <dd id="insp-accel">--</dd>
                            </dl>
                        </div>
                        <div class="card panel-resource-card">
                            <div class="card-title">Electromagnetism</div>
                            <dl class="inspector-grid">
                                <dt>|E|</dt>
                                <dd id="insp-e-mag">--</dd>
                                <dt>|B|</dt>
                                <dd id="insp-b-mag">--</dd>
                            </dl>
                        </div>
                        <div class="card panel-resource-card">
                            <div class="card-title">Forces</div>
                            <dl class="inspector-grid">
                                <dt>Coulomb</dt>
                                <dd id="insp-f-coulomb">--</dd>
                                <dt>Gravity</dt>
                                <dd id="insp-f-gravity">--</dd>
                                <dt>Magnetic</dt>
                                <dd id="insp-f-magnetic">--</dd>
                                <dt>Strong</dt>
                                <dd id="insp-f-strong">--</dd>
                                <dt>Exchange</dt>
                                <dd id="insp-f-exchange">--</dd>
                            </dl>
                        </div>
                    </div>
                </div>
                <div class="inspector-empty scale1-only" id="pe-inspector-empty" title="Viewport selection helper.">
                    Select a particle in the viewport to inspect its trajectory and interactions.
                </div>
                <div id="pe-inspector-content" class="scale1-only panel-resource-hidden">
                    <div class="panel-resource-grid panel-resource-grid-3">
                        <div class="card panel-resource-card" title="Identifiers, category, mass, charge, spin, and color indices.">
                            <div class="card-title">Identity</div>
                            <div class="pe-insp-header">
                                <span class="pe-insp-catalog-dot" id="pe-insp-dot"></span>
                                <span class="pe-insp-name" id="pe-insp-name">--</span>
                                <span class="pe-insp-symbol" id="pe-insp-symbol">--</span>
                            </div>
                            <dl class="inspector-grid">
                                <dt title="The unique identifier assigned to this particle by the simulation engine.">Engine ID</dt>
                                <dd id="pe-insp-id">--</dd>
                                <dt title="The standard model or FTD particle category (e.g., lepton, meson, hadron).">Catalog</dt>
                                <dd id="pe-insp-catalog">--</dd>
                                <dt title="The rest mass of the particle in MeV.">Mass</dt>
                                <dd id="pe-insp-mass">--</dd>
                                <dt title="The electric charge of the particle in units of elementary charge e (e.g. +1, -1, 0).">Charge</dt>
                                <dd id="pe-insp-charge">--</dd>
                                <dt title="Indicates if the particle is locked in space (fixed position) or mobile (moves dynamically under force integration).">Locked</dt>
                                <dd id="pe-insp-locked">--</dd>
                                <dt title="Effective interaction radius (in lattice units, lu).">Radius</dt>
                                <dd id="pe-insp-reff">--</dd>
                                <dt title="Spin quantum number projection.">Spin</dt>
                                <dd id="pe-insp-spin">--</dd>
                                <dt title="Color charge state index.">Color</dt>
                                <dd id="pe-insp-color">--</dd>
                                <dt title="Entanglement pair ID (-1 if unbound).">Pair ID</dt>
                                <dd id="pe-insp-pair">--</dd>
                            </dl>
                        </div>
                        <div class="card panel-resource-card" title="Dynamic motion states (position, velocity, speed, acceleration, and kinetic energy).">
                            <div class="card-title">Dynamics</div>
                            <dl class="inspector-grid">
                                <dt title="The coordinate vector (x, y, z) of the particle on the 3D lattice (in lattice units, lu).">Position</dt>
                                <dd id="pe-insp-pos">--</dd>
                                <dt title="The velocity vector (vx, vy, vz) of the particle (in fractions of speed of light c).">Velocity</dt>
                                <dd id="pe-insp-vel">--</dd>
                                <dt title="The speed of the particle |v| as a fraction of c (where c = 1/sqrt(3) on the cubic lattice).">Speed</dt>
                                <dd id="pe-insp-speed">--</dd>
                                <dt title="The kinetic energy of the particle: 1/2 * m * v^2 (in MeV).">KE</dt>
                                <dd id="pe-insp-ke">--</dd>
                                <dt title="Magnitude of momentum |p| (in MeV/c).">Momentum</dt>
                                <dd id="pe-insp-momentum">--</dd>
                                <dt title="Magnitude of acceleration |a| (change in velocity per tick).">Accel</dt>
                                <dd id="pe-insp-accel">--</dd>
                                <dt title="The orbital radius or distance to the center of mass in 2-body orbits (in lattice units, lu).">Orbital r</dt>
                                <dd id="pe-insp-orbital">--</dd>
                            </dl>
                        </div>
                        <div class="card panel-resource-card" title="Distance and force magnitudes relative to neighboring particles.">
                            <div class="card-title">Interactions</div>
                            <dl class="inspector-grid">
                                <dt title="The Engine ID of the nearest neighboring particle.">Nearest</dt>
                                <dd id="pe-insp-nearest">--</dd>
                                <dt title="The Euclidean distance to the nearest particle (in lattice units, lu).">Distance</dt>
                                <dd id="pe-insp-dist">--</dd>
                                <dt title="The magnitude of the electrostatic Coulomb force acting on this particle.">Coulomb |F|</dt>
                                <dd id="pe-insp-fc">--</dd>
                                <dt title="The magnitude of the net combined force vector acting on this particle.">Net |F|</dt>
                                <dd id="pe-insp-fnet">--</dd>
                            </dl>
                        </div>
                    </div>
                </div>
                <div class="scale-ae panel-resource-hidden" id="ae-scenario-info">
                    <div class="card panel-resource-card panel-resource-section-card">
                        <div class="card-title" id="ae-scenario-title">--</div>
                        <div class="panel-resource-muted-copy" id="ae-inspector-scenario-desc">--</div>
                        <dl class="inspector-grid" id="ae-scenario-fields"></dl>
                    </div>
                </div>
                <div class="scale3-only panel-resource-hidden" id="ae-mol-info">
                    <div class="card panel-resource-card panel-resource-section-card">
                        <div class="card-title" id="ae-mol-title">Molecule</div>
                        <div class="panel-resource-muted-copy" id="ae-mol-desc">--</div>
                        <dl class="inspector-grid">
                            <dt>Formula</dt>
                            <dd id="ae-mol-formula">--</dd>
                            <dt>Category</dt>
                            <dd id="ae-mol-category">--</dd>
                            <dt>Atoms</dt>
                            <dd id="ae-mol-atom-count">--</dd>
                            <dt>Composition</dt>
                            <dd id="ae-mol-composition">--</dd>
                            <dt>Bonds</dt>
                            <dd id="ae-mol-bond-count">--</dd>
                            <dt>Total Mass</dt>
                            <dd id="ae-mol-mass">--</dd>
                        </dl>
                    </div>
                </div>
                <div class="inspector-empty scale-ae" id="ae-inspector-empty">
                    Select an atom or orbital cloud sample to inspect its local chemistry.
                </div>
                <div id="ae-inspector-content" class="scale-ae panel-resource-hidden">
                    <div class="panel-resource-grid panel-resource-grid-3">
                        <div class="card panel-resource-card">
                            <div class="card-title">Element</div>
                            <div class="pe-insp-header">
                                <span class="pe-insp-catalog-dot" id="ae-insp-dot"></span>
                                <span class="pe-insp-name" id="ae-insp-name">--</span>
                                <span class="pe-insp-symbol" id="ae-insp-symbol">--</span>
                            </div>
                            <dl class="inspector-grid">
                                <dt>Atom ID</dt>
                                <dd id="ae-insp-id">--</dd>
                                <dt>Z</dt>
                                <dd id="ae-insp-z">--</dd>
                                <dt>Charge</dt>
                                <dd id="ae-insp-charge">--</dd>
                                <dt>Mass</dt>
                                <dd id="ae-insp-mass">--</dd>
                                <dt>Locked</dt>
                                <dd id="ae-insp-locked">--</dd>
                                <dt>N (neutrons)</dt>
                                <dd id="ae-insp-n">--</dd>
                                <dt>A (mass #)</dt>
                                <dd id="ae-insp-a">--</dd>
                                <dt>Max Bonds</dt>
                                <dd id="ae-insp-maxbonds">--</dd>
                            </dl>
                        </div>
                        <div class="card panel-resource-card">
                            <div class="card-title">Dynamics</div>
                            <dl class="inspector-grid">
                                <dt>Position</dt>
                                <dd id="ae-insp-pos">--</dd>
                                <dt>Velocity</dt>
                                <dd id="ae-insp-vel">--</dd>
                                <dt>Speed</dt>
                                <dd id="ae-insp-speed">--</dd>
                                <dt>KE</dt>
                                <dd id="ae-insp-ke">--</dd>
                                <dt>Net |F|</dt>
                                <dd id="ae-insp-fnet">--</dd>
                            </dl>
                        </div>
                        <div class="card panel-resource-card">
                            <div class="card-title">Neighbors</div>
                            <dl class="inspector-grid scale3-only panel-inspector-bonds" id="ae-insp-bonds"></dl>
                            <div class="panel-resource-divider-card">
                                <dl class="inspector-grid">
                                    <dt>Nearest</dt>
                                    <dd id="ae-insp-nearest">--</dd>
                                    <dt>Distance</dt>
                                    <dd id="ae-insp-nearest-dist">--</dd>
                                </dl>
                            </div>
                            <div class="panel-resource-divider-card">
                                <dl class="inspector-grid">
                                    <dt>vdW &#963;</dt>
                                    <dd id="ae-insp-sigma">--</dd>
                                    <dt>vdW &#949;</dt>
                                    <dd id="ae-insp-epsilon">--</dd>
                                </dl>
                            </div>
                        </div>
                        <div class="card panel-resource-card">
                            <div class="card-title">Closure Context</div>
                            <dl class="inspector-grid" style="grid-template-columns: minmax(80px, auto) 1fr;">
                                <dt title="Polarizability (&alpha;_pol): Cloud volume compliance to external fields.">&alpha;<sub>pol</sub></dt>
                                <dd style="display:flex; flex-direction:column; gap:2px; min-width:0;">
                                    <div id="ae-insp-alpha-pol">--</div>
                                    <div style="font-size:0.65rem; opacity:0.6; display:flex; justify-content:space-between;">
                                        <span id="ae-insp-alpha-pol-min">--</span>
                                        <span id="ae-insp-alpha-pol-avg">--</span>
                                        <span id="ae-insp-alpha-pol-max">--</span>
                                    </div>
                                    <div id="ae-insp-alpha-pol-spark" style="height:24px; width:100%;"></div>
                                </dd>
                                <dt title="Ionization Energy (E_ion): Energy required to strip the most loosely bound electron.">E<sub>ion</sub></dt>
                                <dd style="display:flex; flex-direction:column; gap:2px; min-width:0;">
                                    <div id="ae-insp-e-ion">--</div>
                                    <div style="font-size:0.65rem; opacity:0.6; display:flex; justify-content:space-between;">
                                        <span id="ae-insp-e-ion-min">--</span>
                                        <span id="ae-insp-e-ion-avg">--</span>
                                        <span id="ae-insp-e-ion-max">--</span>
                                    </div>
                                    <div id="ae-insp-e-ion-spark" style="height:24px; width:100%;"></div>
                                </dd>
                                <dt title="Electron Affinity (E_aff): Tendency to capture an external electron.">E<sub>aff</sub></dt>
                                <dd style="display:flex; flex-direction:column; gap:2px; min-width:0;">
                                    <div id="ae-insp-e-aff">--</div>
                                    <div style="font-size:0.65rem; opacity:0.6; display:flex; justify-content:space-between;">
                                        <span id="ae-insp-e-aff-min">--</span>
                                        <span id="ae-insp-e-aff-avg">--</span>
                                        <span id="ae-insp-e-aff-max">--</span>
                                    </div>
                                    <div id="ae-insp-e-aff-spark" style="height:24px; width:100%;"></div>
                                </dd>
                                <dt title="Scattering Cross-section (&sigma;_scatter): Interaction footprint for physical collisions.">&sigma;<sub>scatter</sub></dt>
                                <dd style="display:flex; flex-direction:column; gap:2px; min-width:0;">
                                    <div id="ae-insp-sigma-scatter">--</div>
                                    <div style="font-size:0.65rem; opacity:0.6; display:flex; justify-content:space-between;">
                                        <span id="ae-insp-sigma-scatter-min">--</span>
                                        <span id="ae-insp-sigma-scatter-avg">--</span>
                                        <span id="ae-insp-sigma-scatter-max">--</span>
                                    </div>
                                    <div id="ae-insp-sigma-scatter-spark" style="height:24px; width:100%;"></div>
                                </dd>
                                <dt title="Effective Nuclear Charge (Z_eff): Net positive charge experienced by valence electrons.">Z<sub>eff</sub></dt>
                                <dd style="display:flex; flex-direction:column; gap:2px; min-width:0;">
                                    <div id="ae-insp-zeff">--</div>
                                    <div style="font-size:0.65rem; opacity:0.6; display:flex; justify-content:space-between;">
                                        <span id="ae-insp-zeff-min">--</span>
                                        <span id="ae-insp-zeff-avg">--</span>
                                        <span id="ae-insp-zeff-max">--</span>
                                    </div>
                                    <div id="ae-insp-zeff-spark" style="height:24px; width:100%;"></div>
                                </dd>
                                <dt title="Fractional Charge (q_frac): Continuous charge due to charge equilibration (QEq) with bonded neighbors.">q<sub>frac</sub></dt>
                                <dd style="display:flex; flex-direction:column; gap:2px; min-width:0;">
                                    <div id="ae-insp-q-frac">--</div>
                                    <div style="font-size:0.65rem; opacity:0.6; display:flex; justify-content:space-between;">
                                        <span id="ae-insp-q-frac-min">--</span>
                                        <span id="ae-insp-q-frac-avg">--</span>
                                        <span id="ae-insp-q-frac-max">--</span>
                                    </div>
                                    <div id="ae-insp-q-frac-spark" style="height:24px; width:100%;"></div>
                                </dd>
                            </dl>
                        </div>
                    </div>
                </div>
                <div class="inspector-empty scale4-only panel-resource-hidden" id="planetary-inspector-empty">
                    Select a world or star to inspect its current telemetry.
                </div>
                <div id="planetary-inspector-content" class="scale4-only panel-resource-hidden">
                    <div class="panel-resource-grid panel-resource-grid-2">
                        <div class="card panel-resource-card">
                            <div class="card-title">Astrophysical Identity</div>
                            <div class="pe-insp-header">
                                <span class="pe-insp-catalog-dot" id="planetary-insp-dot"></span>
                                <span class="pe-insp-name" id="planetary-insp-type">--</span>
                            </div>
                            <dl class="inspector-grid">
                                <dt>Body ID</dt>
                                <dd id="planetary-insp-id">--</dd>
                                <dt>Mass (sol)</dt>
                                <dd id="planetary-insp-mass">--</dd>
                                <dt>Temp (K)</dt>
                                <dd id="planetary-insp-temp">--</dd>
                                <dt>Biome</dt>
                                <dd id="planetary-insp-biome">--</dd>
                            </dl>
                        </div>
                        <div class="card panel-resource-card">
                            <div class="card-title">Orbital Dynamics</div>
                            <dl class="inspector-grid">
                                <dt>Pos (AU)</dt>
                                <dd id="planetary-insp-pos">--</dd>
                                <dt>Velocity</dt>
                                <dd id="planetary-insp-vel">--</dd>
                                <dt>Speed</dt>
                                <dd id="planetary-insp-speed">--</dd>
                            </dl>
                        </div>
                    </div>
                </div>
                <div class="inspector-empty scale-cosmic panel-resource-hidden" id="cosmic-inspector-empty">
                    Select a body to inspect its mass, motion, and evolutionary state.
                </div>
                <div id="cosmic-inspector-content" class="scale-cosmic panel-resource-hidden">
                    <div class="panel-resource-grid panel-resource-grid-3">
                        <div class="card panel-resource-card">
                            <div class="card-title">Astrophysical Identity</div>
                            <div class="pe-insp-header">
                                <span class="pe-insp-catalog-dot" id="cosmic-insp-dot"></span>
                                <span class="pe-insp-name" id="cosmic-insp-type">--</span>
                            </div>
                            <dl class="inspector-grid">
                                <dt>Body ID</dt>
                                <dd id="cosmic-insp-id">--</dd>
                                <dt>Mass</dt>
                                <dd id="cosmic-insp-mass">--</dd>
                                <dt>Radius</dt>
                                <dd id="cosmic-insp-radius">--</dd>
                                <dt>Age</dt>
                                <dd id="cosmic-insp-age">--</dd>
                                <dt>Temp (K)</dt>
                                <dd id="cosmic-insp-temp">--</dd>
                                <dt>Luminosity</dt>
                                <dd id="cosmic-insp-lum">--</dd>
                            </dl>
                        </div>
                        <div class="card panel-resource-card">
                            <div class="card-title">Orbital Dynamics</div>
                            <dl class="inspector-grid">
                                <dt>Position</dt>
                                <dd id="cosmic-insp-pos">--</dd>
                                <dt>Velocity</dt>
                                <dd id="cosmic-insp-vel">--</dd>
                                <dt>Speed</dt>
                                <dd id="cosmic-insp-speed">--</dd>
                            </dl>
                        </div>
                        <div class="card panel-resource-card">
                            <div class="card-title">Evolution State</div>
                            <dl class="inspector-grid">
                                <dt>Fuel Rem.</dt>
                                <dd id="cosmic-insp-fuel-frac">--</dd>
                                <dt>Stellar Phase</dt>
                                <dd id="cosmic-insp-fuel-stage">--</dd>
                            </dl>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    `;
}

export function getPhysicsPanelTemplate() {
    // Energy-levels / cross-sections / decay-rates cards RETIRED (2026-07
    // revision): they belonged to the retired parametric SM sandbox
    // (cross-sections.js / decay-rates.js deleted; spectroscopy.js survives
    // for the Scale-0 hydrogen p1-observable only).
    return `
        <div class="panel" id="panel-physics">
            <div class="panel-resource-shell panel-resource-shell-padded">
                <div class="panel-resource-grid panel-resource-grid-2">
                    <div class="card panel-resource-card" id="physics-constants">
                        <div class="card-title">Ontic Chain Constants</div>
                        <div class="panel-resource-empty">Loading...</div>
                    </div>
                </div>
            </div>
        </div>
    `;
}

export function getPlanetaryPanelTemplate() {
    return `
        <div class="panel" id="panel-planetary">
            <div class="panel-section panel-resource-stack">
                <div class="card">
                    <div class="card-title">Celestial Hierarchy</div>
                    <ul id="planetary-layer-list" class="zoo-list panel-resource-list">
                    </ul>
                </div>
            </div>
        </div>
    `;
}

export function getCosmicInfoPanelTemplate() {
    return `
        <div class="panel" id="panel-cosmic-info">
            <div class="panel-resource-scroll panel-resource-content-pad">
                <h3 class="panel-resource-title">Scale 5: Cosmic Simulation</h3>
                <p class="panel-resource-lead">
                    N-body + SPH cosmic simulation driven by FTD constants — a mix of
                    [THEOREM] / [SELECTION] / [IMPOSED] inputs (see table). The algebraic
                    chain from D=3 + ϖ is [THEOREM]; physical identifications are [SMC] (FTD-0013).
                </p>
                <table class="panel-resource-table">
                    <tr>
                        <td class="panel-resource-key">G<sub>N</sub></td>
                        <td>= ${G_N_LABEL}</td>
                        <td class="panel-resource-note" title="G_N = ${G_N_LABEL} is the lattice-natural simulation constant [IMPOSED]. The 1/(b₃+N_c)² identification with the physical Newton constant is RETIRED — FTD-0131 [CLOSED NEGATIVE] (off by 10²⁰–10⁴³ under any natural calibration).">Gravity [IMPOSED] · 1/(b₃+N_c)² RETIRED (FTD-0131)</td>
                    </tr>
                    <tr>
                        <td class="panel-resource-key">&Omega;<sub>&Lambda;</sub></td>
                        <td>= 2/3</td>
                        <td class="panel-resource-note" title="Engine [CONJECTURE], not a derived dark-energy density: 2/3 does NOT match the observed Ω_Λ ≈ 0.685. FTD natively predicts Λ = 0 (FC-1 declines ℏ); any nonzero value is a [BOUNDARY] needing a horizon length L_H. See FAQ 'dark-energy'.">Dark energy [CONJECTURE]</td>
                    </tr>
                    <tr>
                        <td class="panel-resource-key">&Omega;<sub>m</sub></td>
                        <td>= 1/3</td>
                        <td class="panel-resource-note">Matter</td>
                    </tr>
                    <tr>
                        <td class="panel-resource-key">DM frac</td>
                        <td>= 17/27 &asymp; 63%</td>
                        <td class="panel-resource-note" title="Moore-shell selection; does NOT match Planck 2018 observed Ω_DM/Ω_m ≈ 84%. See FAQ 'dark-matter-17-27'.">Moore-shell [SELECTION]</td>
                    </tr>
                    <tr>
                        <td class="panel-resource-key">&gamma;</td>
                        <td>= (D+2)/D = 5/3</td>
                        <td class="panel-resource-note">Adiabatic index [SELECTION]</td>
                    </tr>
                    <tr>
                        <td class="panel-resource-key">c</td>
                        <td>= 1/&radic;3</td>
                        <td class="panel-resource-note">Selected lattice speed [SELECTION]</td>
                    </tr>
                    <tr>
                        <td class="panel-resource-key">r<sub>s</sub></td>
                        <td>= 2 G<sub>N</sub> M / c²</td>
                        <td class="panel-resource-note" title="Schwarzschild-inspired lattice radius proxy, linear in M. Scale 5 does not solve a Schwarzschild metric; G_N and the on-screen visual gauge are imposed, and the rendered radius is clamped to a visible band.">BH radius proxy [IMPOSED]</td>
                    </tr>
                </table>
                <div class="panel-resource-divider">
                    <div class="panel-resource-subtitle">Live Diagnostics</div>
                    <div id="cosmic-panel-diagnostics" class="panel-resource-mono"></div>
                </div>
            </div>
        </div>
    `;
}
