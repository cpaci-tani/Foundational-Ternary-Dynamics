import { energyStringPath, formatEnergy } from './measure.js';

/** Bracket whose length is a whole number of voxels and follows lattice size. */

export function createLatticeRuler() {
    const el = document.createElement('div');
    el.className = 'live-ruler live-ruler-lattice';
    el.title = 'Absolute lattice length. One voxel is the electron-primary Planck length.';
    const string = document.createElement('div');
    string.className = 'live-ruler-string';
    string.hidden = true;
    const wave = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
    wave.setAttribute('viewBox', '0 0 100 16');
    wave.setAttribute('preserveAspectRatio', 'none');
    const wavePath = document.createElementNS('http://www.w3.org/2000/svg', 'path');
    wave.append(wavePath);
    const waveValue = document.createElement('span');
    waveValue.className = 'live-ruler-string-value';
    string.append(wave, waveValue);
    const bracket = document.createElement('div');
    bracket.className = 'live-ruler-bracket';
    bracket.append(
        Object.assign(document.createElement('span'), { className: 'live-ruler-cap' }),
        Object.assign(document.createElement('span'), { className: 'live-ruler-bar' }),
        Object.assign(document.createElement('span'), { className: 'live-ruler-cap' }),
    );
    const label = document.createElement('div');
    label.className = 'live-ruler-label';
    el.append(string, bracket, label);

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
            const waveReading = scale.wave;
            string.hidden = !waveReading;
            if (waveReading) {
                wavePath.setAttribute('d', energyStringPath(waveReading.samples, waveReading.min, waveReading.max));
                waveValue.textContent = formatEnergy(waveReading.value);
            }
            const pinned = scale.screen;
            if (pinned) {
                el.style.left = `${pinned.left}px`;
                el.style.top = `${pinned.placed ? pinned.top : pinned.top - 28}px`;
                el.style.transform = pinned.placed ? 'translate(-50%, -100%)' : 'none';
                el.style.width = pinned.placed ? '' : `${Math.max(2, pinned.width)}px`;
            } else {
                el.style.left = '';
                el.style.top = '';
                el.style.transform = '';
                el.style.width = '';
            }
            const title = scale.name || scale.subject;
            const next = `${title}|${scale.units}|${scale.px.toFixed(1)}|${scale.fill || ''}|${pinned ? pinned.left.toFixed(1) : ''}|${pinned ? pinned.top.toFixed(1) : ''}`;
            if (next === signature) return;
            signature = next;
            bracket.style.width = `${Math.max(2, pinned ? pinned.width : scale.px)}px`;
            const bar = bracket.querySelector('.live-ruler-bar');
            if (scale.fill) {
                bar.style.background = scale.fill;
                bar.style.borderTopColor = scale.fill;
                bar.style.height = 'var(--bar-thickness, 8px)';
                bar.style.marginTop = '';
            } else {
                bar.style.background = '';
                bar.style.borderTopColor = '';
                bar.style.height = '';
                bar.style.marginTop = '';
            }
            label.textContent = `${title} · ${formatLength(scale.units, scale.units)}`;
        },
    };
}
