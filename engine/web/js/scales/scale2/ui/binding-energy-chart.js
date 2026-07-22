// Nuclear binding-energy-per-nucleon curve — the classic B/A vs mass-number
// plot, peaking at Fe-56. Static (Z=1..118 is a pure function of the SEMF,
// not a live simulation quantity), so it is drawn once and never redrawn.
//
// Raw-Canvas-2D, following the plotting pattern established in
// scales/scale0/ui/overlays/genesis-burst-panel.js (axis mapping via
// closures, manual moveTo/lineTo/arc calls) — no charting library, since
// this is a single static curve, not a live telemetry stream (the uPlot/
// ChartCard machinery elsewhere in the app is built around ring buffers
// and would be the wrong tool here).

import { allElementEnergies } from '../../../atomic-energy.js';

const FE56_Z = 26;

export function drawBindingEnergyCurve(canvas) {
    if (!canvas) return;
    const ctx2d = canvas.getContext('2d');
    const W = canvas.width, H = canvas.height;
    const padL = 34, padR = 8, padT = 10, padB = 20;
    const x0 = padL, x1 = W - padR, y0 = H - padB, y1 = padT;

    const energies = allElementEnergies();
    const points = [];
    for (const [, e] of energies) {
        if (e.massNumber > 0) points.push({ A: e.massNumber, BA: e.bindingPerNucleon });
    }
    points.sort((a, b) => a.A - b.A);

    const maxA = points.reduce((m, p) => Math.max(m, p.A), 1);
    const maxBA = points.reduce((m, p) => Math.max(m, p.BA), 1);
    const px = (A) => x0 + (A / maxA) * (x1 - x0);
    const py = (BA) => y0 - (BA / maxBA) * (y0 - y1);

    ctx2d.clearRect(0, 0, W, H);

    // axes
    ctx2d.strokeStyle = 'rgba(136,135,128,0.5)';
    ctx2d.lineWidth = 1;
    ctx2d.beginPath();
    ctx2d.moveTo(x0, y1); ctx2d.lineTo(x0, y0); ctx2d.lineTo(x1, y0);
    ctx2d.stroke();
    ctx2d.fillStyle = 'rgba(150,150,150,0.9)';
    ctx2d.font = '9px sans-serif';
    ctx2d.fillText('B/A', 2, y1 + 8);
    ctx2d.fillText('A', x1 - 8, y0 + 14);
    ctx2d.fillText('0', x0 - 4, y0 + 12);
    ctx2d.fillText(String(Math.round(maxA)), x1 - 12, y0 + 12);

    // the curve
    ctx2d.strokeStyle = '#378ADD';
    ctx2d.lineWidth = 1.5;
    ctx2d.beginPath();
    points.forEach((p, i) => {
        const X = px(p.A), Y = py(p.BA);
        i ? ctx2d.lineTo(X, Y) : ctx2d.moveTo(X, Y);
    });
    ctx2d.stroke();

    // Fe-56 peak marker
    const fe56 = points.find((p) => p.A === (energies.get(FE56_Z)?.massNumber ?? 56));
    if (fe56) {
        ctx2d.strokeStyle = 'rgba(95,94,90,0.8)';
        ctx2d.setLineDash([3, 3]);
        ctx2d.beginPath();
        ctx2d.moveTo(px(fe56.A), y1); ctx2d.lineTo(px(fe56.A), y0);
        ctx2d.stroke();
        ctx2d.setLineDash([]);
        ctx2d.fillStyle = '#BA7517';
        ctx2d.beginPath();
        ctx2d.arc(px(fe56.A), py(fe56.BA), 3, 0, 6.2832);
        ctx2d.fill();
        ctx2d.fillStyle = 'rgba(150,150,150,0.9)';
        ctx2d.fillText('Fe-56', px(fe56.A) + 4, py(fe56.BA) - 4);
    }
}
