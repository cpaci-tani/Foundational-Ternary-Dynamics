/**
 * Cosmic scale-5 post-integration updates.
 *
 * Extracted from mock-scale5.js (MS5-3). After the Verlet kick-drift-kick
 * completes, this runs event-driven bookkeeping that can change body
 * counts or identities: event-horizon absorption, tidal disruption,
 * BH-BH mergers, emergent BH formation, star formation + Bondi
 * accretion (subgrid), stellar fuel burn + death sequence, and
 * Hawking evaporation.
 *
 * Call via `postCosmicUpdates.call(bridgeInstance, TYPE)` so `this`
 * binds to CosmicMockBridge. Mutates `this._bodies` directly and
 * spawns new bodies via `this.addBody(...)`.
 */

import {
    G_N, C_SPEED,
    M_CHANDRA_LATTICE, M_TOV_LATTICE,
} from '../constants.js';

// Wave 2G (2026-04-26): M_CHANDRA_LATTICE / M_TOV_LATTICE migrated to
// constants.js (single source of truth). Conversion to solar mass
// (~50× — i.e. 70 lattice units ≈ 1.4 M☉) is undocumented; tracked as
// follow-up to declare the lattice-mass-to-solar-mass scale factor.

export function postCosmicUpdates(TYPE) {
    const T = TYPE;
    const G = G_N;
    const isGas  = (t) => t === T.GAS || t === T.NEBULA;
    const isBH   = (t) => t === T.BLACK_HOLE || t === T.QUASAR;
    const isStar = (t) => t === T.STAR || t === T.NEUTRON_STAR || t === T.WHITE_DWARF;

    // --- Always-active: event-horizon absorption + AGN jet tracking ---
    // NOTE (audit P0-7): `r_sink` below is the INTERNAL accretion-sink
    // radius — a numerically-bounded tuning parameter that decides when a
    // nearby body is swallowed. It is deliberately small and sub-linear in
    // mass so the merger / binary-AGN scenarios (BH separations ~30 lu)
    // stay stable. It is NOT the displayed Schwarzschild horizon, which is
    // r_s = 2 G_N M (linear in M) and is what the cosmic info panel claims
    // and the renderer draws (cosmic-renderer.js schwarzschildRenderRadius).
    // Keeping these separate is intentional: changing the sink to the full
    // linear r_s would vacuum whole disks instantly. Cross-ref P0-7.
    for (const bh of this._bodies) {
        if (!isBH(bh.type)) continue;
        bh.luminosity = (bh.luminosity || 0) * 0.96;
        const r_sink = Math.max(0.8, Math.cbrt(bh.mass) * 0.12);
        const r_kill = r_sink * 0.4;
        const r_h2 = r_kill * r_kill;
        for (const b of this._bodies) {
            if (b.id === bh.id || b.mass <= 0) continue;
            const dx = b.x - bh.x, dy = b.y - bh.y, dz = b.z - bh.z;
            if (dx * dx + dy * dy + dz * dz < r_h2) {
                bh.mass += b.mass;
                bh.luminosity = Math.min((bh.luminosity || 0) + b.mass * 8.0, 50.0);
                b.mass = 0;
            }
        }
    }

    // --- Gradual tidal disruption (spaghettification) ---
    const newGas = [];
    for (const bh of this._bodies) {
        if (!isBH(bh.type)) continue;
        for (const star of this._bodies) {
            if (!isStar(star.type) || star.mass <= 0) continue;
            const dx = star.x - bh.x, dy = star.y - bh.y, dz = star.z - bh.z;
            const r2 = dx * dx + dy * dy + dz * dz;
            const r = Math.sqrt(r2 + 0.01);
            const R_star = star.radius || Math.cbrt(star.mass) * 0.1;
            const r_tidal = R_star * Math.pow(bh.mass / (star.mass + 0.01), 1 / 3);
            if (r < r_tidal * 1.5) {
                const tidalForce = bh.mass / (r2 * r + 0.01);
                star.tidal_stretch = Math.min(1.5, (star.tidal_stretch || 0) + tidalForce * 0.0005);
            } else {
                star.tidal_stretch = Math.max(0, (star.tidal_stretch || 0) - 0.002);
            }
            if ((star.tidal_stretch || 0) > 0.3 && r < r_tidal * 1.2) {
                const shedFraction = Math.min(0.05, star.tidal_stretch * 0.02);
                const shedMass = star.mass * shedFraction;
                if (shedMass > 0.01) {
                    star.mass -= shedMass;
                    const v = Math.sqrt(star.vx * star.vx + star.vy * star.vy + star.vz * star.vz) + 0.01;
                    const jitter = 0.15;
                    newGas.push({
                        mass: shedMass,
                        x: star.x - star.vx / v * 0.5 + (Math.random() - 0.5) * jitter,
                        y: star.y - star.vy / v * 0.5 + (Math.random() - 0.5) * jitter,
                        z: star.z - star.vz / v * 0.5 + (Math.random() - 0.5) * jitter,
                        vx: star.vx * (0.9 + Math.random() * 0.2),
                        vy: star.vy * (0.9 + Math.random() * 0.2),
                        vz: star.vz * (0.9 + Math.random() * 0.2),
                        temp: 5e4 * (1 + star.tidal_stretch)
                    });
                }
            }
            if (star.mass < (star.original_mass || star.mass) * 0.2 && (star.tidal_stretch || 0) > 0.8) {
                if (star.mass > 0.02) {
                    newGas.push({
                        mass: star.mass, x: star.x, y: star.y, z: star.z,
                        vx: star.vx, vy: star.vy, vz: star.vz, temp: 1e5
                    });
                }
                star.mass = 0;
            }
        }
    }
    for (const g of newGas) {
        this.addBody(T.GAS, g.mass, g.x, g.y, g.z, g.vx, g.vy, g.vz, g.temp);
    }

    // --- BH-BH mergers (always active) ---
    for (let i = 0; i < this._bodies.length; i++) {
        const bi = this._bodies[i];
        if (!isBH(bi.type) || bi.mass <= 0) continue;
        for (let j = i + 1; j < this._bodies.length; j++) {
            const bj = this._bodies[j];
            if (!isBH(bj.type) || bj.mass <= 0) continue;
            const dx = bj.x - bi.x, dy = bj.y - bi.y, dz = bj.z - bi.z;
            const r2 = dx * dx + dy * dy + dz * dz;
            const r_merge = Math.cbrt(bi.mass + bj.mass) * 0.3;
            if (r2 > r_merge * r_merge) continue;
            const m_total = bi.mass + bj.mass;
            bi.vx = (bi.vx * bi.mass + bj.vx * bj.mass) / m_total;
            bi.vy = (bi.vy * bi.mass + bj.vy * bj.mass) / m_total;
            bi.vz = (bi.vz * bi.mass + bj.vz * bj.mass) / m_total;
            bi.mass = m_total * 0.95; // 5% GW
            bj.mass = 0;
        }
    }

    // --- Emergent BH formation (FTD prediction) ---
    const C_LAT = 1.0 / Math.sqrt(3.0);
    const hasBH = this._bodies.some(b => isBH(b.type));
    if (!hasBH) {
        let bestBody = null, bestMenc = 0;
        const checkR = this._softening * 2;
        const checkR2 = checkR * checkR;
        for (const b of this._bodies) {
            if (b.mass <= 0) continue;
            let M_enc = 0;
            for (const other of this._bodies) {
                if (other.id === b.id) continue;
                const dr2 = (b.x - other.x) ** 2 + (b.y - other.y) ** 2 + (b.z - other.z) ** 2;
                if (dr2 < checkR2) M_enc += other.mass;
            }
            if (M_enc > bestMenc) { bestMenc = M_enc; bestBody = b; }
        }
        if (bestBody) {
            const v_esc = Math.sqrt(2 * G * bestMenc / checkR);
            if (v_esc > C_LAT && bestMenc > 50) {
                bestBody.type = T.BLACK_HOLE;
                bestBody.temperature = 0;
                bestBody.luminosity = 0;
                bestBody.tidal_stretch = 0;
            }
        }
    }

    // --- Subgrid: star formation + Bondi accretion ---
    if (this._enableSubgrid) {
        const baseSoft2 = this._softening * this._softening;
        const newStars = [];
        for (const b of this._bodies) {
            if (!isGas(b.type) || b.mass < 0.5) continue;
            let nearby = 0;
            for (const other of this._bodies) {
                if (other.id === b.id || !isGas(other.type)) continue;
                const dr2 = (b.x - other.x) ** 2 + (b.y - other.y) ** 2 + (b.z - other.z) ** 2;
                if (dr2 < baseSoft2 * 9) nearby++;
            }
            if (nearby > 10 && b.temperature < 3000 && Math.random() < 0.01) {
                const starMass = b.mass * 0.15;
                b.mass -= starMass;
                newStars.push({
                    type: T.STAR, mass: starMass,
                    x: b.x, y: b.y, z: b.z, vx: b.vx, vy: b.vy, vz: b.vz,
                    temp: 5800, lum: Math.pow(starMass, 3.5)
                });
            }
        }
        for (const s of newStars) {
            this.addBody(s.type, s.mass, s.x, s.y, s.z, s.vx, s.vy, s.vz, s.temp);
            this._bodies[this._bodies.length - 1].luminosity = s.lum;
        }

        for (const bh of this._bodies) {
            if (!isBH(bh.type)) continue;
            const r_acc = Math.max(1.5, Math.cbrt(bh.mass) * 0.3);
            const r_acc2 = r_acc * r_acc;
            for (const gas of this._bodies) {
                if (!isGas(gas.type) || gas.mass <= 0) continue;
                const dx = gas.x - bh.x, dy = gas.y - bh.y, dz = gas.z - bh.z;
                const r2 = dx * dx + dy * dy + dz * dz;
                if (r2 > r_acc2) continue;
                const dvx = gas.vx - bh.vx, dvy = gas.vy - bh.vy, dvz = gas.vz - bh.vz;
                const v_rel2 = dvx * dvx + dvy * dvy + dvz * dvz;
                const r = Math.sqrt(r2 + 0.01);
                if (v_rel2 > 2 * G * bh.mass / r) continue;
                const rate = 0.005 * bh.mass / (v_rel2 + 0.1);
                const dm = Math.min(gas.mass * 0.1, gas.mass * rate * 0.001);
                bh.mass += dm;
                gas.mass -= dm;
            }
        }
    }

    // --- Stellar evolution (fuel burn + death sequence) ---
    if (this._stellarEvolution) {
        const M_chandrasekhar = M_CHANDRA_LATTICE;
        const M_tov = M_TOV_LATTICE;
        const newEjecta = [];
        for (const b of this._bodies) {
            b.age = (b.age || 0) + 1;
            if (!isStar(b.type) || b.mass <= 0) continue;
            if (b.type === T.NEUTRON_STAR || b.type === T.WHITE_DWARF) continue;

            const fuelRate = 0.00002 * Math.pow(b.mass / 50, 2.5);
            b.fuel_fraction = Math.max(0, (b.fuel_fraction || 1.0) - fuelRate);

            const fusionIncome = b.luminosity * 0.001;
            const gravDrain = G_N * b.mass * b.mass * 0.0001;
            const radLoss = b.luminosity * 0.0005;
            b.budget_income = fusionIncome * (b.fuel_fraction > 0 ? 1 : 0);
            b.budget_expense = gravDrain + radLoss;

            if (b.fuel_fraction > 0.3) {
                b.luminosity = Math.pow(b.mass, 3.5);
                b.temperature = 5800 * Math.pow(b.mass / 50, 0.5);
                b.fuel_stage = 0;
            } else if (b.fuel_fraction > 0.15) {
                b.luminosity = Math.pow(b.mass, 3.5) * 3.0;
                b.temperature = 3500;
                b.radius = Math.cbrt(b.mass) * 0.4;
                b.fuel_stage = 1;
            } else if (b.fuel_fraction > 0.05) {
                b.luminosity = Math.pow(b.mass, 3.5) * 1.5;
                b.temperature = 15000;
                b.radius = Math.cbrt(b.mass) * 0.08;
                b.fuel_stage = Math.min(4, Math.floor((0.15 - b.fuel_fraction) / 0.025) + 2);
            } else if (b.fuel_fraction <= 0) {
                b.fuel_stage = 5;
                if (b.mass < M_chandrasekhar) {
                    b.type = T.WHITE_DWARF;
                    b.luminosity = Math.pow(b.mass, 0.5) * 0.01;
                    b.temperature = 12000;
                    b.radius = Math.cbrt(b.mass) * 0.02;
                    b.fuel_fraction = 0;
                } else if (b.mass < M_tov) {
                    const ejectMass = b.mass * 0.7;
                    b.mass -= ejectMass;
                    b.type = T.NEUTRON_STAR;
                    b.luminosity = 0.1;
                    b.temperature = 1e6;
                    b.radius = Math.cbrt(b.mass) * 0.005;
                    b.fuel_fraction = 0;
                    for (let k = 0; k < 12; k++) {
                        const theta = Math.acos(2 * Math.random() - 1);
                        const phi = Math.PI * 2 * Math.random();
                        const v_eject = 2.0 + Math.random() * 1.0;
                        newEjecta.push({
                            mass: ejectMass / 12,
                            x: b.x + Math.sin(theta) * Math.cos(phi) * 1.5,
                            y: b.y + Math.sin(theta) * Math.sin(phi) * 1.5,
                            z: b.z + Math.cos(theta) * 1.5,
                            vx: b.vx + v_eject * Math.sin(theta) * Math.cos(phi),
                            vy: b.vy + v_eject * Math.sin(theta) * Math.sin(phi),
                            vz: b.vz + v_eject * Math.cos(theta),
                            temp: 1e6
                        });
                    }
                } else {
                    const ejectMass = b.mass * 0.5;
                    b.mass -= ejectMass;
                    b.type = T.BLACK_HOLE;
                    b.luminosity = 0;
                    b.temperature = 0;
                    b.fuel_fraction = 0;
                    for (let k = 0; k < 15; k++) {
                        const theta = Math.acos(2 * Math.random() - 1);
                        const phi = Math.PI * 2 * Math.random();
                        const v_eject = 2.5 + Math.random() * 1.5;
                        newEjecta.push({
                            mass: ejectMass / 15,
                            x: b.x + Math.sin(theta) * Math.cos(phi) * 2.0,
                            y: b.y + Math.sin(theta) * Math.sin(phi) * 2.0,
                            z: b.z + Math.cos(theta) * 2.0,
                            vx: b.vx + v_eject * Math.sin(theta) * Math.cos(phi),
                            vy: b.vy + v_eject * Math.sin(theta) * Math.sin(phi),
                            vz: b.vz + v_eject * Math.cos(theta),
                            temp: 2e6
                        });
                    }
                }
            }
        }
        for (const e of newEjecta) {
            this.addBody(T.NEBULA, e.mass, e.x, e.y, e.z, e.vx, e.vy, e.vz, e.temp);
        }
    }

    // --- Hawking evaporation ---
    if (this._hawkingEvaporation) {
        for (const b of this._bodies) {
            if (!isBH(b.type) || b.mass <= 0) continue;
            const T_hawking = 500.0 / (b.mass + 1);
            const hawkingRate = 0.0001 / (b.mass * b.mass + 1);
            const dm = Math.min(b.mass * 0.01, hawkingRate);
            b.mass -= dm;
            b.hawking_temp = T_hawking;
            b.budget_expense = dm;
            if (b.mass < 2.0) {
                const burstEnergy = b.mass;
                b.mass = 0;
                for (let k = 0; k < 6; k++) {
                    const theta = Math.acos(2 * Math.random() - 1);
                    const phi = Math.PI * 2 * Math.random();
                    const v_burst = 3.0;
                    this.addBody(T.NEBULA, burstEnergy / 6,
                        b.x + Math.sin(theta) * Math.cos(phi),
                        b.y + Math.sin(theta) * Math.sin(phi),
                        b.z + Math.cos(theta),
                        v_burst * Math.sin(theta) * Math.cos(phi),
                        v_burst * Math.sin(theta) * Math.sin(phi),
                        v_burst * Math.cos(theta),
                        1e7);
                }
            }
        }
    }

    // --- Custom telemetry + speed limit + cleanup ---
    this._updateTelemetry();
    enforceCosmicSpeedLimit.call(this);
    this._bodies = this._bodies.filter(b => b.mass > 0.01);
}

/**
 * Clamp every body's speed to the lattice speed of light c = 1/sqrt(3).
 * Extracted alongside postUpdates so _enforceSpeedLimit no longer lives
 * on the class.
 */
export function enforceCosmicSpeedLimit() {
    const c2 = C_SPEED * C_SPEED;
    for (const b of this._bodies) {
        const v2 = b.vx * b.vx + b.vy * b.vy + b.vz * b.vz;
        if (v2 > c2) {
            const s = Math.sqrt(c2 / v2);
            b.vx *= s; b.vy *= s; b.vz *= s;
        }
    }
}
