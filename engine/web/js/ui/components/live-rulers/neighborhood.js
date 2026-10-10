import { energyAreaPath, energyStringPath, formatEnergy, formatLength } from './measure.js';

/** Compact size bar, wave, and joule label for each Moore neighbor. */

// Every readout remembers what it last wrote and writes only a change.
// Comparing against the element is no test: `style.transform` reads back
// `0px` for a `0`, and a `background` set as `hsl()` reads back as `rgb()`,
// so the element never equals the string just built and every frame would
// rewrite every readout.
export function createMooreNeighborhood() {
    const root = document.createElement('div');
    root.className = 'moore-neighborhood';
    const pool = [];
    const energy = document.createElement('div');
    energy.className = 'moore-energy';
    energy.hidden = true;
    const area = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
    area.setAttribute('viewBox', '0 0 100 16');
    area.setAttribute('preserveAspectRatio', 'none');
    const areaPath = document.createElementNS('http://www.w3.org/2000/svg', 'path');
    area.append(areaPath);
    const energyValue = document.createElement('span');
    energyValue.className = 'moore-energy-value';
    const ruler = document.createElement('div');
    ruler.className = 'moore-energy-ruler';
    const energyLabel = document.createElement('div');
    energyLabel.className = 'moore-energy-label';
    energyLabel.textContent = `moore · ${formatLength(3)}`;
    energy.append(area, energyValue, ruler, energyLabel);
    root.append(energy);
    const shown = { hidden: true, x: NaN, y: NaN, width: NaN, path: null, value: NaN };

    const make = () => {
        const el = document.createElement('div');
        el.className = 'moore-readout';
        const wave = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
        wave.setAttribute('viewBox', '0 0 100 16');
        wave.setAttribute('preserveAspectRatio', 'none');
        const path = document.createElementNS('http://www.w3.org/2000/svg', 'path');
        wave.append(path);
        const value = document.createElement('span');
        value.className = 'moore-joule';
        const bar = document.createElement('span');
        bar.className = 'moore-bar';
        el.append(wave, value, bar);
        root.append(el);
        return {
            el, wave, path, value, bar,
            // Last values written to the elements above.
            hidden: false, x: NaN, y: NaN, opacity: NaN,
            waveHidden: false, valueHidden: false, barHidden: false,
            d: null, joule: NaN, px: NaN, fill: null,
        };
    };

    return {
        el: root,
        render(sites, sum) {
            const sumHidden = !sum;
            if (shown.hidden !== sumHidden) {
                shown.hidden = sumHidden;
                energy.hidden = sumHidden;
            }
            if (sum) {
                if (sum.x !== shown.x || sum.y !== shown.y) {
                    shown.x = sum.x;
                    shown.y = sum.y;
                    energy.style.transform = `translate3d(${sum.x}px, ${sum.y}px, 0) translate(-50%, -100%)`;
                }
                const width = Math.max(48, sum.width);
                if (width !== shown.width) {
                    shown.width = width;
                    energy.style.width = `${width}px`;
                }
                const path = sum.area || energyAreaPath(sum.samples, sum.min, sum.max);
                if (path !== shown.path) {
                    shown.path = path;
                    areaPath.setAttribute('d', path);
                }
                if (!Object.is(sum.value, shown.value)) {
                    shown.value = sum.value;
                    energyValue.textContent = formatEnergy(sum.value);
                }
            }
            const list = sites || [];
            while (pool.length < list.length) pool.push(make());
            for (let i = 0; i < pool.length; i++) {
                const site = list[i];
                const item = pool[i];
                const hidden = !site;
                if (item.hidden !== hidden) {
                    item.hidden = hidden;
                    item.el.hidden = hidden;
                }
                if (!site) continue;
                if (site.x !== item.x || site.y !== item.y) {
                    item.x = site.x;
                    item.y = site.y;
                    item.el.style.transform = `translate3d(${site.x}px, ${site.y}px, 0) translate(-50%, -100%)`;
                }
                if (site.opacity !== item.opacity) {
                    item.opacity = site.opacity;
                    item.el.style.opacity = String(site.opacity);
                }
                const waveHidden = !site.wave;
                if (item.waveHidden !== waveHidden) {
                    item.waveHidden = waveHidden;
                    item.wave.hidden = waveHidden;
                }
                const valueHidden = !site.showJoule;
                if (item.valueHidden !== valueHidden) {
                    item.valueHidden = valueHidden;
                    item.value.hidden = valueHidden;
                }
                const barHidden = !site.bar;
                if (item.barHidden !== barHidden) {
                    item.barHidden = barHidden;
                    item.bar.hidden = barHidden;
                }
                if (site.wave) {
                    const d = site.wave.path || energyStringPath(site.wave.samples, site.wave.min, site.wave.max);
                    if (d !== item.d) {
                        item.d = d;
                        item.path.setAttribute('d', d);
                    }
                }
                if (site.showJoule && !Object.is(site.joule, item.joule)) {
                    item.joule = site.joule;
                    item.value.textContent = formatEnergy(site.joule);
                }
                if (site.bar) {
                    const px = Math.max(2, site.px);
                    if (px !== item.px) {
                        item.px = px;
                        item.bar.style.width = `${px}px`;
                    }
                    const fill = site.fill || '#e8eef5';
                    if (fill !== item.fill) {
                        item.fill = fill;
                        item.bar.style.background = fill;
                    }
                }
            }
        },
    };
}
