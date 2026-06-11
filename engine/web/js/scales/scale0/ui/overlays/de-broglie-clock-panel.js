// De Broglie internal clock — interactive panel (FTD-0271).
//
// Mounts a floating panel over the Scale-0 viewport for the
// `s0-seed-de-broglie-clock` scenario. The user picks the clock frequency
// omega0, presses "Run clock", and the panel drives the real engine: it sets up
// the k=0 rest mode (a central manifested block with uniform flux), turns on the
// de_broglie_clock toggle so the Klein-Gordon mass term -omega0^2*J acts, ticks,
// and plots the centre flux J_x(t) — a clean cos(omega0*t) oscillation. That is
// the de Broglie INTERNAL CLOCK: FTD's natively-massless flux now carries a
// rest-frame Compton oscillation. The lower panel draws the analytic de Broglie
// relation lambda ∝ 1/v that follows from this clock.
//
// HONESTY (load-bearing): the clock omega0 ∝ M_REST is [IMPOSED] (A0: FTD's
// native flux is massless, no restoring term). Schrodinger + de Broglie are
// textbook Klein-Gordon identities GIVEN the clock — this is lattice
// correctness, NOT an FTD prediction. The footer states this; do not promote it.

const PANEL_ID = 'de-broglie-clock-panel';
const SCENARIO_ID = 's0-seed-de-broglie-clock';
const C2 = 1.0 / 3.0;               // lattice c^2 (C_WAVE = 1/sqrt(3))
const RUN_TICKS = 160;
const BLOCK_HALF = 3;               // 7^3 central manifested block
const J0 = 0.08;

function buildPanel() {
    const p = document.createElement('div');
    p.id = PANEL_ID;
    p.style.cssText = [
        'position:absolute', 'top:12px', 'right:12px', 'z-index:40', 'width:300px',
        'padding:12px 14px', 'border-radius:12px', 'font-family:var(--font-sans,sans-serif)',
        'font-size:12px', 'background:var(--color-background-primary,rgba(20,20,24,0.92))',
        'border:0.5px solid var(--color-border-secondary,rgba(255,255,255,0.25))',
        'color:var(--color-text-primary,#eee)', 'box-shadow:0 2px 12px rgba(0,0,0,0.3)',
    ].join(';');
    p.innerHTML = `
        <div style="font-weight:500;margin-bottom:8px">De Broglie internal clock &mdash; FTD-0271</div>
        <div style="display:flex;align-items:center;gap:8px;margin-bottom:6px">
            <span>&omega;&#8320;</span>
            <input id="${PANEL_ID}-slider" type="range" min="0.05" max="1.0" step="0.01" value="0.30" style="flex:1">
            <span id="${PANEL_ID}-wval" style="width:30px;text-align:right">0.30</span>
        </div>
        <div style="display:flex;gap:6px;margin-bottom:8px">
            <button id="${PANEL_ID}-run" style="flex:1;padding:5px;border-radius:8px;cursor:pointer">Run clock</button>
            <button id="${PANEL_ID}-clear" style="padding:5px 8px;border-radius:8px;cursor:pointer">Clear</button>
        </div>
        <div id="${PANEL_ID}-status" style="margin-bottom:6px;color:var(--color-text-secondary,#aaa);min-height:15px">ready</div>
        <div style="font-size:10px;color:var(--color-text-tertiary,#888);margin-bottom:2px">centre flux J&#8339;(t) &mdash; live engine</div>
        <canvas id="${PANEL_ID}-clock" width="276" height="120" style="width:100%;display:block;border-radius:8px;background:var(--color-background-secondary,rgba(255,255,255,0.04))"></canvas>
        <div style="font-size:10px;color:var(--color-text-tertiary,#888);margin:6px 0 2px">de Broglie &lambda;(v) &mdash; analytic, &lambda;&prop;1/v</div>
        <canvas id="${PANEL_ID}-debroglie" width="276" height="96" style="width:100%;display:block;border-radius:8px;background:var(--color-background-secondary,rgba(255,255,255,0.04))"></canvas>
        <div style="margin-top:8px;font-size:10px;color:var(--color-text-tertiary,#888);line-height:1.45">
            <b>[CONDITIONAL]</b> the clock &omega;&#8320;&prop;M<sub>REST</sub> is <b>IMPOSED</b> (FTD's native
            flux is massless); de Broglie &lambda;&prop;1/v is a Klein-Gordon identity
            <i>given</i> the clock &mdash; not an FTD prediction.
        </div>`;
    return p;
}

export function mountDeBroglieClockPanel(harness) {
    const host = document.getElementById('viewport') || document.body;
    document.getElementById(PANEL_ID)?.remove();
    if (typeof window !== 'undefined' && window.__ftdDeBroglieClockPanel) {
        try { window.__ftdDeBroglieClockPanel.dispose(); } catch (e) { /* noop */ }
    }
    const panel = buildPanel();
    host.appendChild(panel);

    const el = (id) => panel.querySelector(`#${PANEL_ID}-${id}`);
    const slider = el('slider'), wval = el('wval'), status = el('status');
    const clockCv = el('clock'), dbCv = el('debroglie');
    const clockCtx = clockCv.getContext('2d'), dbCtx = dbCv.getContext('2d');
    let trace = [];     // centre J_x(t) samples from the last run
    let busy = false;
    let lastOmega0 = parseFloat(slider.value);

    slider.addEventListener('input', () => {
        wval.textContent = parseFloat(slider.value).toFixed(2);
        lastOmega0 = parseFloat(slider.value);
        drawDeBroglie(lastOmega0);   // analytic curve updates live with omega0
    });
    el('run').addEventListener('click', () => run(parseFloat(slider.value)));
    el('clear').addEventListener('click', () => { trace = []; drawClock(); status.textContent = 'cleared'; });

    function bridge() { return harness.bridge || harness; }

    function setupRestMode(w0) {
        const b = bridge();
        try { b.setupScenario('empty'); } catch (e) { /* noop */ }
        // The de Broglie INTERNAL CLOCK is the k=0 rest-frame oscillation. With
        // wave_propagation OFF, each manifested voxel is an independent SHO
        // J'' = -omega0^2 J at exactly omega0 (no spatial Laplacian to swamp the
        // clock term), so the omega0 slider directly sets the clock frequency.
        // (A localized block WITH the wave term would oscillate at the full KG
        // dispersion sqrt(c^2 k^2 + omega0^2), dominated by the spatial mode for
        // a small cluster — see the C++ test_de_broglie_clock for the clean
        // wave-on whole-lattice k=0 mode.)
        try {
            harness.setToggle('wave_propagation', false);
            harness.setToggle('de_broglie_clock', true);
            harness.setToggle('coupling', false);
            harness.setToggle('damping', false);
            harness.setToggle('selective_damping', false);  // depends on damping
            harness.setToggle('genesis', false);
            harness.setToggle('gauss_projection', false);
            harness.setToggle('forces', false);
            harness.setToggle('lorentz_force', false);       // depends on forces
            harness.setToggle('movement', false);
            harness.setToggle('dual_substrate', false);
            harness.setToggle('weak_transmutation', false);  // requires dual_substrate
            if (typeof b.setOmega0 === 'function') b.setOmega0(w0);
        } catch (e) { /* noop */ }
        const L = harness.getLatticeSize?.() ?? 33;
        const mc = Math.round((L - 1) / 2);
        for (let dx = -BLOCK_HALF; dx <= BLOCK_HALF; dx++)
            for (let dy = -BLOCK_HALF; dy <= BLOCK_HALF; dy++)
                for (let dz = -BLOCK_HALF; dz <= BLOCK_HALF; dz++) {
                    harness.injectParticle(mc + dx, mc + dy, mc + dz, +1);
                    harness.injectFlux(mc + dx, mc + dy, mc + dz, J0, 0, 0);
                }
        return mc;
    }

    async function run(w0) {
        if (busy) return;
        busy = true;
        const c = (typeof window !== 'undefined') ? window.__ftdCtx : null;
        const wasRunning = !!(c && c.running);
        if (c) c.running = false;
        try {
            const mc = setupRestMode(w0);
            const b = bridge();
            trace = [];
            const sample = () => {
                const v = (typeof b.inspectVoxel === 'function') ? b.inspectVoxel(mc, mc, mc) : null;
                return v ? (v.fluxX ?? 0) : 0;
            };
            trace.push(sample());
            for (let t = 0; t < RUN_TICKS; t++) {
                if (typeof b.tick === 'function') b.tick();
                trace.push(sample());
                if (t % 20 === 0) {
                    status.textContent = `running clock omega0=${w0.toFixed(2)}… (tick ${t}/${RUN_TICKS})`;
                    drawClock();
                    await new Promise((r) => setTimeout(r, 0));
                }
            }
            drawClock();
            const period = estimatePeriod(trace);
            const expected = 2 * Math.PI / w0;
            status.textContent = `omega0=${w0.toFixed(2)}  T_meas=${period ? period.toFixed(1) : '?'}  (2π/ω₀=${expected.toFixed(1)})`;
        } finally {
            // Leave the clock toggle off so it does not leak to other scenarios.
            try { harness.setToggle('de_broglie_clock', false); } catch (e) { /* noop */ }
            if (c) c.running = wasRunning;
            busy = false;
        }
    }

    // crude zero-crossing period estimate of (trace - mean)
    function estimatePeriod(s) {
        if (s.length < 4) return 0;
        const mean = s.reduce((a, b) => a + b, 0) / s.length;
        let crossings = 0;
        for (let i = 1; i < s.length; i++) {
            const a = s[i - 1] - mean, b = s[i] - mean;
            if ((a < 0 && b >= 0) || (a > 0 && b <= 0)) crossings++;
        }
        return crossings > 0 ? 2 * (s.length - 1) / crossings : 0;
    }

    // ---- live clock oscillation J_x(t) ------------------------------------
    function drawClock() {
        const W = clockCv.width, H = clockCv.height;
        const padL = 6, padR = 6, padT = 6, padB = 6;
        const x0 = padL, x1 = W - padR, y0 = padT, y1 = H - padB;
        clockCtx.clearRect(0, 0, W, H);
        // zero line
        const ymid = (y0 + y1) / 2;
        clockCtx.strokeStyle = 'rgba(136,135,128,0.4)'; clockCtx.lineWidth = 1;
        clockCtx.beginPath(); clockCtx.moveTo(x0, ymid); clockCtx.lineTo(x1, ymid); clockCtx.stroke();
        if (trace.length < 2) return;
        const amp = Math.max(J0, Math.max(...trace.map((v) => Math.abs(v))) || J0);
        const px = (i) => x0 + i / (trace.length - 1) * (x1 - x0);
        const py = (v) => ymid - (v / amp) * ((y1 - y0) / 2 - 2);
        clockCtx.strokeStyle = '#378ADD'; clockCtx.lineWidth = 1.5;
        clockCtx.beginPath();
        trace.forEach((v, i) => { const X = px(i), Y = py(v); i ? clockCtx.lineTo(X, Y) : clockCtx.moveTo(X, Y); });
        clockCtx.stroke();
    }

    // ---- analytic de Broglie lambda(v) ∝ 1/v ------------------------------
    function drawDeBroglie(w0) {
        const W = dbCv.width, H = dbCv.height;
        const padL = 26, padR = 8, padT = 8, padB = 16;
        const x0 = padL, x1 = W - padR, y0 = H - padB, y1 = padT;
        dbCtx.clearRect(0, 0, W, H);
        dbCtx.strokeStyle = 'rgba(136,135,128,0.5)'; dbCtx.lineWidth = 1;
        dbCtx.beginPath(); dbCtx.moveTo(x0, y1); dbCtx.lineTo(x0, y0); dbCtx.lineTo(x1, y0); dbCtx.stroke();
        dbCtx.fillStyle = 'rgba(150,150,150,0.9)'; dbCtx.font = '9px sans-serif';
        dbCtx.fillText('λ', 4, y1 + 8); dbCtx.fillText('v', x1 - 6, y0 + 13);
        // lambda = 2π/k, v = c^2 k / sqrt(c^2 k^2 + w0^2); sweep k.
        const pts = [];
        for (let k = 0.05; k <= 1.2; k += 0.02) {
            const om = Math.sqrt(C2 * k * k + w0 * w0);
            const v = C2 * k / om;
            const lam = 2 * Math.PI / k;
            pts.push([v, lam]);
        }
        const vmax = Math.max(...pts.map((p) => p[0]));
        const lammax = Math.min(140, Math.max(...pts.map((p) => p[1])));
        const px = (v) => x0 + (v / vmax) * (x1 - x0);
        const py = (l) => y0 - (Math.min(l, lammax) / lammax) * (y0 - y1);
        dbCtx.strokeStyle = '#BA7517'; dbCtx.lineWidth = 1.5;
        dbCtx.beginPath();
        pts.forEach((p, i) => { const X = px(p[0]), Y = py(p[1]); i ? dbCtx.lineTo(X, Y) : dbCtx.moveTo(X, Y); });
        dbCtx.stroke();
    }

    drawClock();
    drawDeBroglie(lastOmega0);

    // ---- scenario-switch disposal guard -----------------------------------
    const guard = setInterval(() => {
        const sel = document.getElementById('scenario-select');
        if (sel && sel.value !== SCENARIO_ID) api.dispose();
    }, 500);

    const api = {
        element: panel,
        run,
        getTrace: () => trace.slice(),
        dispose: () => {
            clearInterval(guard);
            try { harness.setToggle?.('de_broglie_clock', false); } catch (e) { /* noop */ }
            if (typeof window !== 'undefined' && window.__ftdDeBroglieClockPanel === api) window.__ftdDeBroglieClockPanel = null;
            panel.remove();
        },
    };
    if (typeof window !== 'undefined') window.__ftdDeBroglieClockPanel = api;
    return api;
}
