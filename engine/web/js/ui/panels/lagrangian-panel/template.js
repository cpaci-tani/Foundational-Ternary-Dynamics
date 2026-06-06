export function getLagrangianPanelTemplate() {
    return `
        <div class="mode-unavailable scale1-only">Lagrangian diagnostics available in Scale 0 only.</div>
        <div class="mode-unavailable scale-ae">Lagrangian diagnostics available in Scale 0 only.</div>
        <div class="scale0-only lag-layout">
            <div class="lag-chart-col">
                <div class="lag-charts-grid"></div>
                <div class="lag-term-row-host"></div>
            </div>
            <div class="lag-data-col"></div>
        </div>
    `;
}
