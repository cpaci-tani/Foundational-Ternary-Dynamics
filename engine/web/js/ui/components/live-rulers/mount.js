import {
    anchorRuler, formatLength, lengthGauge, LIVE_MEASURE_DEFAULTS, OUTER_WORLD_PX, rulerInsets, viewportScale, visibleLatticeSpan,
} from './measure.js';
import { createLatticeRuler } from './lattice-ruler.js';
import { createVoxelClocks } from './voxel-clocks.js';
import { createMooreNeighborhood } from './neighborhood.js';
import { createLandmarkRings } from './landmark-rings.js';
import { createViewportRuler } from './viewport-ruler.js';

const OBSTACLE_IDS = ['panel-area', 'viewport-overlay'];
const LAYOUT_MAX_AGE_MS = 250;

export function mountLiveRulers(container) {
    const root = document.createElement('div');
    root.className = 'live-rulers';
    const view = createViewportRuler();
    const lattice = createLatticeRuler();
    const clocks = createVoxelClocks();
    const neighborhood = createMooreNeighborhood();
    const landmarks = createLandmarkRings();
    root.append(view.el, lattice.el, clocks.el, neighborhood.el, landmarks.el);
    container.appendChild(root);

    // A setting reaches the DOM only when its value changes. The last value
    // written is kept here because inline-style getters do not return what
    // was set (`0` reads back as `0px`, `hsl()` as `rgb()`).
    const applied = new Map();
    const once = (key, value) => {
        if (applied.get(key) === value) return false;
        applied.set(key, value);
        return true;
    };
    const setVar = (name, value) => { if (once(name, value)) root.style.setProperty(name, value); };
    const latticeValue = lattice.el.querySelector('.live-ruler-string-value');
    const latticeWave = lattice.el.querySelector('.live-ruler-string path');

    // The view rectangle and the panels covering it come from layout. They
    // are re-read when something resizes or a panel shows or hides, and
    // otherwise at most every LAYOUT_MAX_AGE_MS; a read on every frame
    // forces a reflow on every frame.
    let layout = null;
    let layoutAt = 0;
    const invalidateLayout = () => { layout = null; };
    const observer = typeof ResizeObserver === 'function' ? new ResizeObserver(invalidateLayout) : null;
    const watched = new Set();
    observer?.observe(container);
    window.addEventListener('resize', invalidateLayout);
    window.addEventListener('ftd:panel-visibility-change', invalidateLayout);
    const readLayout = () => {
        const now = performance.now();
        if (layout && now - layoutAt < LAYOUT_MAX_AGE_MS) return layout;
        const viewRect = container.getBoundingClientRect();
        const obstacles = [];
        for (const id of OBSTACLE_IDS) {
            const el = document.getElementById(id);
            if (!el) continue;
            if (observer && !watched.has(el)) {
                watched.add(el);
                observer.observe(el);
            }
            if (el.hidden || el.getClientRects().length === 0) continue;
            obstacles.push(el.getBoundingClientRect());
        }
        layout = { viewRect, obstacles };
        layoutAt = now;
        return layout;
    };

    return {
        update(reading) {
            const measures = reading.measures || LIVE_MEASURE_DEFAULTS;
            setVar('--ruler-scale', String(measures.rulerScale ?? 1));
            setVar('--clock-size', `${measures.clockSize ?? 16}px`);
            setVar('--string-width', `${measures.stringWidth ?? 148}px`);
            setVar('--value-size', `${measures.valueSize ?? 16}px`);
            setVar('--line-thickness', `${measures.lineThickness ?? 1.5}px`);
            setVar('--bar-thickness', `${measures.barThickness ?? 8}px`);
            const valueSize = `${measures.valueSize ?? 16}px`;
            if (latticeValue && once('value-font', valueSize)) latticeValue.style.fontSize = valueSize;
            const lineWidth = `${measures.lineThickness ?? 1.5}px`;
            if (latticeWave && once('wave-stroke', lineWidth)) latticeWave.style.strokeWidth = lineWidth;
            const viewHidden = measures.viewRuler === false;
            if (once('view-hidden', viewHidden)) view.el.hidden = viewHidden;
            const gauge = lengthGauge(reading.engineMode);
            const metres = gauge.metresPerUnit;
            const format = (units, step) => formatLength(units, step, metres);
            const domain = gauge.domainUnits || Math.max(1, reading.latticeSize || 1);
            const span = visibleLatticeSpan(reading);
            const { viewRect, obstacles } = readLayout();
            const insets = rulerInsets(viewRect, obstacles);
            const insetLeft = `${insets.left}px`;
            if (once('inset-left', insetLeft)) view.el.style.left = insetLeft;
            const insetRight = `${insets.right}px`;
            if (once('inset-right', insetRight)) view.el.style.right = insetRight;
            const openPx = Math.max(1, viewRect.width - insets.left - insets.right);
            const openSpan = span.latticeWidth * (openPx / Math.max(1, viewRect.width));
            view.render(viewportScale(openSpan), format, metres);
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
            const pack = reading.mooreNeighborhood || { sites: [], energy: null };
            const perSite = measures.mooreBars !== false || measures.mooreWaves !== false || measures.mooreJoules !== false;
            neighborhood.render(
                close && perSite ? pack.sites : [],
                close && measures.mooreEnergy !== false ? pack.energy : null,
            );
            // Beyond the quasi-domain the lattice is a dot and nothing is
            // simulated; the reference rings take over from the bracket.
            const centre = reading.latticeCentre;
            const beyond = gauge.subject === 'lattice' && !!centre && measures.scaleLandmarks !== false
                && (reading.shellDiameter || domain) * centre.pixelsPerVoxel <= OUTER_WORLD_PX;
            landmarks.render(beyond ? {
                x: centre.x,
                y: centre.y,
                pixelsPerVoxel: centre.pixelsPerVoxel,
                viewWidth: viewRect.width,
                viewHeight: viewRect.height,
                latticeVoxels: domain,
            } : null);
        },
        dispose() {
            observer?.disconnect();
            window.removeEventListener('resize', invalidateLayout);
            window.removeEventListener('ftd:panel-visibility-change', invalidateLayout);
            root.remove();
        },
    };
}
