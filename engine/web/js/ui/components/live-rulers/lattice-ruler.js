import { BRACKET_RISE_PX, energyStringPath, formatEnergy } from './measure.js';

/**
 * Bracket whose length is a whole number of voxels and follows lattice size.
 * Off the lattice it spans the body of the active scale, in that scale's unit.
 * The tooltip is set per scale by mount.js.
 */

export function createLatticeRuler() {
    const el = document.createElement('div');
    el.className = 'live-ruler live-ruler-lattice';
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
    // Last values written. The element is not compared against: inline-style
    // getters do not return what was set (`0` reads back as `0px`).
    const shown = { hidden: null, opacity: null, stringHidden: null, d: null, value: NaN, place: null, width: null, shift: null, compact: null };
    const put = (key, value) => {
        if (shown[key] === value) return false;
        shown[key] = value;
        return true;
    };

    return {
        el,
        /** `gaugeKey` names the scale behind `formatLength`; the label depends on it. */
        render(scale, formatLength, gaugeKey = '') {
            if (put('hidden', !!scale.hidden)) el.hidden = !!scale.hidden;
            const opacity = String(scale.opacity ?? 1);
            if (put('opacity', opacity)) el.style.opacity = opacity;
            if (scale.hidden) {
                signature = '';
                return;
            }
            const waveReading = scale.wave;
            if (put('stringHidden', !waveReading)) string.hidden = !waveReading;
            if (waveReading) {
                const path = waveReading.path || energyStringPath(waveReading.samples, waveReading.min, waveReading.max);
                if (put('d', path)) wavePath.setAttribute('d', path);
                if (!Object.is(shown.value, waveReading.value)) {
                    shown.value = waveReading.value;
                    waveValue.textContent = formatEnergy(waveReading.value);
                }
                const shift = scale.stringShift ? `translateX(${scale.stringShift}px)` : '';
                if (put('shift', shift)) string.style.transform = shift;
                if (put('compact', !!scale.stringCompact)) string.classList.toggle('is-compact', !!scale.stringCompact);
            }
            const pinned = scale.screen;
            if (pinned) {
                const x = pinned.left;
                const y = pinned.placed ? pinned.top : pinned.top - BRACKET_RISE_PX;
                const place = pinned.placed
                    ? `translate3d(${x}px, ${y}px, 0) translate(-50%, -100%)`
                    : `translate3d(${x}px, ${y}px, 0)`;
                if (put('place', place)) {
                    el.style.left = '0px';
                    el.style.top = '0px';
                    el.style.transform = place;
                }
                const width = pinned.placed ? '' : `${Math.max(2, pinned.width)}px`;
                if (put('width', width)) el.style.width = width;
            } else {
                if (put('place', '')) {
                    el.style.left = '';
                    el.style.top = '';
                    el.style.transform = '';
                }
                if (put('width', '')) el.style.width = '';
            }
            const title = scale.name || scale.subject;
            const next = `${gaugeKey}|${title}|${scale.units}|${scale.px.toFixed(1)}|${scale.fill || ''}|${pinned ? pinned.left.toFixed(1) : ''}|${pinned ? pinned.top.toFixed(1) : ''}`;
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
