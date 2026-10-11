/**
 * The FTD lattice logo: a slowly turning cube of lattice sites, wired along
 * its faces, whose sites step through the ternary states 0, +1, 0, −1.
 *
 * One drawing serves the startup splash and the top bar. The defaults are the
 * splash's; the bar passes a smaller cube with heavier ink (BAR_LOGO), since
 * hairlines and faint sites vanish at its size.
 */

/** Tilt of the cube toward the viewer, in radians. */
const TILT = 0.45;
/** Turn per 60 Hz frame, and how fast a site walks through its states. */
const TURN_PER_FRAME = 0.008;
const STATE_TIME_PER_FRAME = 0.016;

const SPLASH_LOGO = Object.freeze({
    cells: 8,            // sites along an edge
    spacing: 16.8,       // canvas pixels between sites
    offsetY: 0,          // canvas pixels the cube's centre sits below the canvas's
    perspective: 220,    // viewing distance, in site spacings
    lineWidth: 0.5,
    edgeAlpha: [0.12, 0.08],   // base, gain with nearness
    dot: [1.0, 0.8],           // radius: base, gain with nearness
    restAlpha: [0.15, 0.15],   // a site at 0: base, spread over sites
    liveAlpha: [0.5, 0.4],     // a site at +1 or −1
    glow: 5,                   // halo radius of a live site, in dot radii; 0 for none
});

/**
 * Screen extent of the cube's corners over a whole turn, in site spacings from
 * the centre: `{ x, top, bottom }`. The tilt and the perspective make it reach
 * farther down than up.
 */
function cubeReach(cells, perspective) {
    const h = (cells - 1) / 2;
    const cb = Math.cos(TILT);
    const sb = Math.sin(TILT);
    let x = 0;
    let top = 0;
    let bottom = 0;
    for (let k = 0; k < 360; k++) {
        const a = (k * Math.PI) / 180;
        for (const cx of [-h, h]) {
            for (const cy of [-h, h]) {
                for (const cz of [-h, h]) {
                    const x1 = cx * Math.cos(a) - cz * Math.sin(a);
                    const z1 = cx * Math.sin(a) + cz * Math.cos(a);
                    const d = perspective / (perspective + cy * sb + z1 * cb);
                    const y1 = (cy * cb - z1 * sb) * d;
                    x = Math.max(x, Math.abs(x1 * d));
                    top = Math.min(top, y1);
                    bottom = Math.max(bottom, y1);
                }
            }
        }
    }
    return { x, top, bottom };
}

/**
 * The cube at top-bar size, for a square canvas `size` pixels across.
 * `cells` sites along an edge; the splash's eight would be a blur at this size.
 * It is scaled and centred so that the turning cube just fills the canvas.
 */
export function barLogoOptions(size, cells = 4) {
    const perspective = 14;
    const dot = [0.1, 0.07];
    const reach = cubeReach(cells, perspective);
    // A site's own radius, at its largest, is added on every side.
    const rim = dot[0] + dot[1] * 1.3;
    const spacing = (size - 2) / (Math.max(reach.bottom - reach.top, 2 * reach.x) + 2 * rim);
    return {
        cells,
        spacing,
        offsetY: -((reach.bottom + reach.top) / 2) * spacing,
        perspective,
        lineWidth: Math.max(1, size / 44),
        edgeAlpha: [0.3, 0.22],
        dot: [spacing * dot[0], spacing * dot[1]],
        restAlpha: [0.6, 0.25],
        liveAlpha: [0.9, 0.1],
        glow: 0,
    };
}

/**
 * @param {HTMLCanvasElement} canvas
 * @param {object} [options] any of SPLASH_LOGO's fields, plus `random` for the sites' starting phases.
 * @returns {{ step(frames?: number): void, draw(): void }} `step` advances by that many 60 Hz frames and draws.
 */
export function createLatticeLogo(canvas, options = {}) {
    const look = { ...SPLASH_LOGO, ...options };
    const random = typeof options.random === 'function' ? options.random : Math.random;
    const c = canvas.getContext('2d');
    const N = look.cells;
    const S = look.spacing;

    const vertices = [];
    for (let x = 0; x < N; x++) {
        for (let y = 0; y < N; y++) {
            for (let z = 0; z < N; z++) {
                vertices.push([x - (N - 1) / 2, y - (N - 1) / 2, z - (N - 1) / 2]);
            }
        }
    }

    // Edges on the six faces only; the interior is sites without wires.
    const edges = [];
    const idx = (x, y, z) => x * N * N + y * N + z;
    for (let x = 0; x < N; x++) {
        for (let y = 0; y < N; y++) {
            for (let z = 0; z < N; z++) {
                const onFace = x === 0 || x === N - 1 || y === 0 || y === N - 1 || z === 0 || z === N - 1;
                if (!onFace) continue;
                if (x < N - 1 && (y === 0 || y === N - 1 || z === 0 || z === N - 1)) {
                    edges.push([idx(x, y, z), idx(x + 1, y, z)]);
                }
                if (y < N - 1 && (x === 0 || x === N - 1 || z === 0 || z === N - 1)) {
                    edges.push([idx(x, y, z), idx(x, y + 1, z)]);
                }
                if (z < N - 1 && (x === 0 || x === N - 1 || y === 0 || y === N - 1)) {
                    edges.push([idx(x, y, z), idx(x, y, z + 1)]);
                }
            }
        }
    }

    const states = vertices.map(() => ({ s: 0, t: random() * 12 }));
    let angle = 0;

    const project = (x, y, z, a, cx, cy) => {
        const ca = Math.cos(a);
        const sa = Math.sin(a);
        const x1 = x * ca - z * sa;
        const z1 = x * sa + z * ca;
        const cb = Math.cos(TILT);
        const sb = Math.sin(TILT);
        const y1 = y * cb - z1 * sb;
        const z2 = y * sb + z1 * cb;
        const d = look.perspective / (look.perspective + z2);
        return [cx + x1 * S * d, cy + y1 * S * d, d];
    };

    function draw() {
        const width = canvas.width;
        const height = canvas.height;
        c.clearRect(0, 0, width, height);
        const projected = vertices.map((vertex) => project(vertex[0], vertex[1], vertex[2], angle, width / 2, height / 2 + look.offsetY));

        edges
            .map(([a, b]) => ({ a, b, z: (projected[a][2] + projected[b][2]) / 2 }))
            .sort((a, b) => a.z - b.z)
            .forEach(({ a, b, z }) => {
                const alpha = look.edgeAlpha[0] + z * look.edgeAlpha[1];
                c.strokeStyle = `rgba(96,165,250,${alpha})`;
                c.lineWidth = look.lineWidth;
                c.beginPath();
                c.moveTo(projected[a][0], projected[a][1]);
                c.lineTo(projected[b][0], projected[b][1]);
                c.stroke();
            });

        projected
            .map((p, i) => ({ p, i, z: p[2] }))
            .sort((a, b) => a.z - b.z)
            .forEach(({ p, i }) => {
                const s = states[i].s;
                const r = look.dot[0] + p[2] * look.dot[1];
                const h = ((i * 137) % 256) / 256;
                let col;
                if (s > 0) {
                    col = `hsl(${130 + h * 30},${50 + h * 40}%,${50 + h * 20}%)`;
                } else if (s < 0) {
                    col = `hsl(${h * 20},${60 + h * 30}%,${50 + h * 20}%)`;
                } else {
                    col = `hsl(${200 + h * 30},${30 + h * 40}%,${40 + h * 25}%)`;
                }
                const alpha = s === 0
                    ? look.restAlpha[0] + h * look.restAlpha[1]
                    : look.liveAlpha[0] + h * look.liveAlpha[1];

                if (s !== 0 && look.glow > 0) {
                    const reach = r * look.glow;
                    const glow = c.createRadialGradient(p[0], p[1], 0, p[0], p[1], reach);
                    glow.addColorStop(0, (s > 0 ? 'rgba(74,222,128,' : 'rgba(248,113,113,') + '0.15)');
                    glow.addColorStop(1, (s > 0 ? 'rgba(74,222,128,' : 'rgba(248,113,113,') + '0)');
                    c.fillStyle = glow;
                    c.fillRect(p[0] - reach, p[1] - reach, reach * 2, reach * 2);
                }

                c.globalAlpha = alpha;
                c.beginPath();
                c.arc(p[0], p[1], r, 0, Math.PI * 2);
                c.fillStyle = col;
                c.fill();
                c.globalAlpha = 1;
            });
    }

    return {
        draw,
        step(frames = 1) {
            angle += TURN_PER_FRAME * frames;
            for (const state of states) {
                state.t += STATE_TIME_PER_FRAME * frames;
                const phase = state.t % 6;
                state.s = phase < 2 ? 0 : phase < 3 ? 1 : phase < 5 ? 0 : -1;
            }
            draw();
        },
    };
}

/** Frames per second the bar's logo is redrawn at. The splash draws every frame. */
const BAR_LOGO_FPS = 30;

/**
 * Run the logo in the top bar's canvas. The canvas keeps its CSS size; its
 * pixel size follows the device pixel ratio. It turns at the splash's pace
 * and is drawn once, still, when the system asks for reduced motion.
 * Returns a function that stops it.
 */
export function mountBarLatticeLogo(canvas) {
    if (!canvas || typeof canvas.getContext !== 'function') return () => {};
    let logo = null;
    let drawnAt = 0;
    // Rebuilt whenever the canvas's CSS size changes (the interface scale).
    const fit = () => {
        const css = Math.round(canvas.getBoundingClientRect().width) || 42;
        if (css === drawnAt) return;
        drawnAt = css;
        const ratio = Math.min(3, Math.max(1, window.devicePixelRatio || 1));
        const size = Math.round(css * ratio);
        canvas.width = size;
        canvas.height = size;
        logo = createLatticeLogo(canvas, barLogoOptions(size));
        // A turn of the cube already under way, so a still logo is not face-on.
        logo.step(75);
    };
    fit();
    const observer = typeof ResizeObserver === 'function' ? new ResizeObserver(fit) : null;
    observer?.observe(canvas);
    const still = typeof window.matchMedia === 'function'
        && window.matchMedia('(prefers-reduced-motion: reduce)').matches;
    if (still) return () => observer?.disconnect();

    let running = true;
    let last = performance.now();
    const interval = 1000 / BAR_LOGO_FPS;
    const tick = (now) => {
        if (!running || !canvas.isConnected) return;
        window.requestAnimationFrame(tick);
        const elapsed = now - last;
        if (elapsed < interval - 1) return;
        last = now;
        // A long gap (a hidden tab) resumes where it left off instead of jumping.
        logo.step(Math.min(6, elapsed / (1000 / 60)));
    };
    window.requestAnimationFrame(tick);
    return () => {
        running = false;
        observer?.disconnect();
    };
}
