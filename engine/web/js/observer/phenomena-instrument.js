// @ts-check
/** Bounded reference instruments. Updated by the workspace UI cadence; no driver or timers. */
import { phenomenaData } from './phenomena-data.js';
/** @typedef {ReturnType<typeof phenomenaData>} PhenomenaData */
/** @typedef {import('./catalog.js').ObserverSettings & import('./optics.js').OpticalSettings} InstrumentSettings */

/** @template {keyof HTMLElementTagNameMap} T @param {T} tag @param {string} className @param {string} [text] */
function node(tag, className, text) {
    const element = document.createElement(tag); element.className = className;
    if (text !== undefined) element.textContent = text;
    return element;
}
/** @param {number} n @param {number} [digits] */
const fixed = (n, digits = 3) => Number.isFinite(n) ? n.toFixed(digits) : 'unavailable';
/** @param {string} label @param {number} height */
function canvas(label, height) {
    const element = node('canvas', 'observer-phenomena-canvas');
    element.width = 680; element.height = height * 2;
    element.style.aspectRatio = `340 / ${height}`;
    element.setAttribute('role', 'img'); element.setAttribute('aria-label', label);
    return element;
}
/** @param {CanvasRenderingContext2D} ctx @param {number[]} a @param {number[]} b @param {string} color @param {boolean} [dashed] */
function line(ctx, a, b, color, dashed = false) {
    ctx.strokeStyle = color; ctx.lineWidth = 1.5; ctx.setLineDash(dashed ? [4, 4] : []);
    ctx.beginPath(); ctx.moveTo(a[0], a[1]); ctx.lineTo(b[0], b[1]); ctx.stroke();
}
/** @param {HTMLCanvasElement} target @param {number} height */
function context(target, height) {
    const ctx = target.getContext('2d');
    if (!ctx) return null;
    ctx.setTransform(2, 0, 0, 2, 0, 0); ctx.clearRect(0, 0, 340, height);
    ctx.font = '11px system-ui'; ctx.textBaseline = 'middle';
    return ctx;
}
/** @param {HTMLCanvasElement} target @param {PhenomenaData} data @param {Readonly<Record<string,boolean>>} layers */
function drawSpacetime(target, data, layers) {
    const ctx = context(target, 244); if (!ctx) return;
    const extent = Math.max(8, Math.min(1e6, (data.received?.delay || 0) * 1.2));
    const unit = 96 / extent;
    /** @param {number} x @param {number} t */
    const point = (x, t) => [170 + x * unit, 120 - t * unit];
    ctx.save(); ctx.beginPath(); ctx.rect(10, 12, 320, 220); ctx.clip();
    for (let j = -4; j <= 4; j++) {
        line(ctx, point(j * extent / 4, -extent), point(j * extent / 4, extent), '#26384b');
        line(ctx, point(-extent * 1.5, j * extent / 4), point(extent * 1.5, j * extent / 4), '#26384b');
    }
    if (layers.lightCones) {
        ctx.fillStyle = '#83d8ff10';
        for (const sign of [-1, 1]) {
            ctx.beginPath(); ctx.moveTo(.../** @type {[number,number]} */ (point(0, 0)));
            for (const x of [-extent, extent]) ctx.lineTo(.../** @type {[number,number]} */ (point(x, sign * extent)));
            ctx.closePath(); ctx.fill();
        }
        line(ctx, point(-extent, -extent), point(extent, extent), '#84dfff');
        line(ctx, point(-extent, extent), point(extent, -extent), '#84dfff');
    }
    line(ctx, point(-extent * 1.5, 0), point(extent * 1.5, 0), '#b1c0d4', true);
    line(ctx, point(0, -extent), point(0, extent), '#62748a');
    if (layers.simultaneity) line(ctx, point(-extent * 1.5, -data.parallelBeta * extent * 1.5), point(extent * 1.5, data.parallelBeta * extent * 1.5), '#d6acff');
    const historyBoundary = data.historyStart - data.time;
    if (historyBoundary > -extent && historyBoundary < extent) line(ctx, point(-extent * 1.5, historyBoundary), point(extent * 1.5, historyBoundary), '#ffb574', true);
    for (const trail of data.trails) for (let j = 1; j < trail.points.length; j++) {
        line(ctx, point(.../** @type {[number,number]} */ (trail.points[j - 1])), point(.../** @type {[number,number]} */ (trail.points[j])), '#8ce0b1', true);
    }
    if (layers.lightPaths && data.received) {
        const start = point(data.received.xi, -data.received.delay);
        line(ctx, start, point(0, 0), '#ffd484');
        ctx.fillStyle = '#ffd484'; ctx.beginPath(); ctx.arc(start[0], start[1], 4, 0, Math.PI * 2); ctx.fill();
    }
    ctx.fillStyle = '#fff'; ctx.beginPath(); ctx.arc(170, 120, 3.5, 0, Math.PI * 2); ctx.fill(); ctx.restore();
    ctx.fillStyle = '#a9bed4'; ctx.fillText('Δt', 178, 16); ctx.fillText('ξ', 319, 131);
    ctx.fillText(`+${fixed(extent, 1)}`, 12, 24); ctx.fillText(`−${fixed(extent, 1)}`, 12, 216);
    ctx.fillText('arrival', 178, 109);
    ctx.fillStyle = '#d5e9fa'; ctx.fillText(data.axisLabel, 12, 236);
}
/** @param {HTMLCanvasElement} target @param {PhenomenaData} data */
function drawCompass(target, data) {
    const ctx = context(target, 132); if (!ctx) return;
    /** Equirectangular full sky in world axes, deliberately independent of FoV.
     * @param {number[]} n */
    const project = n => [170 + (Math.hypot(n[0], n[2]) < 1e-12 ? 0 : Math.atan2(n[0], -n[2])) / Math.PI * 145, 65 - Math.asin(Math.max(-1, Math.min(1, n[1]))) / Math.PI * 92];
    for (const y of [19, 65, 111]) line(ctx, [25, y], [315, y], '#35475b');
    for (const x of [25, 97.5, 170, 242.5, 315]) line(ctx, [x, 19], [x, 111], '#35475b');
    for (const item of data.compass) {
        const original = project(item.original), received = project(item.received);
        // No false connecting stroke across the longitude seam.
        if (Math.abs(original[0] - received[0]) < 145) line(ctx, original, received, '#f3c779');
        ctx.strokeStyle = '#cad6e4'; ctx.setLineDash([]); ctx.strokeRect(original[0] - 3, original[1] - 3, 6, 6);
        ctx.fillStyle = '#ffcd76'; ctx.beginPath(); ctx.arc(received[0], received[1], 3, 0, Math.PI * 2); ctx.fill();
        ctx.fillStyle = '#dcecff'; ctx.fillText(item.label, received[0] + (received[0] > 290 ? -20 : 5), received[1] + (received[1] > 100 ? -8 : 8));
    }
    ctx.fillStyle = '#a9bed4'; ctx.fillText('−180°', 12, 124); ctx.fillText('world −Z', 148, 124); ctx.fillText('+180°', 291, 124);
}

export function createPhenomenaInstrument() {
    const element = node('aside', 'observer-phenomena-instrument'); element.hidden = true;
    element.dataset.observerPhenomena = ''; element.setAttribute('aria-label', 'Reference phenomena instruments'); element.tabIndex = 0;
    const title = node('h2', '', 'REFERENCE INSTRUMENTS · c = 1');
    const provenance = node('p', '', 'Adopted SR and analytic wave illustrations');
    const spacetime = canvas('Spacetime slice: light cones and simultaneity through the observer event', 244);
    spacetime.dataset.observerSpacetime = '';
    const legend = node('p', 'observer-phenomena-legend', 'Cyan: null cone · dashed: coordinate now · purple: observer now · gold: received ray · green: canonical source centre history (unmirrored). Off-axis histories are projected.');
    const clocks = node('p', ''); clocks.dataset.observerFrameReadout = '';
    const received = node('p', ''); received.dataset.observerReceivedEvent = '';
    const compass = canvas('Full-sky aberration: squares are coordinate directions; gold dots are received directions', 132);
    compass.dataset.observerAberration = '';
    const compassLegend = node('p', 'observer-phenomena-legend');
    const waves = node('p', 'observer-phenomena-legend');
    element.append(title, provenance, clocks, spacetime, legend, received, compass, compassLegend, waves);
    return {
        element,
        /** @param {import('./types.js').WorldSnapshot} snapshot @param {InstrumentSettings} settings @param {import('./optics.js').OpticalHit|null} hit */
        update(snapshot, settings, hit) {
            const layers = settings.layers;
            const spacetimeOn = !!(layers.lightCones || layers.simultaneity || layers.lightPaths);
            const waveNames = [['interference', 'Cyan: scalar interference'], ['standingWaves', 'Violet: standing wave; gold: nodes'], ['polarization', 'Orange: E · blue: B']].filter(([id]) => layers[id]).map(([, label]) => label);
            element.hidden = !spacetimeOn && !layers.aberration && !waveNames.length;
            if (element.hidden) return;
            const data = phenomenaData(snapshot, settings, hit);
            clocks.hidden = !spacetimeOn && !layers.aberration;
            clocks.textContent = data.available ? `β ${fixed(data.beta)} · γ ${fixed(data.gamma)} · dτ/dt ${fixed(data.clockRate)} · t ${fixed(data.time, 2)}` : data.reason;
            spacetime.hidden = !spacetimeOn || !data.available; legend.hidden = spacetime.hidden;
            if (!spacetime.hidden) drawSpacetime(spacetime, data, layers);
            received.hidden = !layers.lightPaths || !data.available;
            const event = data.received;
            received.textContent = event
                ? `${event.entityId} · revision ${event.revision}${event.historical ? ' · historical' : ''}${event.mirrored ? ' · mirrored image' : ''}\nEmission ${fixed(event.emissionTime)} → arrival ${fixed(event.arrivalTime)} · delay ${fixed(event.delay)}\nEmitter clock ${fixed(event.properTime)} · Doppler ν/ν₀ ${fixed(event.doppler)} · null residual ${event.nullResidual.toExponential(2)}`
                : settings.optical === false ? 'Received events require the retarded-time optical view.' : 'Aim the crosshair at an object for a current received event. Unavailable or stale history is not replaced with a present pose.';
            compass.hidden = !layers.aberration || !data.available; compassLegend.hidden = compass.hidden;
            if (!compass.hidden) {
                drawCompass(compass, data);
                compassLegend.textContent = `Full sky in world axes. □ coordinate source · ● received direction. Largest deflection ${fixed(Math.max(...data.compass.map(item => item.angle)) * 180 / Math.PI, 1)}°. Distant sources; no finite travel-time claim.`;
            }
            waves.hidden = !waveNames.length;
            waves.textContent = `${waveNames.join(' · ')}. Coordinate-frame reference geometry at t = ${fixed(snapshot.time, 2)}. Field ribbons show orientation, not photon paths.`;
        },
    };
}
