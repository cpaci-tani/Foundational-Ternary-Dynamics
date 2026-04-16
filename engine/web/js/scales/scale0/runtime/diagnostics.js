import { formatEnergy } from '../../../units.js';
import { formatSI } from '../../scale-utils.js';

export function updateDiagnosticsAndPanels(ctx, state) {
    if (ctx.frameCount % 3 !== 0) return;

    const mainScale0 = ctx.bridge.capabilities.scale0;
    const mockScale0 = state.useFluxMock ? state.fluxMock?.capabilities?.scale0 : null;
    const wasmDiag = mainScale0.getScale0Diagnostics();
    const mockDiag = mockScale0 ? mockScale0.getScale0Diagnostics() : null;
    const diag = (mockDiag && !wasmDiag.manifested && mockDiag.totalFlux > 0)
        ? { ...mockDiag, tick: wasmDiag.tick }
        : wasmDiag;

    ctx.dom.statusTick.textContent = formatSI(diag.tick);
    ctx.dom.statusPtime.textContent = formatSI(Math.round(diag.physicalTime !== undefined ? diag.physicalTime : diag.tick));
    ctx.dom.statusParticles.textContent = diag.manifested || 0;
    ctx.dom.statusEnergy.textContent = formatEnergy(diag.totalEnergy, 0).text;

    if (ctx.running) {
        ctx.dom.statusDot.classList.remove('idle');
        ctx.dom.statusState.textContent = 'Running';
    } else {
        ctx.dom.statusDot.classList.add('idle');
        ctx.dom.statusState.textContent = 'Idle';
    }

    ctx.diagnostics.update(diag);
    ctx.fluxEnergyChart.push(diag);
    ctx.particleChart.push(diag);
    if (ctx.chartCharge) ctx.chartCharge.push(diag.chargeBalance || 0);
    if (ctx.chartEntropy) ctx.chartEntropy.push(diag.entropy || 0);

    const lagrangian = mockScale0 ? mockScale0.getScale0Lagrangian() : mainScale0.getScale0Lagrangian();
    ctx.lagrangianChart.push(lagrangian);

    switch (ctx.activeTab) {
        case 'diagnostics': {
            ctx.diagnostics.drawSparklines();
            if (ctx.peTelemetry) ctx.peTelemetry.drawCharts();
            const audit = mockScale0 ? mockScale0.getScale0EnergyAudit() : mainScale0.getScale0EnergyAudit();
            ctx.diagnostics.updateEnergyAudit(audit);
            break;
        }
        case 'charts': {
            ctx.fluxEnergyChart.draw();
            ctx.particleChart.draw();
            const audit = mockScale0 ? mockScale0.getScale0EnergyAudit() : mainScale0.getScale0EnergyAudit();
            if (audit) {
                if (ctx.chartEBEnergy) {
                    ctx.chartEBEnergy.push((audit.EFieldEnergy || audit.eFieldEnergy || 0) - (audit.BFieldEnergy || audit.bFieldEnergy || 0));
                    ctx.chartEBEnergy.draw('#a78bfa');
                }
                if (ctx.chartGauss) {
                    ctx.chartGauss.push(audit.gaussViolation || 0);
                    ctx.chartGauss.draw('#fbbf24');
                }
            }
            if (ctx.chartCharge) ctx.chartCharge.draw('#4ade80');
            if (ctx.chartEntropy) ctx.chartEntropy.draw('#60a5fa');
            break;
        }
        case 'lagrangian':
            ctx.lagrangianChart.draw();
            break;
        case 'inspector':
            ctx.inspector.update();
            break;
        case 'ontic':
            ctx.updateOnticPanel();
            break;
        case 'hierarchy':
            ctx.updateHierarchyPanel();
            break;
    }
}
