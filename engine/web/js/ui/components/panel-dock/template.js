export function getPanelDockShellTemplate() {
    return `
        <div class="panel-dock-head">
            <div class="panel-dock-head-copy">
                <div class="panel-dock-kicker">Dashboard</div>
                <div class="panel-dock-title" id="panel-dock-active-title">Controls</div>
            </div>
            <button class="panel-dock-hide-btn" id="btn-panel-hide-mobile" type="button">Hide</button>
        </div>
        <div class="panel-dock-body" data-panel-dock-body></div>
    `;
}
