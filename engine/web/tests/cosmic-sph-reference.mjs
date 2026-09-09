// Transcribed from engine/src/cosmic/cosmic_sph.cpp (kernel 19-47, density 74-104, forces 107-167) plus the
// energy equation this port adds (not present in the C++ reference).
//
// Density-positivity guards mirror cosmic_sph.cpp:115 (rho_i <= 0 skips body i
// entirely, `continue`) and :134 (the pressure term is added only when
// rho_j > 0). The smoothing length used in the force loop is h_new — the value
// the density loop just wrote — matching cosmic_sph.cpp's in-place update of
// smoothing_length before compute_sph_forces runs this same tick; only the
// neighbour-membership test (the `2*max(h,h)` cutoff) still uses the
// entry-tick bi.h/bj.h, matching find_sph_neighbors() running before the
// density pass.
//
// This file is an INDEPENDENT reimplementation for parity testing — it does
// not import engine/web/js/bridge/cosmic-sph.js, so a bug shared between the
// module under test and this reference would not be masked. Do not refactor
// this file to share code with cosmic-sph.js.
export const GAMMA = 5 / 3, ETA = 1.2, ALPHA = 1.0, BETA = 2.0;
export function W(r, h) { const q = r / h, n = 1 / (Math.PI * h * h * h); if (q < 1) return n * (1 - 1.5 * q * q + 0.75 * q * q * q); if (q < 2) { const t = 2 - q; return n * 0.25 * t * t * t; } return 0; }
export function dW(r, h) { const q = r / h, n = 1 / (Math.PI * h * h * h * h); if (q < 1) return n * (-3 * q + 2.25 * q * q); if (q < 2) { const t = 2 - q; return n * (-0.75 * t * t); } return 0; }
/** bodies: [{x,y,z,vx,vy,vz,mass,u,h}] → {rho, P, c, h_new, ax, ay, az, du} per body (gas only). */
export function referenceStep(bodies) {
    const n = bodies.length, rho = new Float64Array(n), P = new Float64Array(n), c = new Float64Array(n), hNew = new Float64Array(n);
    for (let i = 0; i < n; i++) {
        const bi = bodies[i]; let r = bi.mass * W(0, bi.h);
        for (let j = 0; j < n; j++) if (j !== i) { const bj = bodies[j]; const d = Math.hypot(bi.x - bj.x, bi.y - bj.y, bi.z - bj.z); if (d < 2 * Math.max(bi.h, bj.h)) r += bj.mass * W(d, bi.h); }
        rho[i] = r; P[i] = (GAMMA - 1) * r * bi.u; c[i] = r > 0 ? Math.sqrt(GAMMA * P[i] / r) : 0; hNew[i] = r > 0 ? ETA * Math.cbrt(bi.mass / r) : bi.h;
    }
    const ax = new Float64Array(n), ay = new Float64Array(n), az = new Float64Array(n), du = new Float64Array(n);
    for (let i = 0; i < n; i++) {
        if (rho[i] <= 0) continue; // mirrors cosmic_sph.cpp:115 (rho_i <= 0 -> continue)
        for (let j = 0; j < n; j++) {
            if (j === i) continue; const bi = bodies[i], bj = bodies[j];
            const rx = bi.x - bj.x, ry = bi.y - bj.y, rz = bi.z - bj.z, r2 = rx * rx + ry * ry + rz * rz, r = Math.sqrt(r2);
            if (r >= 2 * Math.max(bi.h, bj.h) || r < 1e-10) continue; // entry-tick h (neighbour membership)
            const hAvg = 0.5 * (hNew[i] + hNew[j]); // post-density h (force pass)
            const g = dW(r, hAvg) / r, gx = g * rx, gy = g * ry, gz = g * rz;
            const vx = bi.vx - bj.vx, vy = bi.vy - bj.vy, vz = bi.vz - bj.vz, vdotr = vx * rx + vy * ry + vz * rz;
            let pi = 0;
            // rho[i] > 0 here (outer guard), rho[j] >= 0 always, so
            // 0.5*(rho[i]+rho[j]) > 0 — no separate viscosity guard needed.
            if (vdotr < 0) { const mu = hAvg * vdotr / (r2 + 0.01 * hAvg * hAvg); pi = (-ALPHA * 0.5 * (c[i] + c[j]) * mu + BETA * mu * mu) / (0.5 * (rho[i] + rho[j])); }
            const pressTerm = rho[j] > 0 ? P[i] / (rho[i] * rho[i]) + P[j] / (rho[j] * rho[j]) : 0; // mirrors :134
            const term = pressTerm + pi;
            ax[i] -= bj.mass * term * gx; ay[i] -= bj.mass * term * gy; az[i] -= bj.mass * term * gz;
            du[i] += 0.5 * bj.mass * term * (vx * gx + vy * gy + vz * gz);
        }
    }
    return { rho, P, c, hNew, ax, ay, az, du };
}
