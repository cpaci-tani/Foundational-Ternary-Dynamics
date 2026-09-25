import {
    anchorRuler, formatLength, lengthGauge, rulerInsets, viewportScale, visibleLatticeSpan,
} from './measure.js';
import { createLatticeRuler } from './lattice-ruler.js';
import { createViewportRuler } from './viewport-ruler.js';

export function mountLiveRulers(container) {
    const root = document.createElement('div');
    root.className = 'live-rulers';
    const view = createViewportRuler();
    const lattice = createLatticeRuler();
    root.append(view.el, lattice.el);
    container.appendChild(root);

    return {
        update(reading) {
            const gauge = lengthGauge(reading.engineMode);
            const metres = gauge.metresPerUnit;
            const format = (units, step) => formatLength(units, step, metres);
            const domain = gauge.domainUnits || Math.max(1, reading.latticeSize || 1);
            const span = visibleLatticeSpan(reading);
            const viewRect = container.getBoundingClientRect();
            const obstacles = ['panel-area', 'viewport-overlay']
                .map((id) => document.getElementById(id))
                .filter((el) => el && !el.hidden && el.getClientRects().length > 0)
                .map((el) => el.getBoundingClientRect());
            const insets = rulerInsets(viewRect, obstacles);
            view.el.style.left = `${insets.left}px`;
            view.el.style.right = `${insets.right}px`;
            const openPx = Math.max(1, viewRect.width - insets.left - insets.right);
            const openSpan = span.latticeWidth * (openPx / Math.max(1, viewRect.width));
            view.render(viewportScale(openSpan), format);
            const anchor = anchorRuler({
                domainUnits: domain,
                pixelsPerUnit: span.pixelsPerVoxel,
                viewPx: openPx,
                subject: gauge.subject,
                quasiUnits: reading.shellDiameter,
            });
            const project = anchor.subject === 'quasi-domain' ? reading.projectDiameter : reading.projectUnits;
            if (project) {
                anchor.screen = project(anchor.units);
                if (anchor.screen) {
                    anchor.px = anchor.screen.width;
                    if (anchor.subject === 'quasi-domain') {
                        const q = anchor.screen.width;
                        anchor.opacity = q <= 5 ? 0 : Math.min(1, (q - 5) / 4);
                        anchor.hidden = q <= 5;
                    }
                }
            }
            lattice.render(anchor, format);
        },
        dispose() {
            root.remove();
        },
    };
}
