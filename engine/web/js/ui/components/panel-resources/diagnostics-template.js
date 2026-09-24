export function getDiagnosticsPanelTemplate() {
    return `
        <div class="panel" id="panel-diagnostics">
            <!-- Scale 0 tables: DiagnosticsPanelComponent (descriptors/scale0.js) -->
            <!-- Scale 1: legacy PE Telemetry Panel RETIRED (2026-07 revision).
                 All Scale-1 quantities live in the descriptor tables
                 (.diag-scale1-root) backed by telemetryHub — the single
                 Scale-1 diagnostics surface. -->
            <!-- Scale 2+3 AE: runtime energy/count cards removed (2026-06-15) — those
                 quantities live in the descriptor table (.diag-scale2-root / .diag-scale3-root). Keep only
                 the supplemental nuclear/binding row for element-level pedagogy. -->
            <details class="scale-ae ae-nuclear-diag" id="ae-nuclear-diag">
                <summary class="ae-nuclear-diag-summary">Nuclear &amp; binding energies</summary>
                <div class="panel-grid panel-grid-4 ae-diag-row ae-diag-nuclear" data-panel-grid>
                    <div class="card">
                        <div class="card-title">Atomic Mass</div>
                        <div class="stat-value" id="ae-diag-mass">--</div>
                    </div>
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
                <div class="card ae-diag-nuclear ae-diag-chart-card">
                    <div class="card-title" title="Binding energy per nucleon (B/A) vs. mass number (A), all 118 elements — the classic curve peaking at Fe-56 that explains why fusion releases energy for light nuclei and fission for heavy ones.">B/A vs. mass number (all 118 elements)</div>
                    <canvas id="ae-diag-ba-chart" class="ae-diag-chart" width="276" height="140"></canvas>
                </div>
            </details>
        </div>
    `;
}
