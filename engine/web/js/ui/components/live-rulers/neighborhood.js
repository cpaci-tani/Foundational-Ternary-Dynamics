import { energyStringPath, formatEnergy } from './measure.js';

/** Compact size bar, wave, and joule label for each Moore neighbor. */

export function createMooreNeighborhood() {
    const root = document.createElement('div');
    root.className = 'moore-neighborhood';
    const pool = [];

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
        return { el, wave, path, value, bar };
    };

    return {
        el: root,
        render(sites) {
            const list = sites || [];
            while (pool.length < list.length) pool.push(make());
            for (let i = 0; i < pool.length; i++) {
                const site = list[i];
                const item = pool[i];
                item.el.hidden = !site;
                if (!site) continue;
                item.el.style.left = `${site.x}px`;
                item.el.style.top = `${site.y}px`;
                item.el.style.opacity = String(site.opacity);
                item.wave.hidden = !site.wave;
                item.value.hidden = !site.showJoule;
                item.bar.hidden = !site.bar;
                if (site.wave) {
                    item.path.setAttribute('d', energyStringPath(site.wave.samples, site.wave.min, site.wave.max));
                }
                if (site.showJoule) item.value.textContent = formatEnergy(site.joule);
                if (site.bar) {
                    item.bar.style.width = `${Math.max(2, site.px)}px`;
                    item.bar.style.background = site.fill || '#e8eef5';
                }
            }
        },
    };
}
