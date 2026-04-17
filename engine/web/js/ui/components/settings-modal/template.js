export function getSettingsModalTemplate() {
    return `
        <div id="settings-modal" role="dialog" aria-modal="true" aria-label="Settings">
            <div class="settings-box">
                <div class="settings-header">
                    <div class="settings-title">Settings</div>
                    <button id="settings-close" class="settings-close-btn" aria-label="Close settings" title="Close (Escape)">&times;</button>
                </div>

                <div class="settings-section">
                    <div class="settings-label-row">
                        <label class="settings-label">Appearance</label>
                        <p class="settings-copy">Control the shell look, reading density, and layout width.</p>
                    </div>

                    <div class="settings-subsection">
                        <label class="settings-label">Theme</label>
                        <div class="settings-theme-grid">
                            <div class="theme-swatch active" data-theme="default" title="Midnight (default)">
                                <div class="theme-swatch-colors" data-theme="default"><div></div><div></div><div></div><div></div></div>
                                <div class="theme-swatch-name">Midnight</div>
                            </div>
                            <div class="theme-swatch" data-theme="abyss" title="Abyss (OLED dark)">
                                <div class="theme-swatch-colors" data-theme="abyss"><div></div><div></div><div></div><div></div></div>
                                <div class="theme-swatch-name">Abyss</div>
                            </div>
                            <div class="theme-swatch" data-theme="nord" title="Nord (cool blue)">
                                <div class="theme-swatch-colors" data-theme="nord"><div></div><div></div><div></div><div></div></div>
                                <div class="theme-swatch-name">Nord</div>
                            </div>
                            <div class="theme-swatch" data-theme="light" title="Light mode">
                                <div class="theme-swatch-colors" data-theme="light"><div></div><div></div><div></div><div></div></div>
                                <div class="theme-swatch-name">Light</div>
                            </div>
                            <div class="theme-swatch" data-theme="parchment" title="Parchment (warm tan)">
                                <div class="theme-swatch-colors" data-theme="parchment"><div></div><div></div><div></div><div></div></div>
                                <div class="theme-swatch-name">Parchment</div>
                            </div>
                        </div>
                    </div>

                    <div class="settings-subsection">
                        <label class="settings-label">UI Scale</label>
                        <div class="settings-scale-row">
                            <input type="range" id="settings-ui-scale" min="0.8" max="1.6" step="0.05" value="1.0">
                            <span id="settings-scale-val" class="settings-scale-val">100%</span>
                        </div>
                        <div class="settings-presets">
                            <button class="settings-preset" data-scale="0.85">Compact</button>
                            <button class="settings-preset active" data-scale="1.0">Default</button>
                            <button class="settings-preset" data-scale="1.2">Large</button>
                            <button class="settings-preset" data-scale="1.4">X-Large</button>
                        </div>
                    </div>

                    <div class="settings-subsection">
                        <label class="settings-label">Density</label>
                        <div class="settings-choice-grid settings-choice-grid-compact">
                            <button class="settings-choice active" type="button" data-setting="density" data-value="comfortable">Comfortable</button>
                            <button class="settings-choice" type="button" data-setting="density" data-value="compact">Compact</button>
                        </div>
                        <p class="settings-help">Compact trims spacing in the toolbar, status bar, panels, and library list.</p>
                    </div>

                    <div class="settings-subsection">
                        <label class="settings-label">Panel Width</label>
                        <div class="settings-choice-grid">
                            <button class="settings-choice" type="button" data-setting="panel-width" data-value="narrow">Focused</button>
                            <button class="settings-choice active" type="button" data-setting="panel-width" data-value="standard">Standard</button>
                            <button class="settings-choice" type="button" data-setting="panel-width" data-value="wide">Wide</button>
                        </div>
                        <p class="settings-help">Change how wide the docked panel surface grows on larger screens.</p>
                    </div>
                </div>

                <div class="settings-section">
                    <div class="settings-label-row">
                        <label class="settings-label">Experience</label>
                        <p class="settings-copy">Tune motion, helper surfaces, and what chrome stays visible.</p>
                    </div>

                    <div class="settings-subsection">
                        <label class="settings-label">Motion</label>
                        <div class="settings-choice-grid">
                            <button class="settings-choice active" type="button" data-setting="motion" data-value="system">System</button>
                            <button class="settings-choice" type="button" data-setting="motion" data-value="full">Full</button>
                            <button class="settings-choice" type="button" data-setting="motion" data-value="reduced">Reduced</button>
                        </div>
                        <p class="settings-help">Use your system preference, keep all motion, or reduce animation and transitions.</p>
                    </div>

                    <div class="settings-subsection">
                        <label class="settings-label">Tooltips</label>
                        <div class="settings-choice-grid settings-choice-grid-compact">
                            <button class="settings-choice active" type="button" data-setting="tooltips" data-value="on">On</button>
                            <button class="settings-choice" type="button" data-setting="tooltips" data-value="off">Off</button>
                        </div>
                        <p class="settings-help">Disable hover help if you want a quieter interface while navigating dense controls.</p>
                    </div>

                    <div class="settings-subsection">
                        <label class="settings-label">Status Bar</label>
                        <div class="settings-choice-grid settings-choice-grid-compact">
                            <button class="settings-choice active" type="button" data-setting="status-bar" data-value="shown">Shown</button>
                            <button class="settings-choice" type="button" data-setting="status-bar" data-value="hidden">Hidden</button>
                        </div>
                        <p class="settings-help">Hide the global runtime strip to give the panel dock a little more breathing room.</p>
                    </div>
                </div>

                <div class="settings-footer">
                    <button id="settings-reset">Reset All</button>
                </div>
            </div>
        </div>
    `;
}
