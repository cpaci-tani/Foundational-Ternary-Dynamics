/** Bracket whose length is a whole number of voxels and follows lattice size. */

export function createLatticeRuler() {
    const el = document.createElement('div');
    el.className = 'live-ruler live-ruler-lattice';
    el.title = 'Absolute lattice length. One voxel is the electron-primary Planck length.';
    const bracket = document.createElement('div');
    bracket.className = 'live-ruler-bracket';
    bracket.append(
        Object.assign(document.createElement('span'), { className: 'live-ruler-cap' }),
        Object.assign(document.createElement('span'), { className: 'live-ruler-bar' }),
        Object.assign(document.createElement('span'), { className: 'live-ruler-cap' }),
    );
    const label = document.createElement('div');
    label.className = 'live-ruler-label';
    el.append(bracket, label);

    let signature = '';

    return {
        el,
        render(scale, formatLength) {
            el.hidden = !!scale.hidden;
            el.style.opacity = String(scale.opacity ?? 1);
            if (scale.hidden) {
                signature = '';
                return;
            }
            const pinned = scale.screen;
            if (pinned) {
                el.style.left = `${pinned.left}px`;
                el.style.top = `${pinned.top - 28}px`;
                el.style.transform = 'none';
                el.style.width = `${Math.max(2, pinned.width)}px`;
            } else {
                el.style.left = '';
                el.style.top = '';
                el.style.transform = '';
                el.style.width = '';
            }
            const next = `${scale.subject}|${scale.units}|${scale.px.toFixed(1)}|${pinned ? pinned.left.toFixed(1) : ''}|${pinned ? pinned.top.toFixed(1) : ''}`;
            if (next === signature) return;
            signature = next;
            bracket.style.width = `${Math.max(2, pinned ? pinned.width : scale.px)}px`;
            label.textContent = `${scale.subject} · ${formatLength(scale.units, scale.units)}`;
        },
    };
}
