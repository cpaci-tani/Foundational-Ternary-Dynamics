import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

test('diagnostic mocked conservation', async ({ page }) => {
    await gotoAndReady(page);

    const result = await page.evaluate(() => {
        const b = window._ftdBridge;
        b.reset(64);
        
        // Override getEnergyAudit on the JS side
        b.getEnergyAudit = () => {
            const audit = b._module.getEnergyAudit(b._bridge);
            const trueH = audit.fieldEnergy + audit.waveEnergy;
            return {
                ...audit,
                EFieldEnergy: 0.5 * trueH,
                BFieldEnergy: 0.5 * trueH
            };
        };

        // Load s0-field-plane-wave scenario
        b.setupScenario('s0-field-plane-wave');

        // Apply conservative config toggles directly
        b.setToggle('selective_damping', false);
        const togglesToDisable = [
            'damping', 'genesis', 'evaporation', 'coupling', 'movement', 
            'forces', 'gravity', 'poisson_coulomb', 'lorentz_force', 'color_forces', 
            'strong_force', 'triad_binding', 'pair_production', 'exchange_force', 
            'latency_field', 'larmor_radiation', 'weak_transmutation', 
            'dual_substrate', 'exact_dual_gauss', 'emergent_forces', 'langevin',
            'su2_gauge', 'su3_gauge', 'confinement', 'gauss_projection'
        ];
        togglesToDisable.forEach(t => b.setToggle(t, false));
        b.setToggle('wave_propagation', true);
        b.setToggle('symplectic_leapfrog', true);
        b.setDt(0.05);

        const energies = [];

        // Sample every 40 ticks over 200 ticks
        for (let t = 0; t <= 200; t++) {
            if (t > 0) {
                b.tick();
            }
            if (t % 40 === 0) {
                const audit = b.getEnergyAudit();
                energies.push(audit.EFieldEnergy + audit.BFieldEnergy);
            }
        }

        return energies;
    });

    const energies = result;
    const E0 = energies[0];
    const maxE = Math.max(...energies);
    const minE = Math.min(...energies);
    const peakToPeak = (maxE - minE) / E0;
    const drift = Math.abs(energies[energies.length - 1] - E0) / E0;

    console.log(`[mocked conservation] E0: ${E0}`);
    console.log(`[mocked conservation] energies:`, energies.map(e => e.toFixed(6)));
    console.log(`[mocked conservation] drift: ${(drift*100).toFixed(6)}%, peakToPeak: ${(peakToPeak*100).toFixed(6)}%`);

    expect(drift).toBeLessThan(0.005);
    expect(peakToPeak).toBeLessThan(0.005);
});
