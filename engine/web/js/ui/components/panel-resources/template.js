export function getScaleControlsBlocksTemplate() {
    return `
        <!-- Planetary Controls (visible only in planetary mode) -->
        <div class="scale4-only scale-controls-block">
            <div class="panel-grid panel-grid-2">
                <div class="card">
                    <div class="card-title">FTD Sandbox Physics</div>
                    <div class="scale-info-mono">
                        <div>Gravity ($G_N$): <span>0.01</span></div>
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
                    </div>
                </div>
                <div class="card">
                    <div class="card-title">Cosmology (FTD)</div>
                    <div class="scale-info-copy">
                        <div>&Omega;<sub>m</sub> = 1/3 &nbsp; &Omega;<sub>&Lambda;</sub> = 2/3</div>
                        <div>DM frac = 17/27 &asymp; 63%</div>
                        <div>G<sub>N</sub> = 1/(b<sub>3</sub>+N<sub>c</sub>)&sup2; = 0.01</div>
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
                <div class="inspector-empty scale1-only" id="pe-inspector-empty">
                    Select a particle in the viewport to inspect its trajectory and interactions.
                </div>
                <div id="pe-inspector-content" class="scale1-only panel-resource-hidden">
                    <div class="panel-resource-grid panel-resource-grid-3">
                        <div class="card panel-resource-card">
                            <div class="card-title">Identity</div>
                            <div class="pe-insp-header">
                                <span class="pe-insp-catalog-dot" id="pe-insp-dot"></span>
                                <span class="pe-insp-name" id="pe-insp-name">--</span>
                                <span class="pe-insp-symbol" id="pe-insp-symbol">--</span>
                            </div>
                            <dl class="inspector-grid">
                                <dt>Engine ID</dt>
                                <dd id="pe-insp-id">--</dd>
                                <dt>Catalog</dt>
                                <dd id="pe-insp-catalog">--</dd>
                                <dt>Mass</dt>
                                <dd id="pe-insp-mass">--</dd>
                                <dt>Charge</dt>
                                <dd id="pe-insp-charge">--</dd>
                                <dt>Locked</dt>
                                <dd id="pe-insp-locked">--</dd>
                            </dl>
                        </div>
                        <div class="card panel-resource-card">
                            <div class="card-title">Dynamics</div>
                            <dl class="inspector-grid">
                                <dt>Position</dt>
                                <dd id="pe-insp-pos">--</dd>
                                <dt>Velocity</dt>
                                <dd id="pe-insp-vel">--</dd>
                                <dt>Speed</dt>
                                <dd id="pe-insp-speed">--</dd>
                                <dt>KE</dt>
                                <dd id="pe-insp-ke">--</dd>
                                <dt>Orbital r</dt>
                                <dd id="pe-insp-orbital">--</dd>
                            </dl>
                        </div>
                        <div class="card panel-resource-card">
                            <div class="card-title">Interactions</div>
                            <dl class="inspector-grid">
                                <dt>Nearest</dt>
                                <dd id="pe-insp-nearest">--</dd>
                                <dt>Distance</dt>
                                <dd id="pe-insp-dist">--</dd>
                                <dt>Coulomb |F|</dt>
                                <dd id="pe-insp-fc">--</dd>
                                <dt>Net |F|</dt>
                                <dd id="pe-insp-fnet">--</dd>
                            </dl>
                        </div>
                    </div>
                </div>
                <div class="scale-ae panel-resource-hidden" id="ae-scenario-info">
                    <div class="card panel-resource-card panel-resource-section-card">
                        <div class="card-title" id="ae-scenario-title">--</div>
                        <div class="panel-resource-muted-copy" id="ae-scenario-desc">--</div>
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
    return `
        <div class="panel" id="panel-physics">
            <div class="panel-resource-shell panel-resource-shell-padded">
                <div class="panel-resource-grid panel-resource-grid-2">
                    <div class="card panel-resource-card" id="physics-energy-levels">
                        <div class="card-title">Energy Levels</div>
                        <div class="panel-resource-empty">Loading spectroscopy...</div>
                    </div>
                    <div class="card panel-resource-card" id="physics-cross-sections">
                        <div class="card-title">Cross-Sections</div>
                        <div class="panel-resource-empty">Loading...</div>
                    </div>
                    <div class="card panel-resource-card" id="physics-decay-rates">
                        <div class="card-title">Decay Rates</div>
                        <div class="panel-resource-empty">Loading...</div>
                    </div>
                    <div class="card panel-resource-card" id="physics-constants">
                        <div class="card-title">Ontic Chain Constants</div>
                        <div class="panel-resource-empty">Loading...</div>
                    </div>
                </div>
                <div class="panel-resource-slider-row">
                    <label class="panel-resource-slider-label" for="physics-z-slider">Z (atomic number)</label>
                    <input class="panel-resource-slider" type="range" id="physics-z-slider" min="1" max="92" step="1" value="1">
                    <span id="physics-z-value" class="panel-resource-slider-value">Z=1</span>
                </div>
            </div>
        </div>
    `;
}

export function getHierarchyPanelTemplate() {
    return `
        <div class="panel" id="panel-hierarchy">
            <div class="panel-resource-shell panel-resource-shell-padded">
                <div class="panel-resource-grid panel-resource-grid-3">
                    <div class="card panel-resource-card" id="hierarchy-tower">
                        <div class="card-title">Aggregation Levels</div>
                        <div class="panel-resource-empty">Loading...</div>
                    </div>
                    <div class="card panel-resource-card" id="hierarchy-bridge">
                        <div class="card-title">Scale Bridge</div>
                        <div class="panel-resource-empty">Loading...</div>
                    </div>
                    <div class="card panel-resource-card" id="hierarchy-emergence">
                        <div class="card-title">Emergence Monitor</div>
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
                <div class="card panel-resource-card-spaced">
                    <div class="card-title">Visualization Overlays</div>
                    <div class="engine-toggles grid-cols-1 panel-resource-toggle-grid">
                        <label class="toggle-control">
                            <input type="checkbox" id="planetary-opt-orbits" checked>
                            <span class="control"></span>
                            <span class="label">Show Orbital Traces</span>
                        </label>
                        <label class="toggle-control">
                            <input type="checkbox" id="planetary-opt-axes">
                            <span class="control"></span>
                            <span class="label">Show Rotational Axes</span>
                        </label>
                    </div>
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
                    N-body + SPH cosmic simulation driven by FTD-derived constants.
                    All physics traces to D=3 and the lemniscate constant.
                </p>
                <table class="panel-resource-table">
                    <tr>
                        <td class="panel-resource-key">G<sub>N</sub></td>
                        <td>= 1/(b<sub>3</sub>+N<sub>c</sub>)<sup>2</sup> = 0.01</td>
                        <td class="panel-resource-note">Gravity</td>
                    </tr>
                    <tr>
                        <td class="panel-resource-key">&Omega;<sub>&Lambda;</sub></td>
                        <td>= 2/3</td>
                        <td class="panel-resource-note">Dark energy</td>
                    </tr>
                    <tr>
                        <td class="panel-resource-key">&Omega;<sub>m</sub></td>
                        <td>= 1/3</td>
                        <td class="panel-resource-note">Matter</td>
                    </tr>
                    <tr>
                        <td class="panel-resource-key">DM frac</td>
                        <td>= 17/27 &asymp; 63%</td>
                        <td class="panel-resource-note">Moore theorem</td>
                    </tr>
                    <tr>
                        <td class="panel-resource-key">&gamma;</td>
                        <td>= (D+2)/D = 5/3</td>
                        <td class="panel-resource-note">Adiabatic index</td>
                    </tr>
                    <tr>
                        <td class="panel-resource-key">c</td>
                        <td>= 1/&radic;3</td>
                        <td class="panel-resource-note">CFL speed</td>
                    </tr>
                    <tr>
                        <td class="panel-resource-key">r<sub>s</sub></td>
                        <td>= 2G<sub>N</sub>M</td>
                        <td class="panel-resource-note">Schwarzschild</td>
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

export function getMetaInfoPanelTemplate() {
    return `
        <div class="panel" id="panel-meta-info">
            <div id="meta-info-panel" class="panel-resource-scroll"></div>
            <div id="meta-inspect-panel" class="panel-resource-inspect">
                <div class="panel-resource-inspect-empty">Click a site to inspect</div>
            </div>
        </div>
    `;
}
