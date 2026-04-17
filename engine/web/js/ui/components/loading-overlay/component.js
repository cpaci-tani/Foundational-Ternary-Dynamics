import { getLoadingOverlayTemplate } from './template.js';

export class LoadingOverlayComponent {
    constructor(root, { version = 'v2.11' } = {}) {
        this.root = root;
        this.version = version;
        this._started = false;
    }

    init() {
        if (!this.root) return this;
        if (!this.root.querySelector('#load-cube')) {
            this.root.innerHTML = getLoadingOverlayTemplate(this.version);
        }
        if (!this._started) {
            this._started = true;
            this._startAnimation();
        }
        return this;
    }

    _startAnimation() {
        const cv = this.root.querySelector('#load-cube');
        if (!cv) return;
        const c = cv.getContext('2d');
        const S = 16.8;
        const CX = 256;
        const CY = 256;
        const N = 8;

        const vertices = [];
        for (let x = 0; x < N; x++) {
            for (let y = 0; y < N; y++) {
                for (let z = 0; z < N; z++) {
                    vertices.push([x - (N - 1) / 2, y - (N - 1) / 2, z - (N - 1) / 2]);
                }
            }
        }

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

        const states = vertices.map(() => ({ s: 0, t: Math.random() * 12 }));
        let angle = 0;

        const project = (x, y, z, a) => {
            const ca = Math.cos(a);
            const sa = Math.sin(a);
            const x1 = x * ca - z * sa;
            const z1 = x * sa + z * ca;
            const cb = Math.cos(0.45);
            const sb = Math.sin(0.45);
            const y1 = y * cb - z1 * sb;
            const z2 = y * sb + z1 * cb;
            const d = 220 / (220 + z2);
            return [CX + x1 * S * d, CY + y1 * S * d, d];
        };

        const draw = () => {
            if (this.root.classList.contains('hidden')) return;
            window.requestAnimationFrame(draw);
            angle += 0.008;
            c.clearRect(0, 0, 512, 512);

            states.forEach((state) => {
                state.t += 0.016;
                const phase = state.t % 6;
                state.s = phase < 2 ? 0 : phase < 3 ? 1 : phase < 5 ? 0 : -1;
            });

            const projected = vertices.map((vertex) => project(vertex[0], vertex[1], vertex[2], angle));

            edges
                .map(([a, b]) => ({ a, b, z: (projected[a][2] + projected[b][2]) / 2 }))
                .sort((a, b) => a.z - b.z)
                .forEach(({ a, b, z }) => {
                    const alpha = 0.12 + z * 0.08;
                    c.strokeStyle = `rgba(96,165,250,${alpha})`;
                    c.lineWidth = 0.5;
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
                    const r = 1.0 + p[2] * 0.8;
                    const h = ((i * 137) % 256) / 256;
                    let col;
                    if (s > 0) {
                        col = `hsl(${130 + h * 30},${50 + h * 40}%,${50 + h * 20}%)`;
                    } else if (s < 0) {
                        col = `hsl(${h * 20},${60 + h * 30}%,${50 + h * 20}%)`;
                    } else {
                        col = `hsl(${200 + h * 30},${30 + h * 40}%,${40 + h * 25}%)`;
                    }
                    const alpha = s === 0 ? 0.15 + h * 0.15 : 0.5 + h * 0.4;

                    if (s !== 0) {
                        const glow = c.createRadialGradient(p[0], p[1], 0, p[0], p[1], r * 5);
                        glow.addColorStop(0, (s > 0 ? 'rgba(74,222,128,' : 'rgba(248,113,113,') + '0.15)');
                        glow.addColorStop(1, (s > 0 ? 'rgba(74,222,128,' : 'rgba(248,113,113,') + '0)');
                        c.fillStyle = glow;
                        c.fillRect(p[0] - r * 5, p[1] - r * 5, r * 10, r * 10);
                    }

                    c.globalAlpha = alpha;
                    c.beginPath();
                    c.arc(p[0], p[1], r, 0, Math.PI * 2);
                    c.fillStyle = col;
                    c.fill();
                    c.globalAlpha = 1;
                });
        };

        window.requestAnimationFrame(draw);
    }
}
