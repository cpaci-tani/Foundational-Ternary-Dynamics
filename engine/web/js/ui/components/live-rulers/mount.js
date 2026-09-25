import {
    anchorRuler, formatLength, lengthGauge, LIVE_MEASURE_DEFAULTS, rulerInsets, viewportScale, visibleLatticeSpan,
} from './measure.js';
import { createLatticeRuler } from './lattice-ruler.js';
import { createVoxelClocks } from './voxel-clocks.js';
import { createMooreNeighborhood } from './neighborhood.js';
import { createViewportRuler } from './viewport-ruler.js';

export function mountLiveRulers(container) {
    const root = document.createElement('div');
    root.className = 'live-rulers';
    const view = createViewportRuler();
    const lattice = createLatticeRuler();
    const clocks = createVoxelClocks();
    const neighborhood = createMooreNeighborhood();
    root.append(view.el, lattice.el, clocks.el, neighborhood.el);
    container.appendChild(root);

    return {
        update(reading) {
            const measures = reading.measures || LIVE_MEASURE_DEFAULTS;
            root.style.setProperty('--ruler-scale', String(measures.rulerScale ?? 1));
            root.style.setProperty('--clock-size', `${measures.clockSize ?? 16}px`);
            root.style.setProperty('--string-width', `${measures.stringWidth ?? 148}px`);
            root.style.setProperty('--value-size', `${measures.valueSize ?? 16}px`);
            root.style.setProperty('--line-thickness', `${measures.lineThickness ?? 1.5}px`);
            root.style.setProperty('--bar-thickness', `${measures.barThickness ?? 8}px`);
            const value = lattice.el.querySelector('.live-ruler-string-value');
            if (value) value.style.fontSize = `${measures.valueSize ?? 16}px`;
            const wave = lattice.el.querySelector('.live-ruler-string path');
            if (wave) wave.style.strokeWidth = `${measures.lineThickness ?? 1.5}px`;
            view.el.hidden = measures.viewRuler === false;
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
            const fluxOn = reading.fluxVisible !== false;
            if (anchor.subject === 'voxel' && !fluxOn) {
                anchor.hidden = true;
                anchor.wave = null;
            } else if (anchor.subject === 'voxel' && reading.pointSprite) {
                anchor.px = reading.pointSprite.px;
                anchor.fill = measures.spectrum === false ? null : reading.pointSprite.fill;
                anchor.wave = measures.energyString === false ? null : reading.pointSprite.wave;
                if (reading.pointSprite.screen) anchor.screen = reading.pointSprite.screen;
                anchor.name = reading.pointSprite.name || '';
            }
            if (measures.latticeRuler === false) anchor.hidden = true;
            const project = anchor.subject === 'quasi-domain' ? reading.projectDiameter
                : anchor.subject === 'voxel' ? null
                : reading.projectUnits;
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
            const close = anchor.subject === 'voxel' && fluxOn;
            clocks.render(close && measures.voxelClocks !== false ? reading.voxelClocks : []);
            const neighborhoodOn = measures.mooreBars !== false || measures.mooreWaves !== false || measures.mooreJoules !== false;
            neighborhood.render(close && neighborhoodOn ? reading.mooreNeighborhood : []);
        },
        dispose() {
            root.remove();
        },
    };
}
