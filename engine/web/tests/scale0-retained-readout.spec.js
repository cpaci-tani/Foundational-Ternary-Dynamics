import { test, expect } from '@playwright/test';

test('retained readouts match browser parsing while preserving live text and SVG nodes', async ({ page }) => {
    await page.goto('/js/ui/panels/retained-readout.js');
    const result = await page.evaluate(async () => {
        const { updateRetainedReadout } = await import('/js/ui/panels/retained-readout.js');
        const actual = document.createElement('div'), oracle = document.createElement('div');
        document.body.append(actual);
        const shape = node => node.nodeType === 1 ? {
            tag: node.localName, ns: node.namespaceURI,
            attrs: [...node.attributes].map(a => [a.namespaceURI, a.name, a.value]).sort(),
            children: [...node.childNodes].map(shape),
        } : { type: node.nodeType, value: node.nodeValue };
        const first = '<p title="current &amp; measured">tick <b>1</b></p><svg viewBox="0 0 10 10"><path d="M0 1L2 3" stroke="red"/></svg>';
        updateRetainedReadout(actual, first);
        const paragraph = actual.firstChild, text = paragraph.lastChild.firstChild;
        const svg = actual.lastChild, path = svg.firstChild;
        let added = 0, removed = 0;
        const observer = new MutationObserver(rows => {
            for (const row of rows) { added += row.addedNodes.length; removed += row.removedNodes.length; }
        });
        observer.observe(actual, { childList: true, subtree: true });
        const second = '<p title="old → current">tick <b>2 &lt; 3</b></p><svg viewBox="0 0 20 10"><path d="M1 2L3 4" fill="none"/></svg>';
        updateRetainedReadout(actual, second); oracle.innerHTML = second;
        const sameShape = JSON.stringify(shape(actual)) === JSON.stringify(shape(oracle));
        await Promise.resolve();
        const stable = paragraph === actual.firstChild && text === paragraph.lastChild.firstChild
            && svg === actual.lastChild && path === svg.firstChild;
        const noReplacement = added === 0 && removed === 0;
        observer.disconnect();
        const transitions = ['', '<div>unavailable</div>', first,
            '<!-- observation --><svg><g><rect width="3"/></g><text>τ &amp; φ</text></svg>',
            '<svg><path d="M0 0"/></svg><span>zero: 0</span>'];
        const equivalence = transitions.map(markup => {
            updateRetainedReadout(actual, markup); oracle.innerHTML = markup;
            return JSON.stringify(shape(actual)) === JSON.stringify(shape(oracle));
        });
        const duplicateSkipped = updateRetainedReadout(actual, transitions.at(-1)) === false;
        const namespaced = [
            '<svg><use xlink:href="#first" xml:lang="en"/></svg>',
            '<svg><use xlink:href="#second" xml:lang="fr"/></svg>',
            '<svg><use/></svg>',
        ];
        let useNode;
        const namespaceChecks = namespaced.map((markup, index) => {
            updateRetainedReadout(actual, markup); oracle.innerHTML = markup;
            const current = actual.querySelector('use');
            if (index === 0) useNode = current;
            return {
                equivalent: JSON.stringify(shape(actual)) === JSON.stringify(shape(oracle)),
                retained: current === useNode,
                href: current.getAttributeNS('http://www.w3.org/1999/xlink', 'href'),
                lang: current.getAttributeNS('http://www.w3.org/XML/1998/namespace', 'lang'),
            };
        });
        actual.remove();
        return { sameShape, stable, noReplacement, equivalence, duplicateSkipped, namespaceChecks };
    });
    expect(result).toEqual({ sameShape: true, stable: true, noReplacement: true,
        equivalence: [true, true, true, true, true], duplicateSkipped: true,
        namespaceChecks: [
            { equivalent: true, retained: true, href: '#first', lang: 'en' },
            { equivalent: true, retained: true, href: '#second', lang: 'fr' },
            { equivalent: true, retained: true, href: null, lang: null },
        ] });
});

test('Wave Lab redraws changed history and still responds to independent history controls', async ({ page }) => {
    await page.goto('/js/ui/panels/retained-readout.js');
    const result = await page.evaluate(async () => {
        let draws = 0;
        window.uPlot = class {
            setData() { draws++; }
            setSize() {}
            destroy() {}
        };
        const { WaveInfoComponent } = await import('/js/scales/scale0/ui/overlays/wave-lab/wave-info.js');
        const component = new WaveInfoComponent();
        component.mount(document.body);
        const lane = { energy: 2, peakDirectionalFlux: 3, peakDirectionalWaveVel: 4,
            sampleFlux: 5, sampleWaveVel: 6 };
        try {
            component._updateTrendlines({ singleScenario: true, tick: 0 }, lane);
            const initial = draws;
            component._updateTrendlines({ singleScenario: true, tick: 0 }, lane);
            const reused = draws;
            component._updateTrendlines({ singleScenario: true, tick: 1 }, lane);
            const next = draws;
            component.historyControl.setTicks(10);
            const windowChanged = draws;
            const counts = Object.values(component.history).map(buffer => buffer.count);
            for (const buffer of Object.values(component.history)) buffer.clear();
            component.lastHistoryTick = null;
            component._updateTrendlines({ singleScenario: true, tick: 1 }, lane);
            return { initial, reused, next, windowChanged, reset: draws, counts,
                resetCounts: Object.values(component.history).map(buffer => buffer.count) };
        } finally { component.unmount(); delete window.uPlot; }
    });
    expect(result).toEqual({ initial: 5, reused: 5, next: 10, windowChanged: 15,
        reset: 20, counts: [2, 2, 2, 2, 2], resetCounts: [1, 1, 1, 1, 1] });
});

test('Wave Lab update retains readouts through numerical changes and recovery with delegated audio intact', async ({ page }) => {
    await page.goto('/js/ui/panels/retained-readout.js');
    const result = await page.evaluate(async () => {
        const previousUPlot = window.uPlot;
        window.uPlot = class { setData() {} setSize() {} destroy() {} };
        const [{ WaveInfoComponent }, { SOUND_LATTICE_WAVE_SCENARIO_ID: scenarioId }] = await Promise.all([
            import('/js/scales/scale0/ui/overlays/wave-lab/wave-info.js'),
            import('/js/scales/scale0/analysis/wave-spectrum.js'),
        ]);
        const component = new WaveInfoComponent();
        component.mount(document.body);
        // Only external audio and plotting devices are replaced. update(), the
        // actual metric reducer, DOM renderer and delegated click remain live.
        let audioStarts = 0, tick = 10, amplitude = 2;
        const calls = [], audioMetrics = [];
        component.synth.init = async () => { audioStarts++; component.synth.active = true; };
        component.synth.stop = () => { component.synth.active = false; };
        component.synth.update = metrics => audioMetrics.push(metrics);
        const sample = value => ({ positions: new Float32Array([16.5,16.5,16.5]),
            vectors: new Float32Array([value,0,0]), count: 1 });
        const electric = stride => { calls.push(['E', stride]); return sample(-1); };
        const bridge = {
            latticeSize: 33, currentTick: () => tick, getToggle: () => false,
            getFluxVectorSampled: stride => { calls.push(['J', stride]); return sample(amplitude); },
            getEFieldSampled: electric,
            tick() { throw new Error('readout attempted physics mutation'); },
            setupScenario() { throw new Error('readout attempted scenario mutation'); },
        };
        const valueFor = label => [...component.refs.body.children].find(row =>
            row.children[0]?.textContent.trim().endsWith(label))?.children[1]?.textContent.trim();
        try {
            component.update(bridge, scenarioId);
            const roots = { title: component.refs.title, info: component.refs.info, body: component.refs.body };
            const title = roots.title.firstElementChild, info = roots.info.firstElementChild;
            const bodyRow = roots.body.firstElementChild;
            const valueText = bodyRow.children[1].firstChild;
            const audioButton = roots.title.querySelector('[data-wave-audio]');
            const firstEnergy = valueFor('lane energy');
            tick = 11; amplitude = 4;
            component.update(bridge, scenarioId);
            const secondEnergy = valueFor('lane energy');
            const numericalRetention = title === roots.title.firstElementChild
                && info === roots.info.firstElementChild && bodyRow === roots.body.firstElementChild
                && valueText === bodyRow.children[1].firstChild;
            const samplingBeforeUnavailable = calls.slice();

            // A missing capability closes the real metric reducer. Keep owner
            // identity unchanged so recovery exercises retained DOM state.
            bridge.getEFieldSampled = undefined;
            component.update(bridge, scenarioId);
            const unavailable = /waiting for field buffers/.test(roots.body.textContent);
            const noMeasurementRows = valueFor('lane energy') === undefined;
            const unavailableRetention = title === roots.title.firstElementChild
                && info === roots.info.firstElementChild && bodyRow === roots.body.firstElementChild;
            const noUnavailableSampling = calls.length === samplingBeforeUnavailable.length;
            const unavailableAudio = audioMetrics.at(-1) === null;
            bridge.getEFieldSampled = electric;
            tick = 12; amplitude = 6;
            component.update(bridge, scenarioId);
            const recoveredEnergy = valueFor('lane energy');
            const recovered = !/waiting for field buffers/.test(roots.body.textContent);
            const recoveryRetention = title === roots.title.firstElementChild
                && info === roots.info.firstElementChild && bodyRow === roots.body.firstElementChild;

            // Click the original retained button, rather than invoking the
            // method directly: this must still pass through delegated events.
            audioButton.click();
            await Promise.resolve(); await Promise.resolve();
            const audioEnabled = audioStarts === 1 && component.synth.active
                && roots.title.querySelector('[data-wave-audio]') === audioButton
                && audioButton.textContent.includes('🔊');
            audioButton.click();
            await Promise.resolve();
            const audioDisabled = !component.synth.active && audioButton.textContent.includes('🔈');
            const rootIdentity = Object.entries(roots).every(([key, root]) => root === component.refs[key]);
            return { firstEnergy, secondEnergy, recoveredEnergy, numericalRetention,
                unavailable, noMeasurementRows, unavailableRetention, noUnavailableSampling,
                unavailableAudio, recovered, recoveryRetention, audioEnabled, audioDisabled,
                rootIdentity, samplingBeforeUnavailable };
        } finally {
            component.unmount();
            if (previousUPlot === undefined) delete window.uPlot; else window.uPlot = previousUPlot;
        }
    });
    expect(result).toEqual({
        firstEnergy: '2.50e+0', secondEnergy: '8.50e+0', recoveredEnergy: '1.85e+1',
        numericalRetention: true, unavailable: true, noMeasurementRows: true,
        unavailableRetention: true, noUnavailableSampling: true, unavailableAudio: true,
        recovered: true, recoveryRetention: true, audioEnabled: true, audioDisabled: true,
        rootIdentity: true, samplingBeforeUnavailable: [['J',1], ['E',1], ['J',1], ['E',1]],
    });
});

test('mounted Wave Lab preserves real zero directional peak histories despite nonzero vector norms', async ({ page }) => {
    await page.goto('/js/ui/panels/retained-readout.js');
    const result = await page.evaluate(async () => {
        const previousUPlot = window.uPlot;
        window.uPlot = class { setData() {} setSize() {} destroy() {} };
        const [{ WaveInfoComponent }, waves] = await Promise.all([
            import('/js/scales/scale0/ui/overlays/wave-lab/wave-info.js'),
            import('/js/scales/scale0/analysis/wave-spectrum.js'),
        ]);
        const rows = [];
        try {
            for (const [scenarioId, axis, componentName] of [
                [waves.SOUND_LATTICE_WAVE_SCENARIO_ID, 0, 'x'],
                [waves.RF_LATTICE_WAVE_SCENARIO_ID, 1, 'y'],
            ]) {
                const component = new WaveInfoComponent();
                component.mount(document.body);
                const perpendicular = axis === 0 ? 1 : 0;
                const J = [0, 0, 0], E = [0, 0, 0];
                J[perpendicular] = 3;
                E[perpendicular] = -4;
                let tick = 1;
                const observed = [];
                component.synth.update = m => observed.push(m);
                const sample = vector => ({ positions: new Float32Array([16.5, 16.5, 16.5]),
                    vectors: new Float32Array(vector), count: 1 });
                const bridge = {
                    latticeSize: 33, currentTick: () => tick, getToggle: () => false,
                    getFluxVectorSampled: () => sample(J), getEFieldSampled: () => sample(E),
                };
                const valueFor = label => [...component.refs.body.children].find(row =>
                    row.children[0]?.textContent.trim().endsWith(label))?.children[1]?.textContent.trim();
                try {
                    component.update(bridge, scenarioId);
                    const displayedZero = [valueFor(`peak |J${componentName}|`), valueFor(`peak |W${componentName}|`)];
                    const first = observed.at(-1).lanes[0];
                    const norms = [first.peakFlux, first.peakWaveVel];
                    J[axis] = 2;
                    E[axis] = -5;
                    tick = 2;
                    component.update(bridge, scenarioId);
                    component.update(bridge, scenarioId);
                    const actualHistory = {
                        energy: [component.history.energy.get(0), component.history.energy.get(1)],
                        peakJ: [component.history.peakJ.get(0), component.history.peakJ.get(1)],
                        peakW: [component.history.peakW.get(0), component.history.peakW.get(1)],
                        count: component.history.energy.count,
                    };
                    // Exercise the actual mounted fallback path for unavailable
                    // directional metadata, separately from the measured zeros.
                    component._updateTrendlines({ singleScenario: true, tick: 3 }, {
                        energy: 1, peakDirectionalFlux: NaN, peakFlux: 6,
                        peakDirectionalWaveVel: undefined, peakWaveVel: 7,
                        sampleFlux: 0, sampleWaveVel: 0,
                    });
                    const fallback = [component.history.peakJ.last(), component.history.peakW.last()];
                    rows.push({ componentName, mounted: component.element.isConnected,
                        displayedZero, norms, actualHistory, fallback });
                } finally { component.unmount(); }
            }
            return rows;
        } finally {
            if (previousUPlot === undefined) delete window.uPlot; else window.uPlot = previousUPlot;
        }
    });
    expect(result).toEqual(['x', 'y'].map(componentName => ({
        componentName, mounted: true, displayedZero: ['0.00e+00', '0.00e+00'], norms: [3, 4],
        actualHistory: { energy: [12.5, 27], peakJ: [0, 2], peakW: [0, 5], count: 2 },
        fallback: [6, 7],
    })));
});

test('mounted Wave Lab waits for missing or malformed fields without appending false zero histories', async ({ page }) => {
    await page.goto('/js/ui/panels/retained-readout.js');
    const result = await page.evaluate(async () => {
        const previousUPlot = window.uPlot;
        window.uPlot = class { setData() {} setSize() {} destroy() {} };
        const [{ WaveInfoComponent }, { SOUND_LATTICE_WAVE_SCENARIO_ID: scenarioId }] = await Promise.all([
            import('/js/scales/scale0/ui/overlays/wave-lab/wave-info.js'),
            import('/js/scales/scale0/analysis/wave-spectrum.js'),
        ]);
        const component = new WaveInfoComponent();
        component.mount(document.body);
        let tick = 10;
        const sample = value => ({ positions: new Float32Array([16.5, 16.5, 16.5]),
            vectors: new Float32Array([value, 0, 0]), count: 1 });
        const empty = () => ({ positions: new Float32Array(), vectors: new Float32Array(), count: 0 });
        let J = sample(2), E = sample(-1);
        const calls = [], audio = [];
        component.synth.update = m => audio.push(m);
        const bridge = {
            latticeSize: 33, currentTick: () => tick, getToggle: () => false,
            getFluxVectorSampled(stride) { calls.push(['J', stride]); return J; },
            getEFieldSampled(stride) { calls.push(['E', stride]); return E; },
            tick() { throw new Error('readout attempted physics mutation'); },
        };
        try {
            component.update(bridge, scenarioId);
            const waiting = [];
            for (const [nextJ, nextE] of [[null, sample(-1)], [sample(4), null], [null, null], [sample(4), {}]]) {
                tick++;
                J = nextJ; E = nextE;
                component.update(bridge, scenarioId);
                waiting.push({ waiting: /waiting for field buffers/.test(component.refs.body.textContent),
                    noEnergyReadout: !/lane energy/.test(component.refs.body.textContent),
                    count: component.history.energy.count, lastTick: component.lastHistoryTick,
                    audioUnavailable: audio.at(-1) === null,
                    sameOwner: component.bridgeRef === bridge });
            }
            tick = 15; J = empty(); E = empty();
            component.update(bridge, scenarioId);
            const completedEmpty = { active: audio.at(-1)?.active, count: component.history.energy.count,
                energy: component.history.energy.last(), peakJ: component.history.peakJ.last(),
                peakW: component.history.peakW.last(), tick: component.lastHistoryTick };
            tick = 16; J = sample(6); E = sample(-1);
            component.update(bridge, scenarioId);
            component.update(bridge, scenarioId);
            const recovered = {
                active: audio.at(-1)?.active,
                waiting: /waiting for field buffers/.test(component.refs.body.textContent),
                energy: Array.from({ length: component.history.energy.count }, (_, i) => component.history.energy.get(i)),
                ticks: Array.from({ length: component.history.energy.count }, (_, i) => component.history.energy.getTick(i)),
                peakJ: component.history.peakJ.last(), peakW: component.history.peakW.last(),
            };
            return { waiting, completedEmpty, recovered, calls, mounted: component.element.isConnected };
        } finally {
            component.unmount();
            if (previousUPlot === undefined) delete window.uPlot; else window.uPlot = previousUPlot;
        }
    });
    expect(result).toEqual({
        waiting: Array.from({ length: 4 }, () => ({ waiting: true, noEnergyReadout: true,
            count: 1, lastTick: 10, audioUnavailable: true, sameOwner: true })),
        completedEmpty: { active: true, count: 2, energy: 0, peakJ: 0, peakW: 0, tick: 15 },
        recovered: { active: true, waiting: false, energy: [2.5, 0, 18.5], ticks: [10, 15, 16], peakJ: 6, peakW: 1 },
        calls: Array.from({ length: 8 }, () => [['J', 1], ['E', 1]]).flat(), mounted: true,
    });
});
