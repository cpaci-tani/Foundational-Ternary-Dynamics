export function getDiagnosticsPanelTemplate() {
    return `
        <div class="panel" id="panel-diagnostics">
            <!-- Scale 0 tables are now rendered by DiagnosticsPanelComponent
                 from descriptors/scale0.js. The old inline markup has been
                 removed (panels redesign 2026-04). -->
            <!-- Legacy Scale 0 block kept hidden for the status-bar-driving
                 ctx.diagnostics object which reads these IDs; it silently
                 no-ops on missing elements. Reintroduce only if you need
                 the IDs. -->
            <div class="scale0-only diag-s0-grid" style="display:none" hidden>
                <div>
                    <div class="combo-section-label">Particle State</div>
                    <div class="inspector-grid">
                        <dt>Manifested</dt>
                        <dd id="diag-manifested">0</dd>
                        <dt>Positive</dt>
                        <dd id="diag-positive" class="diag-dd-positive">0</dd>
                        <dt>Negative</dt>
                        <dd id="diag-negative" class="diag-dd-negative">0</dd>
                        <dt>Charge (net)</dt>
                        <dd id="diag-charge">0</dd>
                        <dt>Spin Up/Down</dt>
                        <dd class="diag-badge-row">
                            <span class="diag-badge" id="diag-spin-up">0</span>
                            <span class="diag-badge" id="diag-spin-down">0</span>
                        </dd>
                        <dt>Color R/G/B</dt>
                        <dd class="diag-badge-row">
                            <span class="diag-badge diag-badge-r" id="diag-color-r">0</span>
                            <span class="diag-badge diag-badge-g" id="diag-color-g">0</span>
                            <span class="diag-badge diag-badge-b" id="diag-color-b">0</span>
                        </dd>
                        <dt>Colorless</dt>
                        <dd id="diag-colorless">0</dd>
                    </div>
                    <div class="combo-section-label">Energy Budget</div>
                    <div class="inspector-grid">
                        <dt>Total Energy</dt>
                        <dd id="diag-energy">0.0000</dd>
                        <dt>Field |J|&sup2;</dt>
                        <dd id="diag-field-energy">0.0000</dd>
                        <dt>Wave |w|&sup2;</dt>
                        <dd id="diag-wave-energy">0.0000</dd>
                        <dt>Particle KE</dt>
                        <dd id="diag-particle-ke">0.0000</dd>
                        <dt>Coulomb PE</dt>
                        <dd id="diag-coulomb-pe">0.0000</dd>
                        <dt>Total Flux</dt>
                        <dd id="diag-flux">0.0000</dd>
                        <dt>Entropy</dt>
                        <dd id="diag-entropy">0.0000</dd>
                    </div>
                </div>
                <!-- Right Column -->
                <div>
                    <div class="combo-section-label">Electromagnetic</div>
                    <div class="inspector-grid">
                        <dt>E-Field |E|&sup2;/2</dt>
                        <dd id="diag-e-field-energy">0.0000</dd>
                        <dt>B-Field |B|&sup2;/2</dt>
                        <dd id="diag-b-field-energy">0.0000</dd>
                        <dt>Poynting |S|</dt>
                        <dd id="diag-poynting">0.0000</dd>
                        <dt>Angular Mom</dt>
                        <dd id="diag-angular-mom" class="diag-angular-mom">0, 0, 0</dd>
                    </div>
                    <div class="combo-section-label">Constraints</div>
                    <div class="inspector-grid">
                        <dt>Gauss &sum;(div J-s)&sup2;</dt>
                        <dd id="diag-gauss-violation">0.0000</dd>
                        <dt>Max Gauss err</dt>
                        <dd id="diag-max-gauss">0.0000</dd>
                        <dt>Self-field inj</dt>
                        <dd id="diag-self-field">0.0000</dd>
                    </div>
                    <div class="combo-section-label">Dual Substrate</div>
                    <div class="inspector-grid">
                        <dt>E<sub>L</sub> (left)</dt>
                        <dd id="diag-e-left">0.0000</dd>
                        <dt>E<sub>R</sub> (right)</dt>
                        <dd id="diag-e-right">0.0000</dd>
                        <dt>Chirality</dt>
                        <dd id="diag-chirality">0.0000</dd>
                        <dt>Wave L / R</dt>
                        <dd id="diag-wave-lr">0 / 0</dd>
                    </div>
                </div>
                <!-- Sparklines row (below both columns) -->
                <div class="diag-sparkline-row">
                    <div>
                        <div class="combo-section-label">Manifested</div>
                        <canvas class="stat-sparkline" id="spark-manifested"></canvas>
                    </div>
                    <div>
                        <div class="combo-section-label">Charge</div>
                        <canvas class="stat-sparkline" id="spark-charges"></canvas>
                    </div>
                    <div>
                        <div class="combo-section-label">Flux</div>
                        <canvas class="stat-sparkline" id="spark-flux"></canvas>
                    </div>
                    <div>
                        <div class="combo-section-label">Energy</div>
                        <canvas class="stat-sparkline" id="spark-energy"></canvas>
                    </div>
                    <div>
                        <div class="combo-section-label">Entropy</div>
                        <canvas class="stat-sparkline" id="spark-entropy"></canvas>
                    </div>
                </div>
            </div>
            <!-- Scale 1 PE Telemetry Panel -->
            <div id="pe-telemetry" class="scale1-only">
                <!-- Section 1: Conservation Laws (always visible) -->
                <div class="pe-telem-section">
                    <div class="pe-telem-title">Conservation Laws</div>
                    <div class="pe-conservation-row">
                        <span class="pe-cons-label">Energy</span>
                        <span class="pe-cons-value" id="pet-energy">0.000000</span>
                        <span class="pe-cons-alarm" id="pet-energy-alarm"></span>
                        <canvas class="pe-cons-spark" id="pet-spark-energy"></canvas>
                    </div>
                    <div class="pe-conservation-row">
                        <span class="pe-cons-label">|p|</span>
                        <span class="pe-cons-value" id="pet-momentum">0.000000</span>
                        <span class="pe-cons-alarm" id="pet-momentum-alarm"></span>
                        <canvas class="pe-cons-spark" id="pet-spark-momentum"></canvas>
                    </div>
                    <div class="pe-conservation-row">
                        <span class="pe-cons-label">|L|</span>
                        <span class="pe-cons-value" id="pet-angmom">0.000000</span>
                        <span class="pe-cons-alarm" id="pet-angmom-alarm"></span>
                        <canvas class="pe-cons-spark" id="pet-spark-angmom"></canvas>
                    </div>
                    <div class="pe-conservation-row">
                        <span class="pe-cons-label">Drift</span>
                        <span class="pe-cons-value" id="pet-drift">0.0000%</span>
                        <span class="pe-cons-alarm" id="pet-drift-alarm"></span>
                        <canvas class="pe-cons-spark" id="pet-spark-drift"></canvas>
                    </div>
                </div>

                <!-- Section 2: System Properties -->
                <details class="pe-telem-details" open>
                    <summary class="pe-telem-summary">System Properties</summary>
                    <div class="panel-grid panel-grid-3 pe-telemetry-row-gap">
                        <div class="card">
                            <div class="card-title">Particles</div>
                            <div class="stat-value" id="pet-count">0</div>
                        </div>
                        <div class="card">
                            <div class="card-title">Virial 2K/|U|</div>
                            <div class="stat-value" id="pet-virial">&mdash;</div>
                        </div>
                        <div class="card">
                            <div class="card-title">Temperature <span class="unit-hint">(MeV)</span></div>
                            <div class="stat-value" id="pet-temp">&mdash;</div>
                        </div>
                    </div>
                    <div class="panel-grid panel-grid-3 pe-telemetry-row-gap">
                        <div class="card">
                            <div class="card-title">RMS Velocity <span class="unit-hint">(c)</span></div>
                            <div class="stat-value" id="pet-vrms">&mdash;</div>
                        </div>
                        <div class="card">
                            <div class="card-title">System Radius <span class="unit-hint">(lu)</span></div>
                            <div class="stat-value" id="pet-radius">&mdash;</div>
                        </div>
                        <div class="card">
                            <div class="card-title">Tick</div>
                            <div class="stat-value" id="pet-tick">0</div>
                        </div>
                    </div>
                    <div class="panel-grid panel-grid-3 pe-telemetry-row-gap">
                        <div class="card">
                            <div class="card-title">KE <span class="unit-hint">(MeV)</span></div>
                            <div class="stat-value" id="pet-ke">0.000000</div>
                        </div>
                        <div class="card">
                            <div class="card-title">PE <span class="unit-hint">(MeV)</span></div>
                            <div class="stat-value" id="pet-pe">0.000000</div>
                        </div>
                        <div class="card">
                            <div class="card-title">CoM</div>
                            <div class="stat-value pet-com-value" id="pet-com">&mdash;</div>
                        </div>
                    </div>
                    <div class="panel-grid panel-grid-2 pe-telemetry-row-gap-tight">
                        <div class="card">
                            <div class="card-title">PE (Coulomb) <span class="unit-hint">(MeV)</span></div>
                            <div class="stat-value" id="pet-pe-coulomb">0.000000</div>
                        </div>
                        <div class="card">
                            <div class="card-title">PE (Gravity) <span class="unit-hint">(MeV)</span></div>
                            <div class="stat-value" id="pet-pe-gravity">0.000000</div>
                        </div>
                    </div>
                </details>

                <!-- Section 3: Per-Particle Table -->
                <details class="pe-telem-details">
                    <summary class="pe-telem-summary">Per-Particle Data</summary>
                    <div class="pe-table-wrap pe-telemetry-row-gap">
                        <table class="pe-table">
                            <thead>
                                <tr>
                                    <th>ID</th>
                                    <th>q</th>
                                    <th>m <span class="unit-hint">(MeV)</span></th>
                                    <th>|r| <span class="unit-hint">(lu)</span></th>
                                    <th>|v| <span class="unit-hint">(c)</span></th>
                                    <th>|a|</th>
                                    <th>|F| <span class="unit-hint">(Pl)</span></th>
                                    <th>KE <span class="unit-hint">(MeV)</span></th>
                                    <th>Lk</th>
                                </tr>
                            </thead>
                            <tbody id="pet-particle-tbody"></tbody>
                        </table>
                    </div>
                </details>

                <!-- Section 4: Orbital Mechanics (2-body) -->
                <details class="pe-telem-details" id="pet-orbital-section" style="display:none">
                    <summary class="pe-telem-summary">Orbital Mechanics (2-body)</summary>
                    <div class="panel-grid panel-grid-3 pe-telemetry-row-gap">
                        <div class="card">
                            <div class="card-title">Separation r <span class="unit-hint">(lu)</span></div>
                            <div class="stat-value" id="pet-orb-r">&mdash;</div>
                        </div>
                        <div class="card">
                            <div class="card-title">Reduced Mass &mu; <span class="unit-hint">(MeV)</span></div>
                            <div class="stat-value" id="pet-orb-mu">&mdash;</div>
                        </div>
                        <div class="card">
                            <div class="card-title">Spec. Ang. Mom h</div>
                            <div class="stat-value" id="pet-orb-h">&mdash;</div>
                        </div>
                    </div>
                    <div class="panel-grid panel-grid-3 pe-telemetry-row-gap">
                        <div class="card">
                            <div class="card-title">Semi-major a <span class="unit-hint">(lu)</span></div>
                            <div class="stat-value" id="pet-orb-a">&mdash;</div>
                        </div>
                        <div class="card">
                            <div class="card-title">Eccentricity e</div>
                            <div class="stat-value" id="pet-orb-e">&mdash;</div>
                        </div>
                        <div class="card">
                            <div class="card-title">Period T</div>
                            <div class="stat-value" id="pet-orb-T">&mdash;</div>
                        </div>
                    </div>
                    <div class="panel-grid panel-grid-2 pe-telemetry-row-gap">
                        <div class="card">
                            <div class="card-title">Vis-viva Check</div>
                            <div class="stat-value pet-visviva-value" id="pet-orb-visviva">&mdash;</div>
                        </div>
                        <div class="card">
                            <div class="card-title">Phase Space (r, v_r)</div>
                            <canvas id="pet-phase-space" class="pet-phase-canvas"></canvas>
                        </div>
                    </div>
                </details>

                <!-- Section 5: Time-Series Charts -->
                <details class="pe-telem-details">
                    <summary class="pe-telem-summary">Time-Series</summary>
                    <div class="pe-telemetry-row-gap">
                        <div class="pe-ts-label">Energy <span class="unit-hint">(MeV)</span> &mdash; KE <span class="pe-ts-legend-ke">&bull;</span> PE <span class="pe-ts-legend-pe">&bull;</span> Total <span class="pe-ts-legend-total">&bull;</span></div>
                        <canvas class="pe-ts-chart" id="pet-ts-energy"></canvas>
                        <div class="pe-ts-label">|p| <span class="pe-ts-legend-mom">&bull;</span></div>
                        <canvas class="pe-ts-chart" id="pet-ts-momentum"></canvas>
                        <div class="pe-ts-label">|L| <span class="pe-ts-legend-angm">&bull;</span></div>
                        <canvas class="pe-ts-chart" id="pet-ts-angmom"></canvas>
                        <div class="pe-ts-label">Virial 2K/|U| <span class="pe-ts-legend-vir">&bull;</span></div>
                        <canvas class="pe-ts-chart" id="pet-ts-virial"></canvas>
                    </div>
                </details>
            </div>
            <!-- Scale 2+3 AE diagnostics (atoms & molecules share AtomEngine) -->
            <div class="panel-grid panel-grid-4 scale-ae ae-diag-row">
                <div class="card">
                    <div class="card-title">Atom Count</div>
                    <div class="stat-value" id="ae-diag-count">0</div>
                </div>
                <div class="card scale3-only">
                    <div class="card-title">Bond Count</div>
                    <div class="stat-value" id="ae-diag-bonds">0</div>
                </div>
                <div class="card">
                    <div class="card-title">Kinetic Energy <span class="unit-hint">(sim)</span></div>
                    <div class="stat-value" id="ae-diag-ke">0.0000</div>
                </div>
                <div class="card">
                    <div class="card-title">Total Energy <span class="unit-hint">(sim)</span></div>
                    <div class="stat-value" id="ae-diag-etotal">0.0000</div>
                </div>
            </div>
            <div class="panel-grid panel-grid-4 scale-ae ae-diag-row">
                <div class="card">
                    <div class="card-title">PE (Ionic) <span class="unit-hint">(sim)</span></div>
                    <div class="stat-value" id="ae-diag-pe-ionic">0.0000</div>
                </div>
                <div class="card">
                    <div class="card-title">PE (Van der Waals) <span class="unit-hint">(sim)</span></div>
                    <div class="stat-value" id="ae-diag-pe-vdw">0.0000</div>
                </div>
                <div class="card scale3-only">
                    <div class="card-title">PE (Bonds) <span class="unit-hint">(sim)</span></div>
                    <div class="stat-value" id="ae-diag-pe-bond">0.0000</div>
                </div>
                <div class="card">
                    <div class="card-title">Temperature <span class="unit-hint">(sim)</span></div>
                    <div class="stat-value" id="ae-diag-temp">0.0000</div>
                </div>
            </div>
            <div class="panel-grid panel-grid-4 scale-ae ae-diag-row">
                <div class="card">
                    <div class="card-title">Momentum |p|</div>
                    <div class="stat-value" id="ae-diag-momentum">0.0000</div>
                </div>
                <div class="card">
                    <div class="card-title">AE Tick</div>
                    <div class="stat-value" id="ae-diag-tick">0</div>
                </div>
                <div class="card">
                    <div class="card-title">Energy Drift</div>
                    <div class="stat-value" id="ae-diag-drift">0.00%</div>
                </div>
                <div class="card">
                    <div class="card-title">Atomic Mass</div>
                    <div class="stat-value" id="ae-diag-mass">--</div>
                </div>
            </div>
            <!-- Nuclear physics row -->
            <div class="panel-grid panel-grid-4 scale-ae ae-diag-row">
                <div class="card">
                    <div class="card-title">Nuclear B.E.</div>
                    <div class="stat-value" id="ae-diag-nbe">--</div>
                </div>
                <div class="card">
                    <div class="card-title">B/A (MeV)</div>
                    <div class="stat-value" id="ae-diag-ba">--</div>
                </div>
                <div class="card">
                    <div class="card-title">Electron B.E.</div>
                    <div class="stat-value" id="ae-diag-ebe">--</div>
                </div>
                <div class="card">
                    <div class="card-title">Mass (K<sub>B</sub>)</div>
                    <div class="stat-value" id="ae-diag-mass-kb">--</div>
                </div>
            </div>
        </div>
    `;
}
