import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';

// Independent tests of actual methods with deterministic timers/DOM ownership.
// These checks do not substitute for browser rendering or hardware FPS gates.
function load(path, symbol, globals = {}) {
    const source = readFileSync(new URL('../js/' + path, import.meta.url), 'utf8')
        .replace(/^import[\s\S]*?from\s+['"][^'"]+['"];\s*/gm, '')
        .replace(/export\s+/g, '');
    return vm.runInNewContext(source + '\n' + symbol, globals);
}
function classList() {
    const values = new Set();
    return { add: x => values.add(x), remove: x => values.delete(x), contains: x => values.has(x) };
}
function element() {
    return {
        children: [], parentNode: null, classList: classList(), dataset: {},
        addEventListener() {}, removeEventListener() {},
        get nextSibling() {
            const siblings = this.parentNode?.children || [];
            return siblings[siblings.indexOf(this) + 1] || null;
        },
        appendChild(child) { return this.insertBefore(child, null); },
        insertBefore(child, next) {
            if (next !== null && next.parentNode !== this) throw new Error('NotFoundError');
            child.remove();
            const index = next === null ? this.children.length : this.children.indexOf(next);
            this.children.splice(index, 0, child); child.parentNode = this; return child;
        },
        remove() {
            if (this.parentNode) {
                const siblings = this.parentNode.children;
                siblings.splice(siblings.indexOf(this), 1); this.parentNode = null;
            }
        },
        querySelector() { return null; }, querySelectorAll() { return []; },
    };
}
function fullscreenFixture() {
    const portal = element(), overlay = element(), body = element();
    overlay.querySelector = () => portal;
    const attach = load('ui/charts/chart-fullscreen.js', 'attachFullscreen', {
        document: { getElementById: () => overlay, body }, requestAnimationFrame: () => 1,
    });
    return { portal, overlay, attach };
}

test('pending fade removal and active replacement both release on panel cleanup', () => {
    const timers = new Map(); let serial = 0;
    const Panel = load('ui/panels/charts-panel/component.js', 'ChartsPanelComponent', {
        scale0Charts: [], scale1Charts: [], scale2Charts: [], scale3Charts: [], scale4Charts: [],
        setTimeout(fn) { const id = ++serial; timers.set(id, fn); return id; },
        clearTimeout(id) { timers.delete(id); },
    });
    const panel = new Panel({ dataset: {} }); panel.grid = {}; panel.active = new Set();
    const fading = { el: element(), calls: 0, destroy() { this.calls++; } };
    const replacement = { calls: 0, destroy() { this.calls++; } };
    panel.cards.set('a', fading); panel._syncCards();
    assert.equal(timers.size, 1); assert.equal(fading.calls, 0);
    panel.cards.set('a', replacement); panel.cleanup(); panel.cleanup();
    assert.equal(fading.calls, 1); assert.equal(replacement.calls, 1);
    assert.equal(timers.size, 0); assert.equal(panel._destroyTimers.size, 0);
});

test('normal fade completion releases once and drops teardown ownership', () => {
    let callback;
    const Panel = load('ui/panels/charts-panel/component.js', 'ChartsPanelComponent', {
        scale0Charts: [], scale1Charts: [], scale2Charts: [], scale3Charts: [], scale4Charts: [],
        setTimeout(fn) { callback = fn; return 1; }, clearTimeout() {},
    });
    const panel = new Panel({ dataset: {} }); panel.active = new Set();
    const card = { el: element(), calls: 0, destroy() { this.calls++; } };
    panel.cards.set('a', card); panel._syncCards(); callback(); panel.cleanup();
    assert.equal(card.calls, 1); assert.equal(panel._destroyTimers.size, 0);
});

for (const operation of ['hide', 'cleanup']) {
    test(`Lagrangian ${operation} exits actual fullscreen portal even after sibling removal`, () => {
        const { portal, overlay, attach } = fullscreenFixture();
        const parent = element(), card = element(), sibling = element();
        parent.appendChild(card); parent.appendChild(sibling); attach(card);
        card._ftdCard._enterFullscreen(); sibling.remove();
        const Component = load('ui/panels/lagrangian-panel/component.js', 'LagrangianPanelComponent', { terms: [] });
        const component = new Component({ dataset: {} }); component.hidden = new Set(['a']);
        let releases = 0;
        component.cards.set('a', { card, chart: { destroy() {
            assert.equal(card._ftdCard._isFullscreen, false);
            assert.equal(portal.children.length, 0); releases++;
        } } });
        if (operation === 'hide') component._syncCards(); else component.cleanup();
        assert.equal(releases, 1); assert.equal(component.cards.size, 0);
        assert.equal(overlay.classList.contains('is-open'), false);
    });
}

test('ChartCard destruction exits portal before releasing chart and removes restored element', () => {
    const { portal, attach } = fullscreenFixture();
    const parent = element(), cardEl = element(); parent.appendChild(cardEl); attach(cardEl);
    cardEl._ftdCard._enterFullscreen();
    const Card = load('ui/panels/charts-panel/chart-card.js', 'ChartCard');
    const card = Object.create(Card.prototype); card.el = cardEl;
    let released = 0; card.chart = { destroy() { assert.equal(portal.children.length, 0); released++; } };
    card.destroy(); assert.equal(released, 1); assert.equal(parent.children.length, 0);
});

function buffer(values, ticks) {
    return {
        count: values.length, total: values.length, size: values.length,
        last: () => values.at(-1), get: i => values[i], getTick: i => ticks[i],
        flattenInto(target, n) { const a = values.slice(-n); target.set(a); return a.length; },
        flattenTicksInto(target, n) { const a = ticks.slice(-n); target.set(a); return a.length; },
    };
}
function chartFixture() {
    const Chart = load('ui/charts/uplot-chart.js', 'UPlotChart');
    const chart = Object.create(Chart.prototype);
    Object.assign(chart, {
        hub: { a: buffer([1, 2, 3], [10, 20, 30]), b: buffer([4, 5, 6], [10, 20, 30]) },
        series: [{ buffer: 'a' }, { buffer: 'b' }], visibleSamples: 160,
        xs: new Float64Array(3), ys: [new Float64Array(3), new Float64Array(3)],
        _bufferStamps: [{}, {}], uplot: { setData(data) { this.data = data; } },
    });
    return chart;
}
test('removing a secondary buffer does not republish its previous numeric column', () => {
    const chart = chartFixture(); chart.update(); delete chart.hub.b; chart.update();
    assert.ok([...chart.uplot.data[2]].every(Number.isNaN));
});
test('shorter and differently stamped secondary data cannot masquerade as first-series times', () => {
    const chart = chartFixture(); chart.update();
    chart.hub.b = buffer([99], [30]); chart.update();
    const short = [...chart.uplot.data[2]];
    assert.ok(Number.isNaN(short[0]) && Number.isNaN(short[1]));
    assert.equal(short[2], 99);
    chart.hub.b = buffer([90, 91, 92], [11, 21, 31]); chart.update();
    assert.ok([...chart.uplot.data[2]].every(Number.isNaN));
});
test('independent stamped history selects latest duplicate and refuses unstamped data', () => {
    const chart = chartFixture();
    chart.hub.b = buffer([40, 50, 51, 60], [10, 20, 20, 30]); chart.update();
    assert.deepEqual([...chart.uplot.data[2]], [40, 51, 60]);
    chart.hub.b = buffer([70, 80, 90], [10, 20, 30]); delete chart.hub.b.getTick;
    chart.update(); assert.ok([...chart.uplot.data[2]].every(Number.isNaN));
});
test('shared channel ring retains aligned exact values without independent timestamp lookup', () => {
    const chart = chartFixture(); const parent = {};
    chart.hub.a.parent = parent; chart.hub.b.parent = parent;
    chart.hub.b.getTick = () => { throw new Error('Shared ring must use its common clock'); };
    chart.update(); assert.deepEqual([...chart.uplot.data[2]], [4, 5, 6]);
});
test('timestamp-less legacy histories align their own ordinal supports', () => {
    const chart = chartFixture(); delete chart.hub.a.flattenTicksInto;
    chart.hub.b = buffer([99], [30]); chart.hub.b.total = 3;
    delete chart.hub.b.getTick; chart.update();
    assert.deepEqual([...chart.uplot.data[0]], [0, 1, 2]);
    assert.ok(Number.isNaN(chart.uplot.data[2][0]));
    assert.ok(Number.isNaN(chart.uplot.data[2][1])); assert.equal(chart.uplot.data[2][2], 99);
});

test('uPlot teardown cancels delayed resize and releases all subscriptions once', () => {
    const releases = []; const Chart = load('ui/charts/uplot-chart.js', 'UPlotChart', {
        cancelAnimationFrame: id => releases.push(`raf${id}`),
    });
    const chart = Object.create(Chart.prototype);
    Object.assign(chart, {
        _resizeFrame: 7, _unsubscribeHistory: () => releases.push('history'),
        _hoverTarget: { removeEventListener: name => releases.push(name) },
        tooltip: { destroy: () => releases.push('tooltip') },
        _ro: { disconnect: () => releases.push('observer') }, container: { _ftdResize() {} },
        uplot: { destroy: () => releases.push('plot') },
    });
    chart.destroy(); chart.destroy(); chart.update();
    assert.equal(releases.length, 9); assert.equal(new Set(releases).size, 9);
    assert.equal(chart.container._ftdResize, undefined);
});

test('history window selects actual irregular ticks without modifying retained samples', () => {
    const History = load('ui/charts/history-window.js', 'TickHistoryControl');
    const history = Object.create(History.prototype); history.state = { mode: 'window', ticks: 10 };
    const values = [1, 2, 3, 4], ticks = [0, 8, 25, 30]; const b = buffer(values, ticks);
    assert.equal(history.visibleCount(b), 2);
    assert.deepEqual(Array.from(history.slice(ticks, t => t)), [25, 30]);
    history.state.mode = 'all'; assert.equal(history.visibleCount(b), 4);
    assert.deepEqual(values, [1, 2, 3, 4]); assert.deepEqual(ticks, [0, 8, 25, 30]);
});
