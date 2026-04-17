import { formatEnergy } from '../../../units.js';
import { formatSI } from '../../scale-utils.js';
import { telemetryHub } from '../../../telemetry-hub.js';

export function updateDiagnosticsAndPanels(ctx, state) {
    if (ctx.frameCount % 3 !== 0) return;

    // ── Collect via TelemetryHub (single source of truth for all bridge calls) ──
    const diag = telemetryHub.collectScale0(ctx.bridge, state.fluxMock, state.useFluxMock);
    if (!diag) return;

    // ── Status bar ───────────────────────────────────────────────────────────
    ctx.dom.statusTick.textContent      = formatSI(diag.tick);
    ctx.dom.statusPtime.textContent     = formatSI(Math.round(diag.physicalTime !== undefined ? diag.physicalTime : diag.tick));
    ctx.dom.statusParticles.textContent = diag.manifested || 0;
    ctx.dom.statusEnergy.textContent    = formatEnergy(diag.totalEnergy, 0).text;

    if (ctx.running) {
        ctx.dom.statusDot.classList.remove('idle');
        ctx.dom.statusState.textContent = 'Running';
    } else {
        ctx.dom.statusDot.classList.add('idle');
        ctx.dom.statusState.textContent = 'Idle';
    }

    // ── Panel objects receive latest hub data ────────────────────────────────
    ctx.diagnostics.update(diag);

    // ── Tab-specific rendering ───────────────────────────────────────────────
    switch (ctx.activeTab) {
        case 'diagnostics': {
            ctx.diagnostics.drawSparklines();
            if (ctx.peTelemetry) ctx.peTelemetry.drawCharts();
            const audit = telemetryHub.collectScale0Audit(ctx.bridge, state.fluxMock, state.useFluxMock);
            ctx.diagnostics.updateEnergyAudit(audit);
            break;
        }
        case 'charts': {
            // Collect lagrangian + audit (needed for chart-tab sparklines)
            telemetryHub.collectScale0Lagrangian(ctx.bridge, state.fluxMock, state.useFluxMock);
            const audit = telemetryHub.collectScale0Audit(ctx.bridge, state.fluxMock, state.useFluxMock);

            ctx.fluxEnergyChart.draw();
            ctx.particleChart.draw();

            if (audit) {
                if (ctx.chartEBEnergy) {
                    ctx.chartEBEnergy.push(
                        (audit.EFieldEnergy || audit.eFieldEnergy || 0) -
                        (audit.BFieldEnergy || audit.bFieldEnergy || 0)
                    );
                    ctx.chartEBEnergy.draw('#a78bfa');
                }
                if (ctx.chartGauss) {
                    ctx.chartGauss.push(audit.gaussViolation || 0);
                    ctx.chartGauss.draw('#fbbf24');
                }
            }
            if (ctx.chartCharge)  ctx.chartCharge.draw('#4ade80');
            if (ctx.chartEntropy) ctx.chartEntropy.draw('#60a5fa');
            break;
        }
        case 'lagrangian':
            telemetryHub.collectScale0Lagrangian(ctx.bridge, state.fluxMock, state.useFluxMock);
            ctx.lagrangianChart.draw();
            break;
        case 'inspector':
            ctx.inspector.update();
            break;
        case 'hierarchy':
            ctx.updateHierarchyPanel();
            break;
    }
}
