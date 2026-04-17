/**
 * Charts panel shell — chip picker + grid. Charts are appended dynamically
 * by the component at init time based on the scale0 descriptor.
 */
export function getChartsPanelTemplate() {
    return `
        <div class="charts-chip-strip" role="toolbar" aria-label="Chart visibility"></div>
        <div class="charts-grid"></div>
    `;
}
