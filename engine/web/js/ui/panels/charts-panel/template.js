export function getChartsPanelTemplate() {
    return `
        <div class="charts-row">
            <div class="chart-container">
                <div class="chart-title">Flux &amp; Energy</div>
                <canvas id="chart-flux-energy"></canvas>
            </div>
            <div class="chart-container">
                <div class="chart-title">Particle Count</div>
                <canvas id="chart-particles"></canvas>
            </div>
            <div class="chart-container">
                <div class="chart-title">Charge Balance</div>
                <canvas id="chart-charge" class="stat-sparkline u-sparkline-canvas"></canvas>
            </div>
        </div>
        <div class="charts-row">
            <div class="chart-container">
                <div class="chart-title">E vs B Field Energy</div>
                <canvas id="chart-eb-energy" class="stat-sparkline u-sparkline-canvas"></canvas>
            </div>
            <div class="chart-container">
                <div class="chart-title">Gauss Violation</div>
                <canvas id="chart-gauss" class="stat-sparkline u-sparkline-canvas"></canvas>
            </div>
            <div class="chart-container">
                <div class="chart-title">Entropy</div>
                <canvas id="chart-entropy" class="stat-sparkline u-sparkline-canvas"></canvas>
            </div>
        </div>
    `;
}
