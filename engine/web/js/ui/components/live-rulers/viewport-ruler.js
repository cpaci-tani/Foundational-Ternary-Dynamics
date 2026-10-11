/**
 * Full-viewport ruler. Its labels are the width currently in view, in the
 * length unit of the active scale. The tooltip is set per scale by mount.js.
 */

export function createViewportRuler() {
    const el = document.createElement('div');
    el.className = 'live-ruler live-ruler-view';
    const label = document.createElement('div');
    label.className = 'live-ruler-label';
    const track = document.createElement('div');
    track.className = 'live-ruler-track';
    const ticks = document.createElement('div');
    ticks.className = 'live-ruler-ticks';
    el.append(label, track, ticks);

    let signature = '';
    let lastSpan = NaN;
    let lastStep = NaN;
    let lastGauge = null;
    let lastTickCount = -1;

    return {
        el,
        /** `gauge` is the scale's length gauge behind `formatLength`; every caption depends on it. */
        render(scale, formatLength, gauge = null) {
            // The ticks follow from span and step, so an unchanged scale needs
            // no string work at all.
            if (scale.span === lastSpan && scale.step === lastStep && gauge === lastGauge
                && scale.ticks.length === lastTickCount) return;
            // Another scale's unit rewrites the tick captions as well.
            if (gauge !== lastGauge) signature = '';
            lastSpan = scale.span;
            lastStep = scale.step;
            lastGauge = gauge;
            lastTickCount = scale.ticks.length;
            label.textContent = `view · ${formatLength(scale.span, scale.step)}`;
            const next = `${scale.step}|${scale.ticks.join(',')}|${scale.span}`;
            if (next === signature) return;
            signature = next;
            ticks.replaceChildren();
            const span = scale.span > 0 ? scale.span : 1;
            for (const value of scale.ticks) {
                const tick = document.createElement('span');
                tick.className = 'live-ruler-tick';
                tick.style.left = `${(value / span) * 100}%`;
                const caption = document.createElement('span');
                caption.textContent = formatLength(value, scale.step);
                tick.appendChild(caption);
                ticks.appendChild(tick);
            }
        },
    };
}
