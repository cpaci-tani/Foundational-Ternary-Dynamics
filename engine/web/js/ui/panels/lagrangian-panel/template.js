export function getLagrangianPanelTemplate() {
    return `
        <div class="mode-unavailable scale1-only">Lagrangian diagnostics available in Scale 0 only.</div>
        <div class="mode-unavailable scale-ae">Lagrangian diagnostics available in Scale 0 only.</div>
        <div class="scale0-only lag-layout">
            <div class="lag-chart-col">
                <div class="lag-charts-grid"></div>
                <div class="lag-term-row-host"></div>
            </div>
            <div class="lag-data-col">
                <section class="lag-observer-baseline"
                    data-lag-observer-status="unavailable"
                    aria-labelledby="lag-observer-baseline-title">
                    <h3 class="diag-section-title" id="lag-observer-baseline-title">
                        <span class="diag-section-title-text">Empty-control observer reference</span>
                    </h3>
                    <table class="diag-table diag-table-static">
                        <colgroup>
                            <col class="diag-col-metric">
                            <col class="diag-col-value">
                            <col class="diag-col-unit">
                        </colgroup>
                        <thead>
                            <tr>
                                <th scope="col">Metric</th>
                                <th scope="col">Value</th>
                                <th scope="col">Unit</th>
                            </tr>
                        </thead>
                        <tbody>
                            <tr class="diag-data-row diag-band-odd">
                                <td class="diag-metric">Born-Infeld observer baseline</td>
                                <td class="diag-value" data-lag-observer-value data-value="">—</td>
                                <td class="diag-unit">E*</td>
                            </tr>
                            <tr class="diag-data-row diag-band-even">
                                <td class="diag-metric">Baseline-subtracted excitation Δℒ</td>
                                <td class="diag-value" data-lag-excitation-value data-value="">—</td>
                                <td class="diag-unit">E*</td>
                            </tr>
                        </tbody>
                    </table>
                    <p class="lag-observer-status" data-lag-observer-status-text aria-live="polite">
                        Awaiting a current empty-scenario Lagrangian sample. Unavailable is not zero.
                    </p>
                    <p class="lag-observer-interpretation">
                        The empty-control Born-Infeld value is a state-independent observer/functional
                        offset in this accounting. It is not physical vacuum energy or zero-point energy.
                        Here Δℒ = ℒ total − Born-Infeld observer baseline.
                    </p>
                </section>
            </div>
        </div>
    `;
}
