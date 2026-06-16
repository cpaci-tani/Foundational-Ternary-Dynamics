/**
 * Classical spin precession for Scale-1 PE — mirrors C++ ParticleEngine::evolve_spin_axes.
 *
 * Magnetic moment μ = (q/m) S (g=2, ℏ=1 in engine units).
 * dS/dt = (q/m) S × B  with B from remote dipoles: B = (3(μ·r̂)r̂ − μ) / (4π r³).
 */

export function magneticMoment(particle) {
    const q = particle.charge;
    const m = particle.mass;
    if (!m || Math.abs(q) < 1e-30) return { mx: 0, my: 0, mz: 0 };
    const sx = particle.spin_ax ?? 0;
    const sy = particle.spin_ay ?? 0;
    const sz = particle.spin_az ?? 0;
    const scale = q / m;
    return { mx: sx * scale, my: sy * scale, mz: sz * scale };
}

/** Dipole field at displacement r from source dipole μ (softened). */
export function dipoleFieldAt(dx, dy, dz, mux, muy, muz, soft) {
    const soft2 = soft * soft;
    const r2 = dx * dx + dy * dy + dz * dz + soft2;
    const r = Math.sqrt(r2);
    if (r < 1e-30) return { bx: 0, by: 0, bz: 0 };
    const r3 = r * r2;
    const invR = 1 / r;
    const rhX = dx * invR;
    const rhY = dy * invR;
    const rhZ = dz * invR;
    const mDotR = mux * rhX + muy * rhY + muz * rhZ;
    const coeff = 1 / (4 * Math.PI * r3);
    return {
        bx: (rhX * (3 * mDotR) - mux) * coeff,
        by: (rhY * (3 * mDotR) - muy) * coeff,
        bz: (rhZ * (3 * mDotR) - muz) * coeff,
    };
}

export function totalBFieldAtParticle(particles, idx, soft) {
    const pi = particles[idx];
    let bx = 0, by = 0, bz = 0;
    for (let j = 0; j < particles.length; j++) {
        if (j === idx) continue;
        const pj = particles[j];
        const sx = pj.spin_ax ?? 0, sy = pj.spin_ay ?? 0, sz = pj.spin_az ?? 0;
        if (sx * sx + sy * sy + sz * sz < 1e-30) continue;
        const mu = magneticMoment(pj);
        const dx = pi.x - pj.x;
        const dy = pi.y - pj.y;
        const dz = pi.z - pj.z;
        const b = dipoleFieldAt(dx, dy, dz, mu.mx, mu.my, mu.mz, soft);
        bx += b.bx;
        by += b.by;
        bz += b.bz;
    }
    return { bx, by, bz };
}

/**
 * Evolve spin axes one timestep (Euler on S, renormalize |S|).
 * Active when magnetic_dipole or lorentz toggles are on.
 */
export function evolveParticleSpins(particles, toggles, soft, dt) {
    if (!toggles.magnetic_dipole && !toggles.lorentz) return;

    for (let i = 0; i < particles.length; i++) {
        const p = particles[i];
        if (p.locked) continue;

        const sax = p.spin_ax ?? 0;
        const say = p.spin_ay ?? 0;
        const saz = p.spin_az ?? 0;
        const sMag2 = sax * sax + say * say + saz * saz;
        if (sMag2 < 1e-30) continue;

        const sMag = Math.sqrt(sMag2);
        const q = p.charge;
        const m = p.mass;
        if (Math.abs(q) < 1e-30 || !m) continue;

        const b = totalBFieldAtParticle(particles, i, soft);
        if (b.bx * b.bx + b.by * b.by + b.bz * b.bz < 1e-60) continue;

        const gamma = q / m;
        const dSx = (say * b.bz - saz * b.by) * gamma * dt;
        const dSy = (saz * b.bx - sax * b.bz) * gamma * dt;
        const dSz = (sax * b.by - say * b.bx) * gamma * dt;

        const nx = sax + dSx;
        const ny = say + dSy;
        const nz = saz + dSz;
        const nMag = Math.sqrt(nx * nx + ny * ny + nz * nz);
        if (nMag < 1e-30) continue;

        const inv = sMag / nMag;
        p.spin_ax = nx * inv;
        p.spin_ay = ny * inv;
        p.spin_az = nz * inv;
    }
}
