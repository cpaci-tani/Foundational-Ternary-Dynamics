/**
 * Scene panel template — 4 sections of curated render controls.
 * Every control has data-scene-control="<id>" for event binding and
 * display-value readouts use data-scene-readout.
 */

import { SceneAdapter } from './adapter.js';

const D = SceneAdapter.DEFAULTS;

export function getScenePanelTemplate() {
    return `
            <div class="scene-shell">
                <header class="scene-header">
                    <div class="scene-kicker">Scene</div>
                    <h2 class="scene-title">Render controls</h2>
                    <p class="scene-copy">
                        Camera, lighting, post-processing, and environment knobs for the
                        shared viewport. Changes persist across reloads. Applies to
                        Scales 0–3; other scales use separate renderers and are
                        unaffected for now.
                    </p>
                </header>

                <section class="scene-section" data-scene-section="camera">
                    <h3 class="scene-section-title">Camera</h3>
                    <div class="scene-control">
                        <label for="scene-fov">Field of view</label>
                        <input type="range" id="scene-fov" data-scene-control="fov"
                            min="15" max="90" step="1" value="${D.fov}">
                        <span class="scene-readout" data-scene-readout="fov">${D.fov}°</span>
                    </div>
                    <div class="scene-control">
                        <label for="scene-orbit-rotate">Orbit rotate speed</label>
                        <input type="range" id="scene-orbit-rotate" data-scene-control="orbitRotateSpeed"
                            min="0.15" max="1.5" step="0.05" value="${D.orbitRotateSpeed}">
                        <span class="scene-readout" data-scene-readout="orbitRotateSpeed">${D.orbitRotateSpeed.toFixed(2)}×</span>
                    </div>
                    <div class="scene-control">
                        <label for="scene-orbit-zoom">Orbit zoom speed</label>
                        <input type="range" id="scene-orbit-zoom" data-scene-control="orbitZoomSpeed"
                            min="0.3" max="3" step="0.1" value="${D.orbitZoomSpeed}">
                        <span class="scene-readout" data-scene-readout="orbitZoomSpeed">${D.orbitZoomSpeed.toFixed(2)}×</span>
                    </div>
                </section>

                <section class="scene-section" data-scene-section="lighting">
                    <h3 class="scene-section-title">Lighting</h3>
                    <div class="scene-control">
                        <label for="scene-ambient-intensity">Ambient intensity</label>
                        <input type="range" id="scene-ambient-intensity" data-scene-control="ambientIntensity"
                            min="0" max="2" step="0.05" value="${D.ambientIntensity}">
                        <span class="scene-readout" data-scene-readout="ambientIntensity">${D.ambientIntensity.toFixed(2)}</span>
                    </div>
                    <div class="scene-control">
                        <label for="scene-ambient-color">Ambient color</label>
                        <input type="color" id="scene-ambient-color" data-scene-control="ambientColor"
                            value="${D.ambientColor}">
                        <span class="scene-readout" data-scene-readout="ambientColor">${D.ambientColor}</span>
                    </div>
                    <div class="scene-control">
                        <label for="scene-key-light">Key light intensity</label>
                        <input type="range" id="scene-key-light" data-scene-control="keyLightIntensity"
                            min="0" max="2" step="0.05" value="${D.keyLightIntensity}">
                        <span class="scene-readout" data-scene-readout="keyLightIntensity">${D.keyLightIntensity.toFixed(2)}</span>
                    </div>
                </section>

                <section class="scene-section" data-scene-section="post">
                    <h3 class="scene-section-title">Post-processing</h3>
                    <div class="scene-control">
                        <label for="scene-exposure">Exposure</label>
                        <input type="range" id="scene-exposure" data-scene-control="exposure"
                            min="0.25" max="2" step="0.05" value="${D.exposure}">
                        <span class="scene-readout" data-scene-readout="exposure">${D.exposure.toFixed(2)}</span>
                    </div>
                    <div class="scene-control scene-control--toggle">
                        <label for="scene-bloom-enabled">Bloom</label>
                        <input type="checkbox" id="scene-bloom-enabled" data-scene-control="bloomEnabled"
                            ${D.bloomEnabled ? 'checked' : ''}>
                        <span class="scene-readout" data-scene-readout="bloomEnabled">${D.bloomEnabled ? 'on' : 'off'}</span>
                    </div>
                    <div class="scene-control" data-scene-dependent="bloomEnabled">
                        <label for="scene-bloom-strength">Bloom strength</label>
                        <input type="range" id="scene-bloom-strength" data-scene-control="bloomStrength"
                            min="0" max="3" step="0.05" value="${D.bloomStrength}">
                        <span class="scene-readout" data-scene-readout="bloomStrength">${D.bloomStrength.toFixed(2)}</span>
                    </div>
                    <div class="scene-control" data-scene-dependent="bloomEnabled">
                        <label for="scene-bloom-threshold">Bloom threshold</label>
                        <input type="range" id="scene-bloom-threshold" data-scene-control="bloomThreshold"
                            min="0" max="1" step="0.01" value="${D.bloomThreshold}">
                        <span class="scene-readout" data-scene-readout="bloomThreshold">${D.bloomThreshold.toFixed(2)}</span>
                    </div>
                </section>

                <section class="scene-section" data-scene-section="environment">
                    <h3 class="scene-section-title">Environment</h3>
                    <div class="scene-control scene-control--toggle">
                        <label for="scene-fog-enabled">Fog</label>
                        <input type="checkbox" id="scene-fog-enabled" data-scene-control="fogEnabled"
                            ${D.fogEnabled ? 'checked' : ''}>
                        <span class="scene-readout" data-scene-readout="fogEnabled">${D.fogEnabled ? 'on' : 'off'}</span>
                    </div>
                    <div class="scene-control" data-scene-dependent="fogEnabled">
                        <label for="scene-fog-density">Fog density</label>
                        <input type="range" id="scene-fog-density" data-scene-control="fogDensity"
                            min="0.001" max="0.08" step="0.001" value="${D.fogDensity}">
                        <span class="scene-readout" data-scene-readout="fogDensity">${D.fogDensity.toFixed(3)}</span>
                    </div>
                    <div class="scene-control" data-scene-dependent="backgroundNone">
                        <label for="scene-bg-color">Background color</label>
                        <input type="color" id="scene-bg-color" data-scene-control="backgroundColor"
                            value="${D.backgroundColor}">
                        <span class="scene-readout" data-scene-readout="backgroundColor">${D.backgroundColor}</span>
                    </div>
                    <div class="scene-control" data-scene-dependent="hdri">
                        <label for="scene-hdri-intensity">HDRI intensity</label>
                        <input type="range" id="scene-hdri-intensity" data-scene-control="hdriIntensity"
                            min="0" max="2" step="0.05" value="${D.hdriIntensity}">
                        <span class="scene-readout" data-scene-readout="hdriIntensity">${D.hdriIntensity.toFixed(2)}</span>
                    </div>
                    <p class="scene-hint scene-hint--bg-none">
                        Background color only applies when the topbar background selector is set to <em>None</em>.
                    </p>
                    <p class="scene-hint scene-hint--hdri">
                        HDRI intensity only applies when an HDRI environment is selected in the topbar background selector.
                    </p>
                </section>

                <footer class="scene-footer">
                    <button type="button" class="scene-reset" id="scene-reset-defaults"
                        title="Reset every Scene control to its default value and clear saved preferences">
                        Reset to defaults
                    </button>
                </footer>
            </div>
    `;
}
