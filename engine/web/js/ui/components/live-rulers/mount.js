import {
    anchorRuler, BRACKET_RISE_PX, ENERGY_TITLES, formatGaugeLength, lengthGauge, LIVE_MEASURE_DEFAULTS, OUTER_WORLD_PX, RULER_END_LABEL_PX,
    placeString, RULER_TICK_SPACING_PX, rulerInsets, viewportScale, visibleLatticeSpan,
} from './measure.js';
import { createLatticeRuler } from './lattice-ruler.js';
import { createVoxelClocks } from './voxel-clocks.js';
import { createMooreNeighborhood } from './neighborhood.js';
import { createLandmarkRings } from './landmark-rings.js';
import { createViewportRuler } from './viewport-ruler.js';

// Everything that can sit in the view ruler's row. The shell pieces live
// outside the view container; whatever floats inside it is found by walking
// the container's own children, so a panel added later is avoided without
// being listed here. `data-view-obstacle` opts in anything else.
const SHELL_OBSTACLES = '#tab-bar, #panel-rail-resizer, #panel-area, #panel-side-resizer, #play-bar, '
    + '.conservation-micropanel, [data-view-obstacle]';
const NOT_OBSTACLES = '.live-rulers, #viewport-frame-chrome, canvas';
const LAYOUT_MAX_AGE_MS = 250;

/** A rectangle only counts while the element is actually on screen. */
function visibleRect(el) {
    if (!el || el.hidden || el.getClientRects().length === 0) return null;
    const style = getComputedStyle(el);
    if (style.visibility === 'hidden' || Number(style.opacity) < 0.05) return null;
    return el.getBoundingClientRect();
}

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
    // The tooltip layer reads data-ui-tooltip. Each scale has its own text,
    // so it is written here when the scale changes.
    const setTooltip = (key, el, text) => { if (once(key, text)) el.dataset.uiTooltip = text; };
    const latticeString = lattice.el.querySelector('.live-ruler-string');
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
        const consider = (el) => {
            if (observer && !watched.has(el)) {
                watched.add(el);
                observer.observe(el);
            }
            const rect = visibleRect(el);
            if (rect) obstacles.push(rect);
        };
        for (const el of document.querySelectorAll(SHELL_OBSTACLES)) consider(el);
        for (const el of container.children) {
            if (el !== root && !el.matches(NOT_OBSTACLES)) consider(el);
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
            const gauge = lengthGauge(reading.engineMode);
            const format = (units, step) => formatGaugeLength(gauge, units, step);
            setTooltip('view-tooltip', view.el, gauge.viewTitle);
            setTooltip('body-tooltip', lattice.el, gauge.bodyTitle);
            const domain = gauge.domainUnits || Math.max(1, reading.latticeSize || 1);
            const span = visibleLatticeSpan(reading);
            const { viewRect, obstacles } = readLayout();
            // The ruler takes the widest open stretch of its row and is hidden
            // when that stretch is too narrow to carry it.
            const insets = rulerInsets(viewRect, obstacles);
            const viewHidden = measures.viewRuler === false || insets.hidden;
            if (once('view-hidden', viewHidden)) view.el.hidden = viewHidden;
            const insetLeft = `${insets.left}px`;
            if (once('inset-left', insetLeft)) view.el.style.left = insetLeft;
            const insetRight = `${insets.right}px`;
            if (once('inset-right', insetRight)) view.el.style.right = insetRight;
            const openPx = Math.max(1, viewRect.width - insets.left - insets.right);
            const openSpan = span.latticeWidth * (openPx / Math.max(1, viewRect.width));
            // As many ticks as the width can label, and none under the end label.
            const divisions = Math.max(2, Math.min(8, Math.floor(openPx / RULER_TICK_SPACING_PX)));
            view.render(viewportScale(openSpan, divisions, RULER_END_LABEL_PX / openPx), format, gauge);
            const anchor = anchorRuler({
                domainUnits: domain,
                pixelsPerUnit: span.pixelsPerVoxel,
                viewPx: openPx,
                subject: gauge.subject,
                unitSubject: gauge.unitSubject,
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
            } else if (anchor.subject === 'lattice') {
                // The bracket spans the whole lattice, so the figure above it
                // is the lattice total. Beyond the lattice nothing is shown.
                anchor.wave = measures.energyString === false ? null : (reading.latticeEnergy || null);
            }
            if (latticeString) {
                setTooltip('energy-tooltip', latticeString,
                    anchor.subject === 'voxel' ? ENERGY_TITLES.voxel : ENERGY_TITLES.lattice);
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
            // The lattice figure stays inside the view and, where the clock
            // face stands over the bracket, steps aside from it.
            const placed = anchor.wave && anchor.subject === 'lattice' && anchor.screen
                ? placeString({
                    centre: anchor.screen.left + anchor.screen.width / 2,
                    top: anchor.screen.top - BRACKET_RISE_PX,
                    lineWidth: measures.stringWidth ?? 148,
                    valueSize: measures.valueSize ?? 16,
                    viewWidth: viewRect.width,
                }, reading.clockDisc)
                : null;
            anchor.stringShift = placed ? placed.shift : 0;
            anchor.stringCompact = placed ? placed.compact : false;
            lattice.render(anchor, format, gauge.mode);
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
