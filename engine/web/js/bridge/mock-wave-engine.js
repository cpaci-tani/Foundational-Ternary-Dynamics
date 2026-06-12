/**
 * @file engine/web/js/bridge/mock-wave-engine.js
 * @purpose Encapsulates the Scale 0 wave solver (finite-difference vector wave equation).
 */

import { G_C, C_SPEED } from '../constants.js';
import { allocSharedField, viewSharedField, CTRL } from './shared-field.js';

export class MockWaveEngine {
    constructor(bridge) {
        this.bridge = bridge;
        this.latticeSize = bridge.latticeSize;

        this._fluxJ = null;
        this._fluxWV = null;
        this._fluxMag = null;
        this._stateGrid = null;
        this._sharedField = null;
        this._fluxDirty = false;

        this._lastStateScatterCount = 0;
        this._cflWarned = false;

        // Wave propagation variables
        this._fluxJ_L = null;
        this._fluxJ_R = null;
        this._fluxJ_prev = null;

        // Boundary containment
        this._boundaryShape = 'cube';
        this._boundaryMask = null;
        this._reflectiveBoundary = false;

        // Sparse active-region parameters
        this._sparseTick = true;
        this._activeBox = { x0: this.latticeSize, x1: -1, y0: this.latticeSize, y1: -1, z0: this.latticeSize, z1: -1 };
        this._activeDense = false;
        this._sparseEps = 0;

        this._spongeTable = null;
    }

    _initFluxGrid() {
        const N = this.latticeSize;
        const total = N * N * N;
        if (this.bridge._useSAB) {
            const sab = allocSharedField(N);
            this._sharedField = sab;
            const v = viewSharedField(sab);
            this._fluxJ = v.fluxJ;
            this._fluxWV = v.fluxWV;
            this._fluxMag = v.fluxMag;
            this._stateGrid = v.state;
            v.ctrl[CTRL.N] = N;
        } else {
            this._fluxJ = new Float64Array(total * 3);
            this._fluxWV = new Float64Array(total * 3);
            this._fluxMag = new Float64Array(total);
        }
        this._fluxDirty = true;
    }

    getSharedField() { return this._sharedField; }

    _fluxIdx(x, y, z) {
        const N = this.latticeSize;
        return ((z + N) % N) * N * N + ((y + N) % N) * N + ((x + N) % N);
    }

    _injectFlux(x, y, z, fx, fy, fz) {
        if (!this._fluxJ) this._initFluxGrid();
        const idx = this._fluxIdx(x, y, z);
        this._fluxJ[idx * 3] += fx;
        this._fluxJ[idx * 3 + 1] += fy;
        this._fluxJ[idx * 3 + 2] += fz;
        this._fluxDirty = true;
        this._expandActiveBox(x, y, z);
    }

    _injectWaveVel(x, y, z, wx, wy, wz) {
        if (!this._fluxWV) this._initFluxGrid();
        const idx = this._fluxIdx(x, y, z);
        this._fluxWV[idx * 3] += wx;
        this._fluxWV[idx * 3 + 1] += wy;
        this._fluxWV[idx * 3 + 2] += wz;
        this._expandActiveBox(x, y, z);
    }

    _resetActiveBox() {
        const N = this.latticeSize;
        this._activeBox = { x0: N, x1: -1, y0: N, y1: -1, z0: N, z1: -1 };
        this._activeDense = false;
    }

    _expandActiveBox(x, y, z) {
        const b = this._activeBox, N = this.latticeSize;
        x = ((x % N) + N) % N; y = ((y % N) + N) % N; z = ((z % N) + N) % N;
        if (x < b.x0) b.x0 = x; if (x > b.x1) b.x1 = x;
        if (y < b.y0) b.y0 = y; if (y > b.y1) b.y1 = y;
        if (z < b.z0) b.z0 = z; if (z > b.z1) b.z1 = z;
    }

    _growActiveBox() {
        const b = this._activeBox, N = this.latticeSize;
        if (b.x1 < b.x0) return;
        b.x0 = Math.max(0, b.x0 - 1); b.x1 = Math.min(N - 1, b.x1 + 1);
        b.y0 = Math.max(0, b.y0 - 1); b.y1 = Math.min(N - 1, b.y1 + 1);
        b.z0 = Math.max(0, b.z0 - 1); b.z1 = Math.min(N - 1, b.z1 + 1);
    }

    _recomputeActiveBox() {
        const N = this.latticeSize, J = this._fluxJ, WV = this._fluxWV, eps = this._sparseEps;
        this._resetActiveBox();
        if (!J) return;
        const b = this._activeBox;
        for (let z = 0; z < N; z++) for (let y = 0; y < N; y++) for (let x = 0; x < N; x++) {
            const i3 = (z * N * N + y * N + x) * 3;
            const a = Math.abs(J[i3]) + Math.abs(J[i3 + 1]) + Math.abs(J[i3 + 2])
                    + Math.abs(WV[i3]) + Math.abs(WV[i3 + 1]) + Math.abs(WV[i3 + 2]);
            if (a > eps) {
                if (x < b.x0) b.x0 = x; if (x > b.x1) b.x1 = x;
                if (y < b.y0) b.y0 = y; if (y > b.y1) b.y1 = y;
                if (z < b.z0) b.z0 = z; if (z > b.z1) b.z1 = z;
            }
        }
    }

    _tickClockOnly() {
        if (!this._fluxJ || this.bridge._particles.length === 0) return;
        const N = this.latticeSize, NN = N * N;
        const dt = this.bridge._dt ?? 1.0;
        const w0 = this.bridge._params.omega0 ?? 1.0;
        const w2dt = w0 * w0 * dt;
        const J = this._fluxJ, WV = this._fluxWV;
        for (const p of this.bridge._particles) {
            if (p.state === 0) continue;
            const px = ((Math.round(p.x) % N) + N) % N;
            const py = ((Math.round(p.y) % N) + N) % N;
            const pz = ((Math.round(p.z) % N) + N) % N;
            const i3 = (pz * NN + py * N + px) * 3;
            for (let c = 0; c < 3; c++) {
                WV[i3 + c] -= w2dt * J[i3 + c];
                J[i3 + c] += WV[i3 + c] * dt;
            }
        }
    }

    _tickFlux() {
        if (!this._fluxJ) return;
        const N = this.latticeSize;
        const dt = this.bridge._dt ?? 1.0;
        if (dt * C_SPEED > 1.0 + 1e-9) {
            if (!this._cflWarned) {
                console.warn(`[FTD] CFL violation: dt*c=${(dt*C_SPEED).toFixed(4)} > 1. Reduce dt (max = sqrt(3) ~= 1.732).`);
                this._cflWarned = true;
            }
        }
        const c2 = C_SPEED * C_SPEED * dt;
        const damp = this.bridge._toggles.damping
            ? Math.max(0, Math.min(1, 1.0 - this.bridge._params.damping))
            : 1.0;
        const J = this._fluxJ;
        const WV = this._fluxWV;
        const NN = N * N;
        const NNN = N * N * N;

        let stateGrid = null;
        const doCoupling = this.bridge._toggles.coupling && this.bridge._particles.length > 0;
        if (doCoupling) {
            if (!this._stateGrid || this._stateGrid.length !== NNN) {
                this._stateGrid = new Int8Array(NNN);
            }
            stateGrid = this._stateGrid;
            if (this._lastStateScatterCount > 0) stateGrid.fill(0);
            let scatterCount = 0;
            for (const p of this.bridge._particles) {
                if (p.state === 0) continue;
                const px = ((Math.round(p.x) % N) + N) % N;
                const py = ((Math.round(p.y) % N) + N) % N;
                const pz = ((Math.round(p.z) % N) + N) % N;
                stateGrid[pz * NN + py * N + px] = p.state;
                scatterCount++;
            }
            this._lastStateScatterCount = scatterCount;
        }

        const W_FACE = 1.0 / 3.0;
        const W_EDGE = 1.0 / 6.0;
        const gc_half = G_C * 0.5;
        const Nm1 = N - 1;

        const o_xp = 3;
        const o_xm = -3;
        const o_yp = N * 3;
        const o_ym = -N * 3;
        const o_zp = NN * 3;
        const o_zm = -NN * 3;
        const o_xpyp = o_xp + o_yp;
        const o_xpym = o_xp + o_ym;
        const o_xmyp = o_xm + o_yp;
        const o_xmym = o_xm + o_ym;
        const o_xpzp = o_xp + o_zp;
        const o_xpzm = o_xp + o_zm;
        const o_xmzp = o_xm + o_zp;
        const o_xmzm = o_xm + o_zm;
        const o_ypzp = o_yp + o_zp;
        const o_ypzm = o_yp + o_zm;
        const o_ymzp = o_ym + o_zp;
        const o_ymzm = o_ym + o_zm;

        let sx0 = 1, sx1 = N - 2, sy0 = 1, sy1 = N - 2, sz0 = 1, sz1 = N - 2;
        let sparseActive = false;
        let runBoundaryWV = true;
        if (this._sparseTick && !this._activeDense) {
            const bx = this._activeBox;
            if (bx.x1 < bx.x0) return;
            const Dsp = this._reflectiveBoundary ? 1 : Math.min(6, Math.max(2, Math.floor(N / 4)));
            const margin = Dsp + 1;
            const nearWall = bx.x0 <= margin || bx.x1 >= N - 1 - margin
                          || bx.y0 <= margin || bx.y1 >= N - 1 - margin
                          || bx.z0 <= margin || bx.z1 >= N - 1 - margin;
            const vol = (bx.x1 - bx.x0 + 1) * (bx.y1 - bx.y0 + 1) * (bx.z1 - bx.z0 + 1);
            if (nearWall || vol > 0.4 * N * N * N) {
                this._activeDense = true;
            } else {
                sparseActive = true;
                runBoundaryWV = false;
                sx0 = Math.max(1, bx.x0 - 1); sx1 = Math.min(N - 2, bx.x1 + 1);
                sy0 = Math.max(1, bx.y0 - 1); sy1 = Math.min(N - 2, bx.y1 + 1);
                sz0 = Math.max(1, bx.z0 - 1); sz1 = Math.min(N - 2, bx.z1 + 1);
            }
        }

        for (let z = sz0; z <= sz1; z++) {
            const zBase = z * NN;
            for (let y = sy0; y <= sy1; y++) {
                const rowStart = zBase + y * N + sx0;
                let i3 = rowStart * 3;
                let vi = rowStart;

                for (let x = sx0; x <= sx1; x++) {
                    const center0 = J[i3];
                    const face0 = J[i3 + o_xp] + J[i3 + o_xm]
                                + J[i3 + o_yp] + J[i3 + o_ym]
                                + J[i3 + o_zp] + J[i3 + o_zm];
                    const edge0 = J[i3 + o_xpyp] + J[i3 + o_xpym]
                                + J[i3 + o_xmyp] + J[i3 + o_xmym]
                                + J[i3 + o_xpzp] + J[i3 + o_xpzm]
                                + J[i3 + o_xmzp] + J[i3 + o_xmzm]
                                + J[i3 + o_ypzp] + J[i3 + o_ypzm]
                                + J[i3 + o_ymzp] + J[i3 + o_ymzm];
                    WV[i3] += c2 * (W_FACE * face0 + W_EDGE * edge0 - 4.0 * center0);

                    const i3p1 = i3 + 1;
                    const center1 = J[i3p1];
                    const face1 = J[i3p1 + o_xp] + J[i3p1 + o_xm]
                                + J[i3p1 + o_yp] + J[i3p1 + o_ym]
                                + J[i3p1 + o_zp] + J[i3p1 + o_zm];
                    const edge1 = J[i3p1 + o_xpyp] + J[i3p1 + o_xpym]
                                + J[i3p1 + o_xmyp] + J[i3p1 + o_xmym]
                                + J[i3p1 + o_xpzp] + J[i3p1 + o_xpzm]
                                + J[i3p1 + o_xmzp] + J[i3p1 + o_xmzm]
                                + J[i3p1 + o_ypzp] + J[i3p1 + o_ypzm]
                                + J[i3p1 + o_ymzp] + J[i3p1 + o_ymzm];
                    WV[i3p1] += c2 * (W_FACE * face1 + W_EDGE * edge1 - 4.0 * center1);

                    const i3p2 = i3 + 2;
                    const center2 = J[i3p2];
                    const face2 = J[i3p2 + o_xp] + J[i3p2 + o_xm]
                                + J[i3p2 + o_yp] + J[i3p2 + o_ym]
                                + J[i3p2 + o_zp] + J[i3p2 + o_zm];
                    const edge2 = J[i3p2 + o_xpyp] + J[i3p2 + o_xpym]
                                + J[i3p2 + o_xmyp] + J[i3p2 + o_xmym]
                                + J[i3p2 + o_xpzp] + J[i3p2 + o_xpzm]
                                + J[i3p2 + o_xmzp] + J[i3p2 + o_xmzm]
                                + J[i3p2 + o_ypzp] + J[i3p2 + o_ypzm]
                                + J[i3p2 + o_ymzp] + J[i3p2 + o_ymzm];
                    WV[i3p2] += c2 * (W_FACE * face2 + W_EDGE * edge2 - 4.0 * center2);

                    if (doCoupling && stateGrid) {
                        WV[i3]     += gc_half * (stateGrid[vi + 1] - stateGrid[vi - 1]);
                        WV[i3p1]   += gc_half * (stateGrid[vi + N] - stateGrid[vi - N]);
                        WV[i3p2]   += gc_half * (stateGrid[vi + NN] - stateGrid[vi - NN]);
                    }

                    i3 += 3;
                    vi++;
                }
            }
        }

        for (let z = 0; runBoundaryWV && z < N; z++) {
            const zw = z * NN;
            const zpw = ((z + 1) % N) * NN;
            const zmw = ((z - 1 + N) % N) * NN;
            const zBoundary = (z === 0 || z === Nm1);
            for (let y = 0; y < N; y++) {
                const yw = y * N;
                const ypw = ((y + 1) % N) * N;
                const ymw = ((y - 1 + N) % N) * N;
                const yBoundary = (y === 0 || y === Nm1);

                if (!zBoundary && !yBoundary) continue;

                for (let x = 0; x < N; x++) {
                    const xpx = (x + 1) % N;
                    const xmx = (x - 1 + N) % N;
                    const idx = zw + yw + x;

                    const xp = zw + yw + xpx;
                    const xm = zw + yw + xmx;
                    const yp = zw + ypw + x;
                    const ym = zw + ymw + x;
                    const zp = zpw + yw + x;
                    const zm = zmw + yw + x;

                    const xpyp = zw + ypw + xpx;
                    const xpym = zw + ymw + xpx;
                    const xmyp = zw + ypw + xmx;
                    const xmym = zw + ymw + xmx;
                    const xpzp = zpw + yw + xpx;
                    const xpzm = zmw + yw + xpx;
                    const xmzp = zpw + yw + xmx;
                    const xmzm = zmw + yw + xmx;
                    const ypzp = zpw + ypw + x;
                    const ypzm = zmw + ypw + x;
                    const ymzp = zpw + ymw + x;
                    const ymzm = zmw + ymw + x;

                    const i3 = idx * 3;

                    const center0 = J[i3];
                    const face0 = J[xp * 3] + J[xm * 3]
                                + J[yp * 3] + J[ym * 3]
                                + J[zp * 3] + J[zm * 3];
                    const edge0 = J[xpyp * 3] + J[xpym * 3]
                                + J[xmyp * 3] + J[xmym * 3]
                                + J[xpzp * 3] + J[xpzm * 3]
                                + J[xmzp * 3] + J[xmzm * 3]
                                + J[ypzp * 3] + J[ypzm * 3]
                                + J[ymzp * 3] + J[ymzm * 3];
                    WV[i3] += c2 * (W_FACE * face0 + W_EDGE * edge0 - 4.0 * center0);

                    const i3p1 = i3 + 1;
                    const center1 = J[i3p1];
                    const face1 = J[xp * 3 + 1] + J[xm * 3 + 1]
                                + J[yp * 3 + 1] + J[ym * 3 + 1]
                                + J[zp * 3 + 1] + J[zm * 3 + 1];
                    const edge1 = J[xpyp * 3 + 1] + J[xpym * 3 + 1]
                                + J[xmyp * 3 + 1] + J[xmym * 3 + 1]
                                + J[xpzp * 3 + 1] + J[xpzm * 3 + 1]
                                + J[xmzp * 3 + 1] + J[xmzm * 3 + 1]
                                + J[ypzp * 3 + 1] + J[ypzm * 3 + 1]
                                + J[ymzp * 3 + 1] + J[ymzm * 3 + 1];
                    WV[i3p1] += c2 * (W_FACE * face1 + W_EDGE * edge1 - 4.0 * center1);

                    const i3p2 = i3 + 2;
                    const center2 = J[i3p2];
                    const face2 = J[xp * 3 + 2] + J[xm * 3 + 2]
                                + J[yp * 3 + 2] + J[ym * 3 + 2]
                                + J[zp * 3 + 2] + J[zm * 3 + 2];
                    const edge2 = J[xpyp * 3 + 2] + J[xpym * 3 + 2]
                                + J[xmyp * 3 + 2] + J[xmym * 3 + 2]
                                + J[xpzp * 3 + 2] + J[xpzm * 3 + 2]
                                + J[xmzp * 3 + 2] + J[xmzm * 3 + 2]
                                + J[ypzp * 3 + 2] + J[ypzm * 3 + 2]
                                + J[ymzp * 3 + 2] + J[ymzm * 3 + 2];
                    WV[i3p2] += c2 * (W_FACE * face2 + W_EDGE * edge2 - 4.0 * center2);

                    if (doCoupling && stateGrid) {
                        WV[i3]     += gc_half * (stateGrid[xp] - stateGrid[xm]);
                        WV[i3p1]   += gc_half * (stateGrid[yp] - stateGrid[ym]);
                        WV[i3p2]   += gc_half * (stateGrid[zp] - stateGrid[zm]);
                    }
                }
            }
        }

        for (let z = 1; runBoundaryWV && z < Nm1; z++) {
            const zw = z * NN;
            const zpw = zw + NN;
            const zmw = zw - NN;
            for (let y = 1; y < Nm1; y++) {
                const yw = y * N;
                const ypw = yw + N;
                const ymw = yw - N;

                for (const x of [0, Nm1]) {
                    const xpx = (x + 1) % N;
                    const xmx = (x - 1 + N) % N;
                    const idx = zw + yw + x;

                    const xp = zw + yw + xpx;
                    const xm = zw + yw + xmx;
                    const yp = zw + ypw + x;
                    const ym = zw + ymw + x;
                    const zp = zpw + yw + x;
                    const zm = zmw + yw + x;

                    const xpyp = zw + ypw + xpx;
                    const xpym = zw + ymw + xpx;
                    const xmyp = zw + ypw + xmx;
                    const xmym = zw + ymw + xmx;
                    const xpzp = zpw + yw + xpx;
                    const xpzm = zmw + yw + xpx;
                    const xmzp = zpw + yw + xmx;
                    const xmzm = zmw + yw + xmx;
                    const ypzp = zpw + ypw + x;
                    const ypzm = zmw + ypw + x;
                    const ymzp = zpw + ymw + x;
                    const ymzm = zmw + ymw + x;

                    const i3 = idx * 3;
                    const center0 = J[i3];
                    const face0 = J[xp * 3] + J[xm * 3] + J[yp * 3] + J[ym * 3] + J[zp * 3] + J[zm * 3];
                    const edge0 = J[xpyp*3]+J[xpym*3]+J[xmyp*3]+J[xmym*3]+J[xpzp*3]+J[xpzm*3]+J[xmzp*3]+J[xmzm*3]+J[ypzp*3]+J[ypzm*3]+J[ymzp*3]+J[ymzm*3];
                    WV[i3] += c2 * (W_FACE * face0 + W_EDGE * edge0 - 4.0 * center0);

                    const i3p1 = i3 + 1;
                    const center1 = J[i3p1];
                    const face1 = J[xp*3+1]+J[xm*3+1]+J[yp*3+1]+J[ym*3+1]+J[zp*3+1]+J[zm*3+1];
                    const edge1 = J[xpyp*3+1]+J[xpym*3+1]+J[xmyp*3+1]+J[xmym*3+1]+J[xpzp*3+1]+J[xpzm*3+1]+J[xmzp*3+1]+J[xmzm*3+1]+J[ypzp*3+1]+J[ypzm*3+1]+J[ymzp*3+1]+J[ymzm*3+1];
                    WV[i3p1] += c2 * (W_FACE * face1 + W_EDGE * edge1 - 4.0 * center1);

                    const i3p2 = i3 + 2;
                    const center2 = J[i3p2];
                    const face2 = J[xp*3+2]+J[xm*3+2]+J[yp*3+2]+J[ym*3+2]+J[zp*3+2]+J[zm*3+2];
                    const edge2 = J[xpyp*3+2]+J[xpym*3+2]+J[xmyp*3+2]+J[xmym*3+2]+J[xpzp*3+2]+J[xpzm*3+2]+J[xmzp*3+2]+J[xmzm*3+2]+J[ypzp*3+2]+J[ypzm*3+2]+J[ymzp*3+2]+J[ymzm*3+2];
                    WV[i3p2] += c2 * (W_FACE * face2 + W_EDGE * edge2 - 4.0 * center2);

                    if (doCoupling && stateGrid) {
                        WV[i3]   += gc_half * (stateGrid[xp] - stateGrid[xm]);
                        WV[i3p1] += gc_half * (stateGrid[yp] - stateGrid[ym]);
                        WV[i3p2] += gc_half * (stateGrid[zp] - stateGrid[zm]);
                    }
                }
            }
        }

        if (this.bridge._toggles.de_broglie_clock && this.bridge._particles.length > 0) {
            const w0 = this.bridge._params.omega0 ?? 1.0;
            const w2dt = w0 * w0 * dt;
            for (const p of this.bridge._particles) {
                if (p.state === 0) continue;
                const px = ((Math.round(p.x) % N) + N) % N;
                const py = ((Math.round(p.y) % N) + N) % N;
                const pz = ((Math.round(p.z) % N) + N) % N;
                const i3 = (pz * NN + py * N + px) * 3;
                WV[i3]     -= w2dt * J[i3];
                WV[i3 + 1] -= w2dt * J[i3 + 1];
                WV[i3 + 2] -= w2dt * J[i3 + 2];
            }
        }

        const total = N * N * N;
        const selective = this.bridge._toggles.selective_damping;
        const total3 = total * 3;

        if (sparseActive && !(selective && damp < 1.0 && this.bridge._particles.length > 0)) {
            const effDamp = (selective && damp < 1.0 && this.bridge._particles.length === 0) ? 1.0 : damp;
            for (let z = sz0; z <= sz1; z++) {
                for (let y = sy0; y <= sy1; y++) {
                    let i3 = (z * NN + y * N + sx0) * 3;
                    for (let x = sx0; x <= sx1; x++) {
                        J[i3]     = (J[i3]     + WV[i3]     * dt) * effDamp;
                        J[i3 + 1] = (J[i3 + 1] + WV[i3 + 1] * dt) * effDamp;
                        J[i3 + 2] = (J[i3 + 2] + WV[i3 + 2] * dt) * effDamp;
                        WV[i3] *= effDamp; WV[i3 + 1] *= effDamp; WV[i3 + 2] *= effDamp;
                        i3 += 3;
                    }
                }
            }
        } else if (selective && damp < 1.0) {
            if (!this._selectiveDampMask || this._selectiveDampMask.length !== total) {
                this._selectiveDampMask = new Uint8Array(total);
            }
            this._selectiveDampMask.fill(0);
            for (const p of this.bridge._particles) {
                const px = ((p.x % N) + N) % N;
                const py = ((p.y % N) + N) % N;
                const pz = ((p.z % N) + N) % N;
                const pidx = pz * N * N + py * N + px;
                this._selectiveDampMask[pidx] = 1;
                const offsets = [
                    [(px + 1) % N, py, pz], [(px - 1 + N) % N, py, pz],
                    [px, (py + 1) % N, pz], [px, (py - 1 + N) % N, pz],
                    [px, py, (pz + 1) % N], [px, py, (pz - 1 + N) % N],
                ];
                for (const [nx, ny, nz] of offsets) {
                    this._selectiveDampMask[nz * N * N + ny * N + nx] = 1;
                }
            }
            for (let i = 0; i < total; i++) {
                const d = this._selectiveDampMask[i] ? damp : 1.0;
                const i3 = i * 3;
                J[i3]     = (J[i3]     + WV[i3]     * dt) * d;
                J[i3 + 1] = (J[i3 + 1] + WV[i3 + 1] * dt) * d;
                J[i3 + 2] = (J[i3 + 2] + WV[i3 + 2] * d) * d;
                WV[i3]     *= d;
                WV[i3 + 1] *= d;
                WV[i3 + 2] *= d;
            }
        } else {
            for (let k = 0; k < total3; k += 3) {
                J[k]     = (J[k]     + WV[k]     * dt) * damp;
                J[k + 1] = (J[k + 1] + WV[k + 1] * dt) * damp;
                J[k + 2] = (J[k + 2] + WV[k + 2] * dt) * damp;
                WV[k]     *= damp;
                WV[k + 1] *= damp;
                WV[k + 2] *= damp;
            }
        }

        if (this._boundaryMask && this._reflectiveBoundary) {
            for (let idx = 0; idx < total; idx++) {
                if (!this._boundaryMask[idx]) {
                    J[idx * 3] = 0; J[idx * 3 + 1] = 0; J[idx * 3 + 2] = 0;
                    WV[idx * 3] = 0; WV[idx * 3 + 1] = 0; WV[idx * 3 + 2] = 0;
                }
            }
        }

        if (!this._reflectiveBoundary && runBoundaryWV) {
            const Nm1 = N - 1;
            const D = Math.min(6, Math.max(2, Math.floor(N / 4)));
            if (!this._spongeTable || this._spongeTable.length !== D + 1) {
                const tbl = new Float32Array(D + 1);
                for (let d = 0; d <= D; d++) {
                    const r = d / D;
                    tbl[d] = r * r;
                }
                this._spongeTable = tbl;
            }
            const f = this._spongeTable;
            for (let z = 0; z < N; z++) {
                const dz = Math.min(z, Nm1 - z);
                for (let y = 0; y < N; y++) {
                    const dy = Math.min(y, Nm1 - y);
                    for (let x = 0; x < N; x++) {
                        const dx = Math.min(x, Nm1 - x);
                        const d = Math.min(dx, dy, dz);
                        if (d >= D) continue;
                        const fd = f[d];
                        const i3 = (z * N * N + y * N + x) * 3;
                        J[i3]   *= fd; J[i3+1] *= fd; J[i3+2] *= fd;
                        WV[i3]  *= fd; WV[i3+1]*= fd; WV[i3+2]*= fd;
                    }
                }
            }
        }

        this._fluxDirty = true;
        if (this._sparseTick && !this._activeDense) {
            this._growActiveBox();
            if ((this.bridge._tick & 31) === 0) this._recomputeActiveBox();
        }
    }

    _divergenceAt(x, y, z) {
        const N = this.latticeSize;
        const J = this._fluxJ;
        const idx = (c, xx, yy, zz) => {
            const i = ((zz % N) + N) % N * N * N + ((yy % N) + N) % N * N + ((xx % N) + N) % N;
            return J[i * 3 + c];
        };
        return (idx(0, x + 1, y, z) - idx(0, x - 1, y, z)
              + idx(1, x, y + 1, z) - idx(1, x, y - 1, z)
              + idx(2, x, y, z + 1) - idx(2, x, y, z - 1)) * 0.5;
    }

    _updateFluxMag() {
        if (!this._fluxDirty || !this._fluxJ) return;
        const total = this.latticeSize ** 3;
        const J = this._fluxJ;
        const M = this._fluxMag;
        for (let i = 0, k = 0; i < total; i++, k += 3) {
            const jx = J[k], jy = J[k + 1], jz = J[k + 2];
            M[i] = Math.sqrt(jx * jx + jy * jy + jz * jz);
        }
        this._fluxDirty = false;
    }

    getFluxSlice(axis, index) {
        if (!this._fluxJ) this._initFluxGrid();
        this._updateFluxMag();
        const N = this.latticeSize;
        const data = new Float64Array(N * N);
        for (let a = 0; a < N; a++) {
            for (let b = 0; b < N; b++) {
                let idx;
                if (axis === 0) idx = this._fluxIdx(index, a, b);
                else if (axis === 1) idx = this._fluxIdx(a, index, b);
                else idx = this._fluxIdx(a, b, index);
                data[a * N + b] = this._fluxMag[idx];
            }
        }
        return data;
    }

    getFluxVolume() {
        if (!this._fluxJ) this._initFluxGrid();
        this._updateFluxMag();
        return this._fluxMag;
    }
}
