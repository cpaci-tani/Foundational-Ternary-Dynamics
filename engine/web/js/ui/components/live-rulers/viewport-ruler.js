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

    return {
        el,
        render(scale, formatLength) {
            const next = `${scale.step}|${scale.ticks.join(',')}|${scale.span}`;
            label.textContent = `view · ${formatLength(scale.span, scale.step)}`;
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
