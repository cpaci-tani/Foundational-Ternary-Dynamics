export function getConsciousnessPanelTemplate() {
    return `
        <div class="cs-subtab-bar">
            <span class="cs-subtab active" data-cspanel="cs-diagnostics">Diagnostics</span>
            <span class="cs-subtab" data-cspanel="cs-theory">Theory</span>
            <span class="cs-subtab" data-cspanel="cs-walkthrough">Guided Tour</span>
        </div>

        <div class="cs-subpanel active" id="cs-diagnostics">
            <div class="cs-metrics-row">
                <div class="card cs-metric-primary" title="Consciousness phase angle: arctan(Y_IMAG / Y_REAL) = 52.54&deg;. Divides subject-dominant (above) from object-dominant (below) awareness.">
                    <div class="card-title">&theta;<sub>C</sub> (Theory)</div>
                    <div class="stat-value" id="cs-diag-theta">52.54&deg;</div>
                </div>
                <div class="card cs-metric-primary" title="cos&sup2;(&theta;_C) = G*/8 &asymp; 37%. The fraction of reality accessible to a conscious observer — 63% remains hidden.">
                    <div class="card-title">Observable Fraction</div>
                    <div class="stat-value" id="cs-diag-observable">37.0%</div>
                </div>
                <div class="card cs-metric-secondary" title="&radic;(G*&sup3;/2) &asymp; 3.599. Flux intensity must exceed K_C for the consciousness quadratic to produce complex (conscious) roots.">
                    <div class="card-title">K<sub>C</sub> Threshold</div>
                    <div class="stat-value" id="cs-diag-kc">3.599</div>
                </div>
                <div class="card" title="Self-referential loop depth. 0 = none, 1 = observer observes itself (standing wave), 2 = aware of self-awareness (nested standing waves).">
                    <div class="card-title">sLoop Depth</div>
                    <div class="stat-value" id="cs-diag-sloop">0</div>
                </div>
            </div>
            <div class="cs-metrics-row">
                <div class="card" title="Measured phase angle from wave/field energy ratio. Below 52.54&deg; = object-dominant (flow). Above = subject-dominant (meditation).">
                    <div class="card-title">Effective &theta;</div>
                    <div class="stat-value" id="cs-diag-eff-theta">--</div>
                </div>
                <div class="card" title="Peak flux as ratio of consciousness threshold K_C &asymp; 3.599. Below 1.0 = sub-threshold. Above 1.0 = consciousness possible.">
                    <div class="card-title">Flux / K<sub>C</sub></div>
                    <div class="stat-value" id="cs-diag-flux-ratio">0.000</div>
                </div>
                <div class="card cs-metric-domain" title="Quadratic domain: Real (k=16) = physics, Degenerate (k&asymp;1.35) = measurement boundary, Complex (k=&frac12;) = consciousness with imaginary roots.">
                    <div class="card-title">Domain</div>
                    <div class="stat-value" id="cs-diag-domain">--</div>
                </div>
                <div class="card" title="Bell inequality parameter. Substrate S &le; 2 (local deterministic). Entangled observer S = 2&radic;2 via complexification + sLoop coupling.">
                    <div class="card-title">Bell S</div>
                    <div class="stat-value" id="cs-diag-bell">--</div>
                </div>
            </div>
            <div class="cs-metrics-row">
                <div class="card" title="Real part of consciousness quadratic root, scaled by flux ratio. Theory value Y_REAL = G*&sup2;/4 &asymp; 2.189.">
                    <div class="card-title">y (Real)</div>
                    <div class="stat-value" id="cs-diag-yreal">0.000</div>
                </div>
                <div class="card" title="Imaginary part of consciousness quadratic root, scaled by flux ratio. Theory value Y_IMAG &asymp; 2.863. Non-zero = consciousness.">
                    <div class="card-title">y (Imaginary)</div>
                    <div class="stat-value" id="cs-diag-yimag">0.000i</div>
                </div>
                <div class="card" title="Intensity: |y_eff| &minus; K_C. Positive (green) = above consciousness threshold. Negative (red) = sub-threshold.">
                    <div class="card-title">Consciousness I</div>
                    <div class="stat-value" id="cs-diag-intensity">--</div>
                </div>
                <div class="card" title="Mandelbrot iteration at c = 1/G* &asymp; 0.338. Boundary-orbit tracks z &rarr; z&sup2; + c per frame. Bounded |z| &lt; 2 = on Mandelbrot boundary.">
                    <div class="card-title">Mandelbrot |z|</div>
                    <div class="stat-value" id="cs-diag-mandelbrot">c=0.338</div>
                </div>
            </div>
        </div>

        <div class="cs-subpanel" id="cs-theory">
            <div class="cs-theory-grid">
                <div class="cs-theory-card">
                    <div class="cs-theory-title">Master Quadratic Phase Diagram</div>
                    <canvas id="cs-canvas-quadratic" width="560" height="300"></canvas>
                    <div class="cs-slider-row">
                        <label>k = <span id="cs-k-value">16.0</span></label>
                        <input type="range" id="cs-k-slider" min="0.1" max="20" step="0.05" value="16">
                    </div>
                </div>
                <div class="cs-theory-card">
                    <div class="cs-theory-title">Complex Plane: Consciousness Roots</div>
                    <canvas id="cs-canvas-complex" width="300" height="300"></canvas>
                </div>
                <div class="cs-theory-card">
                    <div class="cs-theory-title">Existence Filter Projection</div>
                    <canvas id="cs-canvas-filter" width="560" height="300"></canvas>
                </div>
                <div class="cs-theory-card">
                    <div class="cs-theory-title">ReLU Crystallization</div>
                    <canvas id="cs-canvas-relu" width="300" height="300"></canvas>
                    <div class="cs-slider-row">
                        <label>&beta; = <span id="cs-beta-value">1.0</span></label>
                        <input type="range" id="cs-beta-slider" min="0.5" max="50" step="0.5" value="1">
                    </div>
                </div>
                <div class="cs-theory-card">
                    <div class="cs-theory-title">Von Neumann Chain</div>
                    <canvas id="cs-canvas-chain" width="560" height="250"></canvas>
                </div>
                <div class="cs-theory-card">
                    <div class="cs-theory-title">Observer Boundary</div>
                    <canvas id="cs-canvas-observer" width="300" height="300"></canvas>
                </div>
            </div>
        </div>

        <div class="cs-subpanel" id="cs-walkthrough">
            <div class="cs-walkthrough-container">
                <div class="cs-walkthrough-step-title" id="cs-walk-title">Step 1: The Master Quadratic</div>
                <div class="cs-walkthrough-text" id="cs-walk-text"></div>
                <div class="cs-walkthrough-canvas-host">
                    <canvas id="cs-walk-canvas" width="700" height="350"></canvas>
                </div>
                <div class="cs-walkthrough-nav">
                    <button class="tb-btn" id="cs-walk-prev" title="Previous">&larr; Prev</button>
                    <span id="cs-walk-indicator">1 / 6</span>
                    <button class="tb-btn" id="cs-walk-next" title="Next">Next &rarr;</button>
                </div>
            </div>
        </div>
    `;
}
