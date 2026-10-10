/** Full-viewport ruler. Its labels are how many lattice voxels currently fit across the view. */

export function createViewportRuler() {
    const el = document.createElement('div');
    el.className = 'live-ruler live-ruler-view';
    el.title = 'Visible width in metres. One voxel is the electron-primary Planck length.';
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
    let lastUnit = NaN;
    let lastTickCount = -1;

    return {
        el,
        /** `unit` is the metres per voxel behind `formatLength`; the label depends on it. */
        render(scale, formatLength, unit = 0) {
            // The ticks follow from span and step, so an unchanged scale needs
            // no string work at all.
            if (scale.span === lastSpan && scale.step === lastStep && unit === lastUnit
                && scale.ticks.length === lastTickCount) return;
            lastSpan = scale.span;
            lastStep = scale.step;
            lastUnit = unit;
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
