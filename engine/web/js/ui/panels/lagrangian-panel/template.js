export function getLagrangianPanelTemplate() {
    return `
        <div class="mode-unavailable scale1-only">Lagrangian diagnostics available in Scale 0 only.</div>
        <div class="mode-unavailable scale-ae">Lagrangian diagnostics available in Scale 0 only.</div>
        <div class="scale0-only lag-layout">
            <div>
                <div class="chart-container">
                    <div class="chart-title">Lagrangian Density (Stacked Area)</div>
                    <canvas id="chart-lagrangian" class="lag-chart-canvas"></canvas>
                    <div class="chart-legend">
                        <span class="chart-legend-item" style="--color:var(--legend-bi)">Born-Infeld</span>
                        <span class="chart-legend-item" style="--color:var(--legend-coupling)">Coupling</span>
                        <span class="chart-legend-item" style="--color:var(--legend-velocity)">Velocity</span>
                        <span class="chart-legend-item" style="--color:var(--legend-gauss)">Gauss</span>
                        <span class="chart-legend-item" style="--color:var(--legend-dissipation)">Dissipation</span>
                    </div>
                </div>
                <div class="lag-term-row">
                    <label class="lag-term-toggle" data-term="field-kinetic"><input type="checkbox" id="lt-field-kinetic" checked>Field KE</label>
                    <label class="lag-term-toggle" data-term="field-gradient"><input type="checkbox" id="lt-field-gradient" checked>Gradient</label>
                    <label class="lag-term-toggle" data-term="bi"><input type="checkbox" id="lt-bi" checked>Born-Infeld</label>
                    <label class="lag-term-toggle" data-term="coupling"><input type="checkbox" id="lt-coupling" checked>Coupling</label>
                    <label class="lag-term-toggle" data-term="velocity"><input type="checkbox" id="lt-velocity" checked>Velocity</label>
                    <label class="lag-term-toggle" data-term="gauss"><input type="checkbox" id="lt-gauss" checked>Gauss</label>
                    <label class="lag-term-toggle" data-term="dissipation"><input type="checkbox" id="lt-dissipation" checked>Dissipation</label>
                </div>
            </div>
            <div class="lag-data-column">
                <div class="combo-section-label">Action &amp; Constraints</div>
                <dl class="inspector-grid">
                    <dt>Action S</dt>
                    <dd id="lag-action">--</dd>
                    <dt>Gauss ||div J-s||</dt>
                    <dd id="lag-gauss-viol">--</dd>
                    <dt>Max Gauss err</dt>
                    <dd id="lag-max-gauss">--</dd>
                    <dt>Total |J|</dt>
                    <dd id="lag-flux-mag">--</dd>
                    <dt>Wave KE</dt>
                    <dd id="lag-wave-ke">--</dd>
                    <dt>Manifested</dt>
                    <dd id="lag-manifested">--</dd>
                    <dt>Locked</dt>
                    <dd id="lag-locked">--</dd>
                </dl>
                <div class="combo-section-label">Ontic Constants</div>
                <dl class="inspector-grid">
                    <dt>G*</dt>
                    <dd id="const-gstar">2.9586751</dd>
                    <dt>1/&alpha;</dt>
                    <dd id="const-alpha-inv">137.03600</dd>
                    <dt>&alpha;</dt>
                    <dd id="const-alpha">0.0072974</dd>
                    <dt>K<sub>B</sub></dt>
                    <dd id="const-kb">0.5110000</dd>
                    <dt>G<sub>N</sub></dt>
                    <dd id="const-gn">0.0100000</dd>
                    <dt>g<sub>c</sub></dt>
                    <dd id="const-gc">0.0854245</dd>
                    <dt>N<sub>c</sub></dt>
                    <dd id="const-nc">3</dd>
                    <dt>N<sub>eff</sub></dt>
                    <dd id="const-neff">13</dd>
                </dl>
            </div>
        </div>
    `;
}
