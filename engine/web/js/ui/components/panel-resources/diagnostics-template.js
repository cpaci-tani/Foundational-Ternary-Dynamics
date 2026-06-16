export function getDiagnosticsPanelTemplate() {
    return `
        <div class="panel" id="panel-diagnostics">
            <!-- Scale 0 tables: DiagnosticsPanelComponent (descriptors/scale0.js) -->
            <!-- Scale 1 PE Telemetry Panel -->
            <div id="pe-telemetry" class="scale1-only">
                <!--
                  Conservation Laws + System Properties were removed (2026-06-15):
                  energy / KE / PE / Coulomb PE / Gravity PE / total / drift /
                  |p| / |L| / virial / temperature / RMS v / radius now live ONLY
                  in the descriptor tables above (.diag-scale1-root: Active
                  Hamiltonian + Conservation + Forces & Geometry). This legacy
                  block keeps just the non-duplicated drill-down surfaces below.
                -->

                <!-- Per-Particle Table -->
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
                <div class="card">
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
                <div class="card">
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
