export function getLagrangianPanelTemplate() {
    return `
        <div class="mode-unavailable scale1-only">Lagrangian diagnostics available in Scale 0 only.</div>
        <div class="mode-unavailable scale-ae">Lagrangian diagnostics available in Scale 0 only.</div>
        <div class="scale0-only lag-layout">
            <div class="lag-chart-col">
                <div class="chart-card is-mounted">
                    <header class="chart-card-head">
                        <h3 class="chart-card-title">Lagrangian Density (Stacked Area)</h3>
                        <button type="button" class="chart-card-expand"
                            title="View fullscreen (Esc to close)"
                            aria-label="View Lagrangian chart fullscreen">⛶</button>
                    </header>
                    <div class="chart-card-plot" id="lag-plot-host"></div>
                    <div class="lag-term-row-host"></div>
                </div>
            </div>
            <div class="lag-data-col"></div>
        </div>
    `;
}
