/** Top-level Verify panel skeleton. Body is filled in by component.js. */
export function getVerifyPanelTemplate() {
    return `
        <div class="panel" id="panel-verification-lab">
            <div class="verify-shell">
                <div class="verify-kicker">Verify</div>
                <div id="verify-header-slot"></div>
                <div class="verify-filters" id="verify-filters">
                    <button class="verify-filter active" data-filter="all">All</button>
                    <button class="verify-filter" data-filter="hard">Hard predictions</button>
                    <button class="verify-filter" data-filter="parametric">Parametric</button>
                    <button class="verify-filter" data-filter="unpredicted">Unpredicted</button>
                    <button class="verify-export" id="verify-export-btn" title="Download raw manifest JSON">Export manifest</button>
                </div>
                <div id="verify-tiers-slot" class="verify-tiers"></div>
                <div id="verify-error-slot" class="verify-error" hidden></div>
            </div>
        </div>
    `;
}
